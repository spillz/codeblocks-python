/***************************************************************
 * Name:      pluginxray
 * Purpose:   Code::Blocks plugin
 * Author:    Damien Moore ()
 * Created:   2006-09-28
 * Copyright: Damien Moore
 * License:   GPL
 **************************************************************/

#ifndef PyPlugin_H_INCLUDED
#define PyPlugin_H_INCLUDED

// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h>
#include <loggers.h>
#include <logger.h>
#include <cbplugin.h> // for "class cbPlugin"
#include "ConfigDialog.h"
#include "dialogs.h"


#define DBGCMDTYPE_FLOWCONTROL 1
#define DBGCMDTYPE_BREAKPOINT 2
#define DBGCMDTYPE_WATCHEXPRESSION 3
#define DBGCMDTYPE_EVALUATE 4
#define DBGCMDTYPE_USERCOMMAND 5
#define DBGCMDTYPE_OTHER 6

//class ConfigDialog;

//class InterpreterCollection;

typedef std::set<int> BPLtype;

struct FileBreakpoints
{
    wxString filename;
    BPLtype linenums;
};

typedef std::vector<FileBreakpoints> BPList;

struct PythonCmdDispatchData
{
    int type;
    wxString cmdtext;
};

class PyPlugin : public cbDebuggerPlugin
{
    public:
		/** Constructor. */
        PyPlugin();
		/** Destructor. */
        virtual ~PyPlugin();

// Debugger Plugin Specific Virtuals
		virtual bool AddBreakpoint(const wxString& file, int line);
		virtual bool AddBreakpoint(const wxString& functionSignature) {wxMessageBox(_T("Unhandled Pyplugin Event")); return true;}
		virtual bool RemoveBreakpoint(const wxString& file, int line);
		virtual bool RemoveBreakpoint(const wxString& functionSignature) {wxMessageBox(_T("Unhandled Pyplugin Event")); return true;}
		virtual bool RemoveAllBreakpoints(const wxString& file = wxEmptyString) {return true;}
		virtual void EditorLinesAddedOrRemoved(cbEditor* editor, int startline, int lines) {if(m_DebuggerActive) wxMessageBox(_T("Editing files containing breakpoints will result in loss of Python debugger synchronization"));}
		virtual int Debug(); /** Start a new debugging process. */
		virtual void Continue();
		virtual void Next();
		virtual void Step();
		virtual void Stop();
		virtual void Break() {}
        bool IsRunning() const { return false; } /** Is the plugin currently debugging? */
        int GetExitCode() const { return 0; }
// Misc Plugin Virtuals
        virtual int Configure(); /** Invoke configuration dialog. */
        virtual int GetConfigurationPriority() const { return 50; }
        virtual int GetConfigurationGroup() const { return cgUnknown; }
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent); /** Return plugin's configuration panel.*/
        virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; } /** Return plugin's configuration panel for projects.*/
        virtual void BuildMenu(wxMenuBar* menuBar); /** add plugin items to the main menu bar*/
        virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0); /** add context menu items for the plugin*/
        virtual bool BuildToolBar(wxToolBar* toolBar); /** register and add plugin toolbars*/
    protected:
        virtual void OnAttach();
        virtual void OnRelease(bool appShutDown);

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
        bool IsPythonFile(const wxString &file);

        void ClearActiveMarkFromAllEditors();
        void SyncEditor(const wxString& filename, int line, bool setMarker=true);

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
        wxTimer m_TimerPollDebugger;
        int m_DebugCommandCount; //number of commands dispatched to the debugger that have yet to be handled
        std::list<PythonCmdDispatchData> m_DispatchedCommands;

        // breakpoint list
        BPList m_bplist;

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

#endif // PyPlugin_H_INCLUDED
