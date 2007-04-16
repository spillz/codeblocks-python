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

#include <cbplugin.h> // for "class cbPlugin"
#include "sdk.h"
#include "ConfigDialog.h"
#include "Dialogs.h"


#define DBGCMDTYPE_FLOWCONTROL 1
#define DBGCMDTYPE_BREAKPOINT 2
#define DBGCMDTYPE_WATCHEXPRESSION 3
#define DBGCMDTYPE_EVALUATE 4
#define DBGCMDTYPE_USERCOMMAND 5
#define DBGCMDTYPE_OTHER 6

//class ConfigDialog;

//class InterpreterCollection;

/*
What does this plugin do?

a python debugger now runs on a simple test file
while the debugger is active can call next, step into, continue, stop, add and remove breakpoints (bps).

also offers the full feature set of the InterpetedLangs plugin (add/remove interpreters with various "actions")
- most of this functionality will be removed to make this a Python specific plugin

What it doesn't do but will do eventually
A) Debugger
1. restrict bp responses to python files
2. error handling from debugger (post mortem mode etc)
3. output streaming
4. Full UI interface with target selection, toolbars, breakpoints etc
5. Add/remove breakpoints even while debugger is not in an active session (also add bps that are present in the debugger when debug session begins - problematic when has preselected a whole bunch of breakpoints in other python files that can't ever be encountered in this session)
6. Handle effect of add/remove lines on breakpoints
7. Custom control to watch variables and expressions
8. Config dialog to handle Regexes and text strings for passing commands to the debugger and interpreting its output
(I'll add other to this list as I think of them)
B) Python Projects (Easy)
C) Python Source Browser and Code Completion (Medium/Hard)
*/

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
//		cbDebuggerPlugin();

		/** @brief Request to add a breakpoint.
		  * @param file The file to add the breakpoint based on a file/line pair.
		  * @param line The line number to put the breakpoint in @c file.
		  * @return True if succeeded, false if not.
		  */
		virtual bool AddBreakpoint(const wxString& file, int line);

		/** @brief Request to add a breakpoint based on a function signature.
		  * @param functionSignature The function signature to add the breakpoint.
		  * @return True if succeeded, false if not.
		  */
		virtual bool AddBreakpoint(const wxString& functionSignature) {wxMessageBox(_T("Unhandled Pyplugin Event")); return true;}

		/** @brief Request to remove a breakpoint based on a file/line pair.
		  * @param file The file to remove the breakpoint.
		  * @param line The line number the breakpoint is in @c file.
		  * @return True if succeeded, false if not.
		  */
		virtual bool RemoveBreakpoint(const wxString& file, int line);

		/** @brief Request to remove a breakpoint based on a function signature.
		  * @param functionSignature The function signature to remove the breakpoint.
		  * @return True if succeeded, false if not.
		  */
		virtual bool RemoveBreakpoint(const wxString& functionSignature) {wxMessageBox(_T("Unhandled Pyplugin Event")); return true;}

		/** @brief Request to remove all breakpoints from a file.
		  * @param file The file to remove all breakpoints in. If the argument is empty, all breakpoints are removed from all files.
		  * @return True if succeeded, false if not.
		  */
		virtual bool RemoveAllBreakpoints(const wxString& file = wxEmptyString) {return true;}

		/** @brief Notify the debugger that lines were added or removed in an editor.
		  * This causes the debugger to keep the breakpoints list in-sync with the
		  * editors (i.e. what the user sees).
		  * @param editor The editor in question.
		  * @param startline The starting line this change took place.
		  * @param lines The number of lines added or removed. If it's a positive number,
		  *              lines were added. If it's a negative number, lines were removed.
		  */
		virtual void EditorLinesAddedOrRemoved(cbEditor* editor, int startline, int lines) {if(m_DebuggerActive) wxMessageBox(_T("Editing files containing breakpoints will result in loss of Python debugger synchronization"));}

		/** @brief Start a new debugging process. */
		virtual int Debug();

		/** @brief Continue running the debugged program. */
		virtual void Continue();

		/** @brief Execute the next instruction and return control to the debugger. */
		virtual void Next();

		/** @brief Execute the next instruction, stepping into function calls if needed, and return control to the debugger. */
		virtual void Step();

		/** @brief Stop the debugging process. */
		virtual void Stop();

        /** @brief Is the plugin currently debugging? */
        bool IsRunning() const { return false; }
        int GetExitCode() const { return 0; }


