#include <wx/menu.h>
#include <wx/regex.h>

#include <configurationpanel.h>

//#include "ConfigDialog.h"
#ifdef TOOLSPLUSLINK
#include "ToolsPlus.h"
#endif

#include "PythonInterpreter.h"

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<PythonInterpreter> reg(_T("PythonInterpreter"));
}


bool WildCardListMatch(wxString list, wxString name)
{
    if(list==_T("")) //any empty list matches everything by default
        return true;
    wxString wildlist=list;
    wxString wild=list.BeforeFirst(';');
    while(wildlist!=_T(""))
    {
        if(wild!=_T("") && ::wxMatchWild(wild,name))
            return true;
        wildlist=wildlist.AfterFirst(';');
        wild=wildlist.BeforeFirst(';');
    }
    return false;
}


int ID_INTERP_WINDOW_TOGGLE=wxNewId();

// events handling
BEGIN_EVENT_TABLE(PythonInterpreter, cbPlugin)
	// add any events you want to handle here
    EVT_MENU(ID_INTERP_WINDOW_TOGGLE,PythonInterpreter::OnToggleInterpreterWindow)
    EVT_UPDATE_UI(wxID_ANY/*ID_INTERP_WINDOW_TOGGLE*/, PythonInterpreter::OnUpdateUI)
    EVT_COMMAND(0,wxEVT_SHELL_ADD_CLICKED, PythonInterpreter::AddNewInterpreter)
END_EVENT_TABLE()


void PythonInterpreter::OnUpdateUI(wxUpdateUIEvent& event)
{
#ifndef TOOLSPLUSLINK
    if(m_ViewMenu)
    {
        m_ViewMenu->Check(ID_INTERP_WINDOW_TOGGLE,IsWindowReallyShown(m_shellmgr));
        // allow other UpdateUI handlers to process this event
        // *very* important! don't forget it...
        event.Skip();
    }
#endif // TOOLSPLUSLINK
}

void PythonInterpreter::OnToggleInterpreterWindow(wxCommandEvent &event)
{
#ifndef TOOLSPLUSLINK
    CodeBlocksDockEvent evt(event.IsChecked()? cbEVT_SHOW_DOCK_WINDOW : cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_shellmgr;
    Manager::Get()->ProcessEvent(evt);
#endif // TOOLSPLUSLINK
}

// constructor
PythonInterpreter::PythonInterpreter()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
//    if(!Manager::LoadResource(_T("PythonInterpreter.zip")))
//    {
//        NotifyMissingFile(_T("PythonInterpreter.zip"));
//    }
}

cbConfigurationPanel* PythonInterpreter::GetConfigurationPanel(wxWindow* parent)
{
//    MyDialog* dlg = new MyDialog(this, *m_pKeyProfArr, parent,
//        wxT("Keybindings"), mode);
    return 0;
//    return new ConfigDialog(parent, this);
}

// destructor
PythonInterpreter::~PythonInterpreter()
{

}

void PythonInterpreter::OnAttach()
{
    m_ViewMenu = 0;
#ifndef TOOLSPLUSLINK
    m_shellmgr = new ShellManager(Manager::Get()->GetAppWindow());

    CodeBlocksDockEvent evt(cbEVT_ADD_DOCK_WINDOW);
    evt.name = _T("PythonWindow");
    evt.title = _("Python Interpreters");
    evt.pWindow = m_shellmgr;
    evt.dockSide = CodeBlocksDockEvent::dsFloating;
    evt.desiredSize.Set(400, 300);
    evt.floatingSize.Set(400, 300);
    evt.minimumSize.Set(200, 150);
    Manager::Get()->ProcessEvent(evt);

    //TODO: Add UI to open a terminal instead of opening on attach
//    wxArrayString as;
//    m_shellmgr->LaunchProcess(_T(""),_T("Python"),_("Python Interpreter"),as);
#endif
}

void PythonInterpreter::AddNewInterpreter(wxCommandEvent &event)
{
    wxArrayString as;
    m_shellmgr->LaunchProcess(_T(""),_T("Python"),_("Python Interpreter"),as);
}


void PythonInterpreter::OnRelease(bool appShutDown)
{
#ifndef TOOLSPLUSLINK
    if (m_shellmgr) //remove the Shell Terminals Notebook from its dockable window and delete it
    {
        CodeBlocksDockEvent evt(cbEVT_REMOVE_DOCK_WINDOW);
        evt.pWindow = m_shellmgr;
        Manager::Get()->ProcessEvent(evt);
        m_shellmgr->Destroy();
    }
    m_shellmgr = 0;
#endif
}

int PythonInterpreter::Execute()
{
#ifdef TOOLSPLUSLINK
    ToolsPlus *tp = dynamic_cast<ToolsPlus*>(Manager::Get()->GetPluginManager()->FindPluginByName(_T("ToolsPlus")));
    wxArrayString as;
    if (tp && tp->IsAttached())
        return tp->LaunchProcess(_T(""),_T("Python"),_T("Python Interpreter"),as);
    else
        return -1;
#else
    CodeBlocksDockEvent evt(cbEVT_SHOW_DOCK_WINDOW);
    evt.pWindow = m_shellmgr;
    Manager::Get()->ProcessEvent(evt);
    wxArrayString as;
    return m_shellmgr->LaunchProcess(_T(""),_T("Python"),_("Python Interpreter"),as);
#endif
}

int PythonInterpreter::Configure()
{
	//create and display the configuration dialog for the plugin
	cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("Python Language Properties"));
	cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
	if (panel)
	{
		dlg.AttachConfigurationPanel(panel);
		PlaceWindow(&dlg);
		return dlg.ShowModal() == wxID_OK ? 0 : -1;
	}
	return -1;
}


void PythonInterpreter::BuildMenu(wxMenuBar* menuBar)
{
	//The application is offering its menubar for your plugin,
	//to add any menu items you want...
	//Append any items you need in the menu...
	//NOTE: Be careful in here... The application's menubar is at your disposal.
#ifndef TOOLSPLUSLINK
	int pos = menuBar->FindMenu(_("&View"));
	if(pos==wxNOT_FOUND)
        return;
    m_ViewMenu = menuBar->GetMenu(pos);
    int id = m_ViewMenu->FindItem(_("S&cript console"));
	if(id==wxNOT_FOUND)
    {
        m_ViewMenu = 0;
        return;
    }
    size_t ipos;
    m_ViewMenu->FindChildItem(id,&ipos);
    m_ViewMenu->InsertCheckItem(ipos+1,ID_INTERP_WINDOW_TOGGLE,_("Python &interpreters"),_("Show or hide the python interpreter window"));
#endif
}

void PythonInterpreter::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
	//Some library module is ready to display a pop-up menu.
	//Check the parameter \"type\" and see which module it is
	//and append any items you need in the menu...
	//TIP: for consistency, add a separator as the first item...
}

bool PythonInterpreter::BuildToolBar(wxToolBar* toolBar)
{
//    if (!IsAttached() || !toolBar)
//        return false;
    return false;
}
