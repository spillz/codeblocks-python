#include "PyPlugin.h"

#include <configurationpanel.h>
#include <wx/regex.h>

//#include "ConfigDialog.h"
#ifdef TOOLSPLUSLINK
#include "ToolsPlus.h"
#endif

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<PyPlugin> reg(_T("PythonInterpreter"));
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


int ID_LangMenu_Settings=wxNewId();
int ID_LangMenu_Run=wxNewId();
int ID_LangMenu_DebugSendCommand=wxNewId();
int ID_LangMenu_ShowWatch=wxNewId();
int ID_LangMenu_UpdateWatch=wxNewId();

int ID_PipedProcess=wxNewId();
int ID_TimerPollDebugger=wxNewId();

//assign menu IDs to correspond with toolbar buttons
int ID_LangMenu_RunPiped = wxNewId();//XRCID("idPyDebuggerMenuDebug");

// events handling
BEGIN_EVENT_TABLE(PyPlugin, cbToolPlugin)
	// add any events you want to handle here
//    EVT_MENU(ID_LangMenu_Run,PyPlugin::OnRun)
//    EVT_MENU(ID_LangMenu_RunPiped,PyPlugin::OnDebugTarget)
//    EVT_MENU(XRCID("idPyDebuggerMenuDebug"),PyPlugin::OnContinue)
//    EVT_MENU(XRCID("idPyDebuggerMenuNext"),PyPlugin::OnNext)
//    EVT_MENU(XRCID("idPyDebuggerMenuStep"),PyPlugin::OnStep)
//    EVT_MENU(XRCID("idPyDebuggerMenuStop"),PyPlugin::OnStop)
//    EVT_MENU(ID_LangMenu_DebugSendCommand,PyPlugin::OnSendCommand)
//    EVT_MENU(ID_LangMenu_ShowWatch,PyPlugin::OnViewWatch)
//    EVT_MENU(ID_LangMenu_UpdateWatch,PyPlugin::OnUpdateWatch)
//    EVT_END_PROCESS(ID_PipedProcess, PyPlugin::OnTerminatePipedProcess)
//    EVT_TIMER(ID_TimerPollDebugger, PyPlugin::OnTimer)
END_EVENT_TABLE()



// constructor
PyPlugin::PyPlugin()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
//    if(!Manager::LoadResource(_T("PyPlugin.zip")))
//    {
//        NotifyMissingFile(_T("PyPlugin.zip"));
//    }
}

cbConfigurationPanel* PyPlugin::GetConfigurationPanel(wxWindow* parent)
{
//    MyDialog* dlg = new MyDialog(this, *m_pKeyProfArr, parent,
//        wxT("Keybindings"), mode);
    return 0;
//    return new ConfigDialog(parent, this);
}

// destructor
PyPlugin::~PyPlugin()
{

}

void PyPlugin::OnAttach()
{
	// do whatever initialization you need for your plugin
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be TRUE...
	// You should check for it in other functions, because if it
	// is FALSE, it means that the application did *not* "load"
	// (see: does not need) this plugin...
}

void PyPlugin::OnRelease(bool appShutDown)
{
	// do de-initialization for your plugin
	// if appShutDown is false, the plugin is unloaded because Code::Blocks is being shut down,
	// which means you must not use any of the SDK Managers
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be FALSE...
}

int PyPlugin::Execute()
{
#ifdef TOOLSPLUSLINK
    ToolsPlus *tp = dynamic_cast<ToolsPlus*>(Manager::Get()->GetPluginManager()->FindPluginByName(_T("ToolsPlus")));
    wxArrayString as;
    if (tp && tp->IsAttached())
        return tp->LaunchProcess(_T(""),_T("Python"),_T("Python Interpreter"),as);
    else
        return -1;
#else
    return 0;
#endif
}

int PyPlugin::Configure()
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


void PyPlugin::BuildMenu(wxMenuBar* menuBar)
{
	//The application is offering its menubar for your plugin,
	//to add any menu items you want...
	//Append any items you need in the menu...
	//NOTE: Be careful in here... The application's menubar is at your disposal.
//	LangMenu=new wxMenu;
//	CreateMenu();
//	int pos = menuBar->FindMenu(_T("Plugins"));
//	if(pos!=wxNOT_FOUND)
//        menuBar->Insert(pos, LangMenu, _T("P&yDebug"));
//    else
//    {
//        delete LangMenu;
//        LangMenu=0;
//    }
}

void PyPlugin::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
	//Some library module is ready to display a pop-up menu.
	//Check the parameter \"type\" and see which module it is
	//and append any items you need in the menu...
	//TIP: for consistency, add a separator as the first item...
//	if(type==mtProjectManager)
//	{
//	    if(data)
//	    {
//            if(data->GetKind()==FileTreeData::ftdkFile)
//            {
//                ProjectFile *f=data->GetProjectFile();
//                if(f)
//                {
//                    wxString name=f->file.GetFullPath();
//                    wxString ext=f->file.GetExt();
//                    if(IsPythonFile(name))
//                    {
//                        m_RunTarget=name;
//                        m_RunTargetSelected=true;
//                        menu->AppendSeparator();
////                        menu->Append(ID_LangMenu_Run,_T("Python Run"),_T(""));
//                        menu->Append(ID_LangMenu_RunPiped,_T("Python Debug"),_T(""));
//                    }
//                }
//            }
//	    }
//	}
//	if(type==mtEditorManager) // also type==mtOpenFilesList - not sure how to find out which file has been right clicked.
//	{
//        EditorManager* edMan = Manager::Get()->GetEditorManager();
//        wxFileName activefile(edMan->GetActiveEditor()->GetFilename());
//        wxString name=activefile.GetFullPath();
//        if(IsPythonFile(name))
//        {
//            m_RunTarget=name;
//            m_RunTargetSelected=true;
//            menu->AppendSeparator();
////            menu->Append(ID_LangMenu_Run,_T("Python Run"),_T(""));
//            menu->Append(ID_LangMenu_RunPiped,_T("Python Debug"),_T(""));
//
//        }
//	}
//	if(type==mtUnknown) // also type==mtOpenFilesList - not sure how to find out which file has been right clicked.
//	{
//        if(data->GetKind()==FileTreeData::ftdkFile)  //right clicked on folder in file explorer
//        {
//            wxFileName f(data->GetFolder());
//            wxString filename=f.GetFullPath();
//            wxString name=f.GetFullName();
////            cbMessageBox(filename+_T("  ")+name);
//            if(IsPythonFile(name))
//            {
//                m_RunTarget=filename;
//                m_RunTargetSelected=true;
//                menu->AppendSeparator();
////            menu->Append(ID_LangMenu_Run,_T("Python Run"),_T(""));
//                menu->Append(ID_LangMenu_RunPiped,_T("Python Debug"),_T(""));
//
//            }
//        }
//	}
}

bool PyPlugin::BuildToolBar(wxToolBar* toolBar)
{
//    m_pTbar = toolBar;
//    /* Loads toolbar using new Manager class functions */
//    if (!IsAttached() || !toolBar)
//        return false;
//    wxString my_16x16=Manager::isToolBar16x16(toolBar) ? _T("_16x16") : _T("");
//    Manager::AddonToolBar(toolBar,wxString(_T("py_debugger_toolbar"))+my_16x16);
//    toolBar->Realize();
    return true;
}
