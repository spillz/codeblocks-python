/***************************************************************
 * Name:      Python Debugger Plugin
 * Purpose:   Code::Blocks plugin
 * Author:    Damien Moore ()
 * Created:   2006-09-28
 * Copyright: Damien Moore
 * License:   GPL
 **************************************************************/

#ifndef PyDebugger_H_INCLUDED
#define PyDebugger_H_INCLUDED

// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h>
#include <loggers.h>
#include <logger.h>
#include <cbplugin.h> // for "class cbPlugin/cbDebuggerPlugin"
#include "ConfigDialog.h"
#include "dialogs.h"


#define DBGCMDTYPE_FLOWCONTROL 1
#define DBGCMDTYPE_BREAKPOINT 2
#define DBGCMDTYPE_WATCHEXPRESSION 3
#define DBGCMDTYPE_WATCHTOOLTIP 4
#define DBGCMDTYPE_EVALUATE 5
#define DBGCMDTYPE_USERCOMMAND 6
#define DBGCMDTYPE_CALLSTACK 7
#define DBGCMDTYPE_OTHER 8

//class ConfigDialog;

//class InterpreterCollection;
#include <tr1/memory>

typedef std::set<int> BPLtype;

typedef std::tr1::shared_ptr<cbBreakpoint> Pointer;
typedef std::vector<Pointer> BPList;
typedef std::vector<cbStackFrame> StackList;

struct StackInfo
{
    int activeframe;
    StackList frames;
};

struct PythonCmdDispatchData
{
    int type;
    wxString cmdtext;
};

class PyDebugger : public cbDebuggerPlugin
{
    public:
		/** Constructor. */
        PyDebugger();
		/** Destructor. */
        virtual ~PyDebugger();

        virtual void OnAttachReal();
        virtual void OnReleaseReal(bool appShutDown);

        virtual void ShowToolMenu() {}
        virtual bool ToolMenuEnabled() {return true;}

//        /** @brief Start a new debugging process. */
//        virtual bool Debug(bool breakOnEntry) = 0;
//
//        /** @brief Continue running the debugged program. */
//        virtual void Continue() = 0;
//
//        /** @brief Run the debugged program until it reaches the cursor at the current editor */
//        virtual bool RunToCursor(const wxString& filename, int line, const wxString& line_text) {return false;}
//
//        /** @brief Sets the position of the Program counter to the specified filename:line */
//        virtual void SetNextStatement(const wxString& filename, int line) {}
//
//        /** @brief Execute the next instruction and return control to the debugger. */
//        virtual void Next() = 0;
//
//        /** @brief Execute the next instruction and return control to the debugger. */
//        virtual void NextInstruction() {}
//
//        /** @brief Execute the next instruction and return control to the debugger, if the instruction is a function call step into it. */
//        virtual void StepIntoInstruction() {}
//
//        /** @brief Execute the next instruction, stepping into function calls if needed, and return control to the debugger. */
//        virtual void Step() = 0;
//
//        /** @brief Execute the next instruction, stepping out of function calls if needed, and return control to the debugger. */
//        virtual void StepOut() {}
//
//        /** @brief Break the debugging process (stop the debuggee for debugging). */
//        virtual void Break() = 0;
//
//        /** @brief Stop the debugging process (exit debugging). */
//        virtual void Stop() = 0;
//
        /** @brief Is the plugin currently debugging? */
//        virtual bool IsRunning() const  {}

        /** @brief Is the plugin stopped on breakpoint? */
        virtual bool IsStopped() const  {return m_DebugCommandCount==0;}

        /** @brief Get the exit code of the last debug process. */
//        virtual int GetExitCode() const  {}

        // stack frame calls;
        virtual int GetStackFrameCount() const;
        virtual const cbStackFrame& GetStackFrame(int index) const;
        virtual void SwitchToFrame(int number);
        virtual int GetActiveStackFrame() const;

        // breakpoints calls
        /** @brief Request to add a breakpoint.
          * @param file The file to add the breakpoint based on a file/line pair.
          * @param line The line number to put the breakpoint in @c file.
          * @return True if succeeded, false if not.
          */
        virtual cbBreakpoint* AddBreakpoint(const wxString& filename, int line);

        /** @brief Request to add a breakpoint based on a data expression.
          * @param dataExpression The data expression to add the breakpoint.
          * @return True if succeeded, false if not.
          */
        virtual cbBreakpoint* AddDataBreakpoint(const wxString& dataExpression) {return NULL;}
        virtual int GetBreakpointsCount() const  {return m_bplist.size();}
        virtual cbBreakpoint* GetBreakpoint(int index);
        virtual const cbBreakpoint* GetBreakpoint(int index) const;
        virtual void UpdateBreakpoint(cbBreakpoint *breakpoint)  {}
        virtual void DeleteBreakpoint(cbBreakpoint* breakpoint);
        virtual void DeleteAllBreakpoints();
        virtual void ShiftBreakpoint(int index, int lines_to_shift)  {}
        // threads
        virtual int GetThreadsCount() const  {return 0;}
        virtual const cbThread& GetThread(int index) const  {}
        virtual bool SwitchToThread(int thread_number)  {return false;}

        // watches
        virtual cbWatch* AddWatch(const wxString& symbol)  {return NULL;}
        virtual void DeleteWatch(cbWatch *watch)  {}
        virtual bool HasWatch(cbWatch *watch)  {return false;}
        virtual void ShowWatchProperties(cbWatch *watch)  {}
        virtual bool SetWatchValue(cbWatch *watch, const wxString &value)  {return false;}
        virtual void ExpandWatch(cbWatch *watch)  {}
        virtual void CollapseWatch(cbWatch *watch)  {}

