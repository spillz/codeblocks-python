/***************************************************************
 * Name:      ShellExtensions
 * Purpose:   Code::Blocks plugin
 * Author:    Damien Moore ()
 * Created:   2006-09-28
 * Copyright: Damien Moore
 * License:   GPL
 **************************************************************/

#ifndef SHELLEXTENSIONS_H_INCLUDED
#define SHELLEXTENSIONS_H_INCLUDED

// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h>
#include <cbplugin.h> // for "class cbPlugin"
#include "shellproperties.h"
#include "CmdConfigDialog.h"
#include "ShellCtrl.h"
#include "FileExplorer.h"

class ShellExtensions : public cbPlugin
{
    public:
		/** Constructor. */
        ShellExtensions();
		/** Destructor. */
        virtual ~ShellExtensions();

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
        virtual bool BuildToolBar(wxToolBar* toolBar){ return false; }
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
        void CreateMenu();
        void AddModuleMenuEntry(wxMenu *modmenu,int entrynum, int idref);
        void ShowConsole();
        void HideConsole();
        CommandCollection m_ic; //object holding interpreter properties

    private:
        void OnSettings(wxCommandEvent& event);
        void OnSubMenuSelect(wxUpdateUIEvent& event);
        void OnSetTarget(wxCommandEvent& event);
        void OnSetDirTarget(wxCommandEvent& event);
        void OnSetMultiTarget(wxCommandEvent& event);
        void OnRunTarget(wxCommandEvent& event);
        void OnRun(wxCommandEvent& event);
        void OnRunPiped(wxCommandEvent &event);
        void OnShowConsole(wxCommandEvent& event);
        void OnUpdateUI(wxUpdateUIEvent& event);

        wxMenu *m_LangMenu;  // pointer to the interpreters menu
        unsigned int m_interpnum;
        unsigned int m_contextactions[10];
        wxString m_wildcard;

        ShellCommandMenuVec m_contextvec;
        ShellCommandMenuVec m_menuvec;

        wxString m_RunTarget;

        // data structures used to pipe console output to a dockable window (m_shellmgr)
        bool m_pipeoutput;
        wxOutputStream *m_ostream; //handle to console program output
        wxInputStream *m_istream; //handle to console program input
        wxProcess *m_pp; // handle to the console program
        wxTimer m_timerpollconsole; //timer object used to regularly poll the piped console program for new output
        // Output from the piped console program held in buffers
        wxString m_outbuf; //contains buffer of streamed output from console
        int m_bufpos; //search position in console output buffer (used to check whether processing is finished)

        ShellManager *m_shellmgr;
        FileExplorer *m_fe;

        DECLARE_EVENT_TABLE();
};

#endif // SHELLEXTENSIONS_H_INCLUDED