// Plugin Virtuals

		/** Invoke configuration dialog. */
        virtual int Configure();

        /** Return the plugin's configuration priority.
          * This is a number (default is 50) that is used to sort plugins
          * in configuration dialogs. Lower numbers mean the plugin's
          * configuration is put higher in the list.
          */
        virtual int GetConfigurationPriority() const { return 50; }

        /** Return the configuration group for this plugin. Default is cgUnknown.
          * Notice that you can logically OR more than one configuration groups,
          * so you could set it, for example, as "cgCompiler | cgContribPlugin".
          */
        virtual int GetConfigurationGroup() const { return cgUnknown; }

		/** Return plugin's configuration panel.
		  * @param parent The parent window.
		  * @return A pointer to the plugin's cbConfigurationPanel. It is deleted by the caller.
		  */
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent);

		/** Return plugin's configuration panel for projects.
		  * The panel returned from this function will be added in the project's
		  * configuration dialog.
		  * @param parent The parent window.
		  * @param project The project that is being edited.
		  * @return A pointer to the plugin's cbConfigurationPanel. It is deleted by the caller.
		  */
        virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; }

		/** This method is called by Code::Blocks and is used by the plugin
		  * to add any menu items it needs on Code::Blocks's menu bar.\n
		  * It is a pure virtual method that needs to be implemented by all
		  * plugins. If the plugin does not need to add items on the menu,
		  * just do nothing ;)
		  * @param menuBar the wxMenuBar to create items in
		  */
        virtual void BuildMenu(wxMenuBar* menuBar);

		/** This method is called by Code::Blocks core modules (EditorManager,
		  * ProjectManager etc) and is used by the plugin to add any menu
		  * items it needs in the module's popup menu. For example, when
		  * the user right-clicks on a project file in the project tree,
		  * ProjectManager prepares a popup menu to display with context
		  * sensitive options for that file. Before it displays this popup
		  * menu, it asks all attached plugins (by asking PluginManager to call
		  * this method), if they need to add any entries
		  * in that menu. This method is called.\n
		  * If the plugin does not need to add items in the menu,
		  * just do nothing ;)
		  * @param type the module that's preparing a popup menu
		  * @param menu pointer to the popup menu
		  * @param data pointer to FileTreeData object (to access/modify the file tree)
		  */
        virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0);

		/** This method is called by Code::Blocks and is used by the plugin
		  * to add any toolbar items it needs on Code::Blocks's toolbar.\n
		  * It is a pure virtual method that needs to be implemented by all
		  * plugins. If the plugin does not need to add items on the toolbar,
		  * just do nothing ;)
		  * @param toolBar the wxToolBar to create items on
		  * @return The plugin should return true if it needed the toolbar, false if not
		  */
        virtual bool BuildToolBar(wxToolBar* toolBar);
    protected:
		/** Any descendent plugin should override this virtual method and
		  * perform any necessary initialization. This method is called by
		  * Code::Blocks (PluginManager actually) when the plugin has been
		  * loaded and should attach in Code::Blocks. When Code::Blocks
		  * starts up, it finds and <em>loads</em> all plugins but <em>does
		  * not</em> activate (attaches) them. It then activates all plugins
		  * that the user has selected to be activated on start-up.\n
		  * This means that a plugin might be loaded but <b>not</b> activated...\n
		  * Think of this method as the actual constructor...
		  */
        virtual void OnAttach();

		/** Any descendent plugin should override this virtual method and
		  * perform any necessary de-initialization. This method is called by
		  * Code::Blocks (PluginManager actually) when the plugin has been
		  * loaded, attached and should de-attach from Code::Blocks.\n
		  * Think of this method as the actual destructor...
		  * @param appShutDown If true, the application is shutting down. In this
		  *         case *don't* use Manager::Get()->Get...() functions or the
		  *         behaviour is undefined...
		  */
        virtual void OnRelease(bool appShutDown);




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
        std::vector<wxString> m_PythonFileExtensions;

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

        SimpleTextLog *m_DebugLog; // pointer to the text log (initialized with OnAttach)
        int m_DebugLogPageIndex; //page index of the debug log

        DebuggerWatch *m_WatchDlg;
        wxString m_watchstr;

        wxString m_RunTarget;
        bool m_RunTargetSelected;
        wxToolBar *m_pTbar;

        DECLARE_EVENT_TABLE();
};

#endif // PyPlugin_H_INCLUDED