        virtual void OnWatchesContextMenu(wxMenu &menu, const cbWatch &watch, wxObject *property) {};

        virtual void SendCommand(const wxString& cmd, bool debugLog)  {}

        virtual void AttachToProcess(const wxString& pid)  {}
        virtual void DetachFromProcess()  {}
        virtual bool IsAttachedToProcess() const;

        virtual void GetCurrentPosition(wxString &filename, int &line)  {filename=m_curfile;line=m_curline;}

        virtual void ConvertDirectory(wxString& str, wxString base = _T(""), bool relative = true) {}
        virtual cbProject* GetProject() {return NULL;}
        virtual void ResetProject() {}
        virtual void CleanupWhenProjectClosed(cbProject *project) {}

        virtual void RequestUpdate(DebugWindows window) {}

// Debugger Plugin Specific Virtuals

        virtual bool Debug(bool breakOnEntry);
		virtual void Continue();
		virtual void Next();
        virtual void NextInstruction();
		virtual void Step();
        virtual void StepIntoInstruction();
        virtual void StepOut();
		virtual void Stop();
		virtual void Break();
        virtual bool RunToCursor(const wxString& filename, int line, const wxString& line_text);
        virtual void SetNextStatement(const wxString& filename, int line);
        bool IsRunning() const { return m_DebuggerActive; } /** Is the plugin currently debugging? */
        int GetExitCode() const { return 0; }
// Misc Plugin Virtuals
        virtual int Configure(); /** Invoke configuration dialog. */
        virtual int GetConfigurationPriority() const { return 50; }
        virtual int GetConfigurationGroup() const { return cgUnknown; }
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent); /** Return plugin's configuration panel.*/
        virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; } /** Return plugin's configuration panel for projects.*/
        virtual void BuildMenu(wxMenuBar* menuBar); /** add plugin items to the main menu bar*/
        virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0); /** add context menu items for the plugin*/

        void OnValueTooltip(CodeBlocksEvent& event);
        void SetWatchTooltip(const wxString &tip, int definition_length);

    protected:
//        virtual void OnAttach();
//        virtual void OnRelease(bool appShutDown);

/// Non-boiler plate methods
    public:
        void UpdateMenu();
        void UpdateConfig() {;}
        void CreateMenu();

    private:
        void OnSettings(wxCommandEvent& event);
        void OnSubMenuSelect(wxUpdateUIEvent& event);
        void OnSetTarget(wxCommandEvent& event);
        void OnRunTarget(wxCommandEvent& event);
        void OnDebugTarget(wxCommandEvent& event);
        void OnRun(wxCommandEvent& event);
        void OnRunPiped(wxCommandEvent &event);
        void OnStep(wxCommandEvent &event);
        void OnStop(wxCommandEvent &event);
        void OnNext(wxCommandEvent &event);
        void OnContinue(wxCommandEvent &event);
        void OnSendCommand(wxCommandEvent &event);
        void OnTerminatePipedProcess(wxProcessEvent &event);
        void OnPipedOutput(wxCommandEvent& event);
        void OnIdle(wxIdleEvent& event);
        void OnTimer(wxTimerEvent& event);
        void OnViewWatch(wxCommandEvent& event);
        void OnUpdateWatch(wxCommandEvent& event);
        void ReadPluginConfig();
        void WritePluginConfig();
        bool IsPythonFile(const wxString &file) const;

        void ClearActiveMarkFromAllEditors();
        //void SyncEditor(const wxString& filename, int line, bool setMarker=true);

        bool DispatchCommands(const wxString& cmd, int cmdtype=DBGCMDTYPE_FLOWCONTROL, bool poll=true);
        wxString AssembleBreakpointCommands();
        wxString AssembleWatchCommands();
        wxString AssembleAliasCommands();

        wxMenu *LangMenu;  // pointer to the interpreters menu
//        int m_interpnum;
        wxString m_PythonFileExtensions;

        // Information about current debug location
        unsigned long m_curline;
        wxString m_curfile;
        bool m_changeposition;

        long m_watch_tooltip_pos;

        // Output from the debugger and program held in buffers
        wxString m_outbuf; //contains buffer of streamed output from Python console since last action
        int m_bufpos; //search position in console output buffer (used to check whether processing is finished)
        wxString m_outdebugbuf; //contains buffer of streamed output from Python console since last action
        int m_debugbufpos; //search position in console output buffer (used to check whether processing is finished)
        wxString m_outprogbuf; //contains buffer of streamed output from Python console since last action
        int m_progbufpos; //search position in console output buffer (used to check whether processing is finished)
        wxOutputStream *m_ostream;
        wxInputStream *m_istream;
        wxProcess *m_pp;
        long m_pid;
        wxTimer m_TimerPollDebugger;
        int m_DebugCommandCount; //number of commands dispatched to the debugger that have yet to be handled
        std::list<PythonCmdDispatchData> m_DispatchedCommands;

        // breakpoint list
        BPList m_bplist;
        StackInfo m_stackinfo;

        bool m_DebuggerActive;
        wxString m_DefaultInterpreter;
        wxString m_DefaultDebugCmdLine;
        wxString m_debugfile; // file and line of current debug code position
        wxString m_debugline;

        TextCtrlLogger *m_DebugLog; // pointer to the text log (initialized with OnAttach)
        int m_DebugLogPageIndex; //page index of the debug log

        DebuggerWatch *m_WatchDlg;
        wxString m_watchstr;

        wxString m_RunTarget;
        bool m_RunTargetSelected;
        wxToolBar *m_pTbar;

        DECLARE_EVENT_TABLE();
};

#endif // PyDebugger_H_INCLUDED
