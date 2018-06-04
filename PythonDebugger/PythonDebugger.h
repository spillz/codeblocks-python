/***************************************************************
 * Name:      Python Debugger Plugin
 * Purpose:   Code::Blocks plugin
 * Author:    Damien Moore ()
 * Created:   2006-09-28
 * Copyright: Damien Moore
 * License:   GPL
 **************************************************************/

#ifndef PYTHONDEBUGGER_H_INCLUDED
#define PYTHONDEBUGGER_H_INCLUDED

#include <list>

#include <wx/stream.h>
#include <wx/timer.h>

#include <cbplugin.h> // for "class cbPlugin/cbDebuggerPlugin"
#include <loggers.h>
#include <logger.h>

#include <debuggermanager.h>
#include "debuggeroptionsdlg.h"


#define DBGCMDTYPE_FLOWCONTROL 1
#define DBGCMDTYPE_BREAKPOINT 2
#define DBGCMDTYPE_WATCHEXPRESSION 3
#define DBGCMDTYPE_WATCHTOOLTIP 4
#define DBGCMDTYPE_WATCHGETCHILDREN 5
#define DBGCMDTYPE_EVALUATE 6
#define DBGCMDTYPE_USERCOMMAND 7
#define DBGCMDTYPE_CALLSTACK 8
#define DBGCMDTYPE_OTHER 9

class wxProcessEvent;
class wxTimerEvent;

typedef std::set<int> BPLtype;

typedef std::vector<cb::shared_ptr<cbStackFrame> > StackList;

class PyBreakpoint:public cbBreakpoint
{
    public:
        PyBreakpoint(wxString file, int line)
        {
            m_file=file;
            m_line=line;
        }
        virtual ~PyBreakpoint() {}
        void SetEnabled(bool flag) {m_enabled = flag;}
        wxString GetLocation() const {return m_file;}
        int GetLine() const {return m_line;}
        wxString GetLineString() const {return wxString::Format(wxT("%i"),m_line);}
        wxString GetType() const {return m_type;}
        wxString GetInfo() const {return m_info;}
        bool IsEnabled() const {return m_enabled;}
        bool IsVisibleInEditor()const  {return true;}
        bool IsTemporary() const {return false;}
    private:
        bool m_enabled;
        wxString m_file;
        int m_line;
        wxString m_type;
        wxString m_info;
};


typedef cb::shared_ptr<PyBreakpoint> Pointer;
typedef std::vector<Pointer> BPList;

class PythonWatch :public cbWatch
{
    public:
        PythonWatch(wxString const &symbol) :
            m_symbol(symbol),
            m_has_been_expanded(false)
        {
        }
        typedef cb::shared_ptr<PythonWatch> Pointer;
        virtual ~PythonWatch() {}

        void Reset()
        {
            m_id = m_type = m_value = wxEmptyString;
            m_has_been_expanded = false;
            RemoveChildren();
            Expand(false);
        }

        wxString const & GetID() const { return m_id; }
        void SetID(wxString const &id) { m_id = id; }

        bool HasBeenExpanded() const { return m_has_been_expanded; }
        void SetHasBeenExpanded(bool expanded) { m_has_been_expanded = expanded; }
        virtual void GetSymbol(wxString &symbol) const { symbol = m_symbol; }
        virtual void GetValue(wxString &value) const { value = m_value; }
        virtual bool SetValue(const wxString &value) { m_value = value; return true; }
        virtual void GetFullWatchString(wxString &full_watch) const { full_watch = m_value; }
        virtual void GetType(wxString &type) const { type = m_type; }
        virtual void SetType(const wxString &type) { m_type = type; }

        virtual wxString const & GetDebugString() const
        {
            m_debug_string = m_id + wxT("->") + m_symbol + wxT(" = ") + m_value;
            return m_debug_string;
        }
	protected:
        virtual void DoDestroy() {}
    private:
        wxString m_id;
        wxString m_symbol;
        wxString m_value;
        wxString m_type;
        mutable wxString m_debug_string;
        bool m_has_been_expanded;
};

typedef std::vector<PythonWatch::Pointer> PythonWatchesContainer;


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

class PythonDebugger : public cbDebuggerPlugin
{
    public:
		/** Constructor. */
        PythonDebugger();
		/** Destructor. */
        virtual ~PythonDebugger();

        virtual void OnAttachReal();
        virtual void OnReleaseReal(bool appShutDown);

        virtual void SetupToolsMenu(wxMenu &) {}
        virtual bool SupportsFeature(cbDebuggerFeature::Flags f);

        /** @brief Is the plugin stopped on breakpoint? */
        virtual bool IsStopped() const  {return m_DebugCommandCount==0;}

        /** @brief Get the exit code of the last debug process. */
//        virtual int GetExitCode() const  {}

        // stack frame calls;
        virtual int GetStackFrameCount() const;
        virtual cb::shared_ptr<const cbStackFrame> GetStackFrame(int index) const;
        virtual void SwitchToFrame(int number);
        virtual int GetActiveStackFrame() const;

        // breakpoints calls
        /** @brief Request to add a breakpoint.
          * @param file The file to add the breakpoint based on a file/line pair.
          * @param line The line number to put the breakpoint in @c file.
          * @return True if succeeded, false if not.
          */
        virtual cb::shared_ptr<cbBreakpoint> AddBreakpoint(const wxString& filename, int line);

