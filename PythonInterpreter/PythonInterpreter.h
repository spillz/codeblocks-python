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
#include <cbplugin.h> // for "class cbPlugin"
//#include "ConfigDialog.h"
//#include "dialogs.h"

//class ConfigDialog;

//class InterpreterCollection;


class PyPlugin : public cbToolPlugin
{
    public:
		/** Constructor. */
        PyPlugin();
		/** Destructor. */
        virtual ~PyPlugin();

// Misc Plugin Virtuals
        virtual int Configure(); /** Invoke configuration dialog. */
        virtual int GetConfigurationPriority() const { return 50; }
        virtual int GetConfigurationGroup() const { return cgUnknown; }
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent); /** Return plugin's configuration panel.*/
        virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0; } /** Return plugin's configuration panel for projects.*/
        virtual void BuildMenu(wxMenuBar* menuBar); /** add plugin items to the main menu bar*/
        virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = 0); /** add context menu items for the plugin*/
        virtual bool BuildToolBar(wxToolBar* toolBar); /** register and add plugin toolbars*/
        virtual int Execute();
    protected:
        virtual void OnAttach();
        virtual void OnRelease(bool appShutDown);

/// Non-boiler plate methods
    public:

    private:

        DECLARE_EVENT_TABLE();
};

#endif // PyPlugin_H_INCLUDED
