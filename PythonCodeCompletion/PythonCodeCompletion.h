/***************************************************************
 * Name:      PythonCodeCompletion
 * Purpose:   Code::Blocks plugin
 * Author:    Damien Moore (damienlmoore@gmail.com)
 * Created:   2012-10-02
 * Copyright: Damien Moore
 * License:   GPL
 **************************************************************/

//TODO: Possible need to relaunch the py_server if it fails
//TODO: Let user customize the python command used to launch the server
//TODO: Code browsing pane


#ifndef PYTHON_CODE_COMPLETION_H_INCLUDED
#define PYTHON_CODE_COMPLETION_H_INCLUDED


#include <cbplugin.h> // for "class cbPlugin"
#include "xmlrpc_embedder.h"

class cbEditor;
class wxScintillaEvent;

class PythonCodeCompletion : public cbCodeCompletionPlugin
{
    public:
        enum StateType
        {
            STATE_NONE,
            STATE_COMPLETION_REQUEST,
            STATE_COMPLETION_RETURNED,
            STATE_CALLTIP_REQUEST,
            STATE_CALLTIP_RETURNED,
            STATE_DOC_REQUEST,
            STATE_DOC_RETURNED,
            STATE_GOTO_REQUEST,
            STATE_GOTO_RETURNED
        };
        PythonCodeCompletion();
        virtual ~PythonCodeCompletion();
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
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent){ return 0; }

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
        virtual bool BuildToolBar(wxToolBar* toolBar){ return false; }

        virtual CCProviderStatus GetProviderStatusFor(cbEditor* ed);

        virtual std::vector<CCToken> GetAutocompList(bool isAuto, cbEditor* ed, int& tknStart, int& tknEnd);
        /// returns html
        virtual wxString GetDocumentation(const CCToken& token);
        virtual std::vector<CCCallTip> GetCallTips(int pos, int style, cbEditor* ed, int& argsPos);
        virtual std::vector<CCToken> GetTokenAt(int pos, cbEditor* ed, bool& allowCallTip);
        /// dismissPopup is false by default
        virtual wxString OnDocumentationLink(wxHtmlLinkEvent& event, bool& dismissPopup);

    protected:
        /// Called when the plugin is enabled
        virtual void OnAttach();
        /// Called when the plugin is disabled
        virtual void OnRelease(bool appShutDown);

        //Handlers for the responses to asynchronous XMLRPC requests
        void OnCalltip(XmlRpcResponseEvent &event);
        void OnCompletePhrase(XmlRpcResponseEvent &event);
        void OnClickedGotoDefinition(wxCommandEvent& event);
        void OnGotoDefinition(XmlRpcResponseEvent &event);
        void OnXmlRpcTerminated(wxCommandEvent &event);
        void HandleError(XmlRpcResponseEvent &event, wxString message);

        // Additional implementation details
        // Helper for figuring out the position of calltips
        void GetCalltipPositions(cbEditor* editor, int pos, int &argsStartPos, int &argNumber);
        // Functions to make the XmlRpc server request
        void RequestCompletion(cbStyledTextCtrl *control, int pos, const wxString &filename);
        void RequestCallTip(cbStyledTextCtrl *control, int pos, const wxString &filename);
        wxString RequestDocString(int id);

    private:
        int m_state; // takes one of the values of the StateType enum (used to report current state of the engine)
        int m_request_submitted_count; // m_request_id is used to keep track of the active request and is used to prevent responses from stale requests from propagating
        int m_request_completed_count; // m_request_id is used to keep track of the active request and is used to prevent responses from stale requests from propagating
        int m_argsPos; // position of the call args in the active editor
        int m_argNumber; // zero-based numerical position of the cursor within the function call spec
        wxString GetExtraFile(const wxString &short_name);
        XmlRpcInstance *py_server; //Code Completion Server (a python process running an XMLRPC server)
        wxImageList* m_pImageList; //Type icons displayed in the code completion popup
        CCCallTip m_ActiveCalltipDef; //contains the call tip definition retrieved from the server
        wxArrayString m_comp_results; //contains an array of completion results retrieved from the server
        struct {
            int line;
            int column;
        } m_comp_position;
        DECLARE_EVENT_TABLE();
};

#endif // PYTHON_CODE_COMPLETION_H_INCLUDED