        /** @brief Request to add a breakpoint based on a data expression.
          * @param dataExpression The data expression to add the breakpoint.
          * @return True if succeeded, false if not.
          */
        virtual cb::shared_ptr<cbBreakpoint> AddDataBreakpoint(const wxString& dataExpression) {return cb::shared_ptr<cbBreakpoint>();}
        virtual int GetBreakpointsCount() const  {return m_bplist.size();}
        virtual cb::shared_ptr<cbBreakpoint> GetBreakpoint(int index);
        virtual cb::shared_ptr<const cbBreakpoint> GetBreakpoint(int index) const;
        virtual void UpdateBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint)  {}
        virtual void DeleteBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint);
        virtual void DeleteAllBreakpoints();
        virtual void ShiftBreakpoint(int index, int lines_to_shift)  {}
        // threads
        virtual int GetThreadsCount() const  {return 0;}
        virtual cb::shared_ptr<const cbThread> GetThread(int index) const  { cb::shared_ptr<const cbThread> p(new cbThread()); return p;}
        virtual bool SwitchToThread(int thread_number)  {return false;}

        // watches
        virtual cb::shared_ptr<cbWatch> AddWatch(const wxString& symbol);
        virtual void DeleteWatch(cb::shared_ptr<cbWatch> watch);
        virtual bool HasWatch(cb::shared_ptr<cbWatch> watch);
        virtual void ShowWatchProperties(cb::shared_ptr<cbWatch> watch);
        virtual bool SetWatchValue(cb::shared_ptr<cbWatch> watch, const wxString &value);
        virtual void ExpandWatch(cb::shared_ptr<cbWatch> watch);
        virtual void CollapseWatch(cb::shared_ptr<cbWatch> watch);
        virtual void UpdateWatch(cb::shared_ptr<cbWatch> watch);
        virtual void OnWatchesContextMenu(wxMenu &menu, const cbWatch &watch, wxObject *property);

        virtual void SendCommand(const wxString& cmd, bool debugLog);

        virtual void AttachToProcess(const wxString& pid)  {}
        virtual void DetachFromProcess()  {}
        virtual bool IsAttachedToProcess() const;

        virtual void GetCurrentPosition(wxString &filename, int &line)  {filename=m_curfile;line=m_curline;}

        virtual void ConvertDirectory(wxString& str, wxString base = _T(""), bool relative = true) {}
        virtual cbProject* GetProject() {return NULL;}
        virtual void ResetProject() {}
        virtual void CleanupWhenProjectClosed(cbProject *project) {}

        virtual void RequestUpdate(DebugWindows window);

// Debugger Plugin Specific Virtuals

        virtual bool IsBusy() const {return m_TimerPollDebugger.IsRunning();}
        virtual void EnableBreakpoint(cb::shared_ptr<cbBreakpoint> breakpoint, bool enable) {}
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
        virtual cbDebuggerConfiguration* LoadConfig(const ConfigManagerWrapper &config) {return new PyDebuggerConfiguration(config);}
        virtual int GetConfigurationPriority() const { return 50; }
        virtual int GetConfigurationGroup() const { return cgUnknown; }
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent); /** Return plugin's configuration panel.*/
        virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; } /** Return plugin's configuration panel for projects.*/
        virtual void BuildMenu(wxMenuBar* menuBar) {} /** add plugin items to the main menu bar*/

        void OnValueTooltip(CodeBlocksEvent& event);
        void SetWatchTooltip(const wxString &tip, int definition_length);

        PyDebuggerConfiguration& GetActiveConfigEx()
        {
            return static_cast<PyDebuggerConfiguration&>(GetActiveConfig());
        }

/// Non-boiler plate methods
    public:
        void UpdateMenu();
        void UpdateConfig() {;}
        void CreateMenu();

    private:
        void OnTerminatePipedProcess(wxProcessEvent &event);
        void OnTimer(wxTimerEvent& event);
        bool IsPythonFile(const wxString &file) const;

        void ClearActiveMarkFromAllEditors();

        bool DispatchCommands(const wxString& cmd, int cmdtype=DBGCMDTYPE_FLOWCONTROL, bool poll=true);
        wxString AssembleBreakpointCommands();
        void DispatchWatchCommands();
        wxString AssembleAliasCommands();

        wxMenu *LangMenu;  // pointer to the interpreters menu
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
        StackInfo m_stackinfo; //call stack
        PythonWatchesContainer m_watchlist;
        PythonWatch::Pointer m_locals_watch;
        PythonWatch::Pointer m_functions_watch;
        PythonWatch::Pointer m_classes_watch;
        PythonWatch::Pointer m_modules_watch;

        bool m_DebuggerActive;
        wxString m_debugfile; // file and line of current debug code position
        wxString m_debugline;

        TextCtrlLogger *m_DebugLog; // pointer to the text log (initialized with OnAttach)
        int m_DebugLogPageIndex; //page index of the debug log

        wxString m_RunTarget;
        bool m_RunTargetSelected;
        wxToolBar *m_pTbar;

        DECLARE_EVENT_TABLE();
};

#endif // PYTHONDEBUGGER_H_INCLUDED
