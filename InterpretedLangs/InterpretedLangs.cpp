#include <configurationpanel.h>
#include "InterpretedLangs.h"

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<InterpretedLangs> reg(_T("InterpretedLangs"));
}


int ID_LangMenu_Settings=wxNewId();
int ID_LangMenu_RunPiped=wxNewId();
int ID_LangMenu_ShowConsole=wxNewId();
int ID_PipedProcess=wxNewId();
int ID_PollPipedProcess=wxNewId();
//int ID_LangMenu_SetTarget=wxNewId();
//int ID_LangMenu_RunTarget=wxNewId();
//int ID_LangMenu_Run=wxNewId();
//int ID_LangMenu_SetInterp=wxNewId();
//int ID_LangMenu_RunTargetFileTree=wxNewId();



// Ugly ugly hack to handle dynamic menus
int ID_ContextMenu_0=wxNewId();
int ID_ContextMenu_1=wxNewId();
int ID_ContextMenu_2=wxNewId();
int ID_ContextMenu_3=wxNewId();
int ID_ContextMenu_4=wxNewId();
int ID_ContextMenu_5=wxNewId();
int ID_ContextMenu_6=wxNewId();
int ID_ContextMenu_7=wxNewId();
int ID_ContextMenu_8=wxNewId();
int ID_ContextMenu_9=wxNewId();
int ID_Menu_0=wxNewId();
int ID_Menu_1=wxNewId();
int ID_Menu_2=wxNewId();
int ID_Menu_3=wxNewId();
int ID_Menu_4=wxNewId();
int ID_Menu_5=wxNewId();
int ID_Menu_6=wxNewId();
int ID_Menu_7=wxNewId();
int ID_Menu_8=wxNewId();
int ID_Menu_9=wxNewId();
int ID_NoTargMenu_0=wxNewId();
int ID_NoTargMenu_1=wxNewId();
int ID_NoTargMenu_2=wxNewId();
int ID_NoTargMenu_3=wxNewId();
int ID_NoTargMenu_4=wxNewId();
int ID_NoTargMenu_5=wxNewId();
int ID_NoTargMenu_6=wxNewId();
int ID_NoTargMenu_7=wxNewId();
int ID_NoTargMenu_8=wxNewId();
int ID_NoTargMenu_9=wxNewId();
int ID_SubMenu_0=wxNewId();
int ID_SubMenu_1=wxNewId();
int ID_SubMenu_2=wxNewId();
int ID_SubMenu_3=wxNewId();
int ID_SubMenu_4=wxNewId();
int ID_SubMenu_5=wxNewId();
int ID_SubMenu_6=wxNewId();
int ID_SubMenu_7=wxNewId();
int ID_SubMenu_8=wxNewId();
int ID_SubMenu_9=wxNewId();
int ID_SubMenu_10=wxNewId();
int ID_SubMenu_11=wxNewId();
int ID_SubMenu_12=wxNewId();
int ID_SubMenu_13=wxNewId();
int ID_SubMenu_14=wxNewId();
int ID_SubMenu_15=wxNewId();
int ID_SubMenu_16=wxNewId();
int ID_SubMenu_17=wxNewId();
int ID_SubMenu_18=wxNewId();
int ID_SubMenu_19=wxNewId();
int ID_SubMenu_20=wxNewId();



// events handling
BEGIN_EVENT_TABLE(InterpretedLangs, cbPlugin)
	// add any events you want to handle here
//    EVT_MENU(ID_LangMenu_Settings,InterpretedLangs::OnSettings)
//    EVT_MENU(ID_LangMenu_SetTarget,InterpretedLangs::OnSetTarget)
//    EVT_MENU(ID_LangMenu_SetInterp,InterpretedLangs::OnSetTarget)
//    EVT_MENU(ID_LangMenu_Run,InterpretedLangs::OnRun)
//    EVT_MENU(ID_LangMenu_RunTarget,InterpretedLangs::OnRunTarget)
//    EVT_MENU(ID_LangMenu_RunTargetFileTree,InterpretedLangs::OnRunTarget)

//    EVT_MENU(ID_LangMenu_RunPiped,InterpretedLangs::OnRunPiped)
    EVT_MENU_RANGE(ID_ContextMenu_0,ID_ContextMenu_9,InterpretedLangs::OnRunTarget)
    EVT_MENU_RANGE(ID_NoTargMenu_0, ID_NoTargMenu_9, InterpretedLangs::OnRun)
    EVT_MENU_RANGE(ID_SubMenu_0, ID_SubMenu_20, InterpretedLangs::OnRunTarget)
    EVT_MENU(ID_LangMenu_ShowConsole,InterpretedLangs::OnShowConsole)

END_EVENT_TABLE()


void InterpretedLangs::OnShowConsole(wxCommandEvent& event)
{
    // This toggles display of the console I/O window
    CodeBlocksDockEvent evt(event.IsChecked() ? cbEVT_SHOW_DOCK_WINDOW : cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_commandio;
    Manager::Get()->GetAppWindow()->ProcessEvent(evt);
}

void InterpretedLangs::ShowConsole()
{
    // This toggles display of the console I/O window
    CodeBlocksDockEvent evt(cbEVT_SHOW_DOCK_WINDOW);
    evt.pWindow = m_commandio;
    Manager::Get()->GetAppWindow()->ProcessEvent(evt);
}

void InterpretedLangs::HideConsole()
{
    // This toggles display of the console I/O window
    CodeBlocksDockEvent evt(cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_commandio;
    Manager::Get()->GetAppWindow()->ProcessEvent(evt);
}


void InterpretedLangs::OnSettings(wxCommandEvent& event)
{
    wxMessageBox(_T("Settings..."));
}

void InterpretedLangs::OnSubMenuSelect(wxUpdateUIEvent& event)
{
    int num=event.GetId()-ID_Menu_0;
    if(num>=0 && num<=9)
    {
        m_interpnum=num;
        if(num==1)
        {
        wxString a;
        a<<_T("Sub menu")<<m_interpnum<<_T(" opened");
        wxMessageBox(a);
        }
    }
}

void InterpretedLangs::OnSetTarget(wxCommandEvent& event)
{
    wxFileDialog *fd=new wxFileDialog(NULL,_T("Choose the interpreter Target"),_T(""),_T(""),m_wildcard,wxOPEN|wxFILE_MUST_EXIST);
    if(fd->ShowModal()==wxID_OK)
    {
        m_RunTarget=fd->GetPath();
    } else
        m_RunTarget=_T("");
    delete fd;
}

void InterpretedLangs::OnRunTarget(wxCommandEvent& event)
{
    int ID=event.GetId();
    wxString commandstr;
    wxString consolename;
    bool windowed=false;
    if(ID>=ID_ContextMenu_0&&ID<=ID_ContextMenu_9)
    {
        if(!wxFileName::FileExists(m_RunTarget))
        {
            wxMessageBox(_T("Target ")+m_RunTarget+_T(" does not exist, please select another or Cancel"));
            OnSetTarget(event);
            if(m_RunTarget==_T(""))
                return;
        }
        int actionnum=ID-ID_ContextMenu_0;
        commandstr=m_ic.interps[m_interpnum].actions[actionnum].command;
        consolename=m_ic.interps[m_interpnum].name+_T(" ")+m_ic.interps[m_interpnum].actions[actionnum].name;
        windowed=(m_ic.interps[m_interpnum].actions[actionnum].windowed==_("W"));
        commandstr.Replace(_T("$interpreter"),wxFileName(m_ic.interps[m_interpnum].exec).GetShortPath(),false);
        commandstr.Replace(_T("$file"),wxFileName(m_RunTarget).GetShortPath(),false);
    } else
    if(ID>=ID_SubMenu_0&&ID<=ID_SubMenu_20)
    {
        wxMenu *m=LangMenu->FindItem(ID)->GetMenu(); // get pointer object to selected item in submenu
        if(m==NULL)
        {
            wxMessageBox(_T("WARNING: Sub menu not found - cancelling command"));
            return;
        }
        // need to figure out which interpeter we're using by matching the pointer to the parent of the selected item with pointers to our menus
        for(m_interpnum=0;m_interpnum<m_ic.interps.size()&&m_interpnum<10;m_interpnum++)
        {
            if(LangMenu->FindItem(ID_Menu_0+m_interpnum)->GetSubMenu()==m) //compare pointer to submenu with known submenus and break out of loop if matched
                break;
        }
        if(m_interpnum>=m_ic.interps.size()||m_interpnum>=10)
        {
            wxMessageBox(_T("WARNING: Sub menu not found - cancelling command"));
            return;
        }
        int actionnum=ID-m->FindItemByPosition(0)->GetId();
        commandstr=m_ic.interps[m_interpnum].actions[actionnum].command;
        consolename=m_ic.interps[m_interpnum].name+_T(" ")+m_ic.interps[m_interpnum].actions[actionnum].name;
        windowed=(m_ic.interps[m_interpnum].actions[actionnum].windowed==_("W"));
        m_wildcard=m_ic.interps[m_interpnum].extensions;
        OnSetTarget(event);
        wxFileName fn(m_RunTarget);
        if(!fn.FileExists(m_RunTarget))
            return;

        commandstr.Replace(_T("$interpreter"),wxFileName(m_ic.interps[m_interpnum].exec).GetShortPath(),false);
        commandstr.Replace(_T("$file"),wxFileName(m_RunTarget).GetShortPath(),false);
    } else
    {
        wxMessageBox(_T("WARNING: Unprocessed Interpreter Message"));
        return;
    }
    if(windowed)
    {
        m_commandio->LaunchProcess(commandstr,consolename,0);
        ShowConsole();
    } else
    {
        wxExecute(commandstr,wxEXEC_ASYNC);
    }
}

void InterpretedLangs::OnRun(wxCommandEvent& event)
{
    int ID=event.GetId();
    wxString commandstr;
    wxString consolename;
    wxMenu *m=LangMenu->FindItem(ID)->GetMenu(); // get pointer object to selected item in submenu
    for(m_interpnum=0;m_interpnum<m_ic.interps.size()&&m_interpnum<10;m_interpnum++)
    {
        if(LangMenu->FindItem(ID_Menu_0+m_interpnum)->GetSubMenu()==m) //compare pointer to submenu with known submenus and break out if loop if matched
            break;
    }
    if(m_interpnum>=m_ic.interps.size()||m_interpnum>=10)
    {
        wxMessageBox(_T("Warning: Sub menu not found - cancelling command"));
        return;
    }
    commandstr=m_ic.interps[m_interpnum].exec;
    consolename=m_ic.interps[m_interpnum].name;

    wxExecute(commandstr,wxEXEC_ASYNC);
//    m_commandio->LaunchProcess(commandstr,consolename,0);
}




// constructor
InterpretedLangs::InterpretedLangs()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("InterpretedLangs.zip")))
    {
        NotifyMissingFile(_T("InterpretedLangs.zip"));
    }
}

cbConfigurationPanel* InterpretedLangs::GetConfigurationPanel(wxWindow* parent)
{
//    MyDialog* dlg = new MyDialog(this, *m_pKeyProfArr, parent,
//        wxT("Keybindings"), mode);

    return new ConfigDialog(parent, this);
}

// destructor
InterpretedLangs::~InterpretedLangs()
{

}

void InterpretedLangs::OnAttach()
{
	// do whatever initialization you need for your plugin
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be TRUE...
	// You should check for it in other functions, because if it
	// is FALSE, it means that the application did *not* "load"
	// (see: does not need) this plugin...

	m_ic.ReadConfig();

	m_pipeoutput=true;

    m_commandio = new ShellManager(Manager::Get()->GetAppWindow());

    CodeBlocksDockEvent evt(cbEVT_ADD_DOCK_WINDOW);
    evt.name = _T("Shells");
    evt.title = _T("Shells");
    evt.pWindow = m_commandio;
    evt.dockSide = CodeBlocksDockEvent::dsFloating;
    evt.desiredSize.Set(400, 300);
    evt.floatingSize.Set(400, 300);
    evt.minimumSize.Set(200, 150);
    Manager::Get()->GetAppWindow()->ProcessEvent(evt);
}

void InterpretedLangs::OnRelease(bool appShutDown)
{
	// do de-initialization for your plugin
	// if appShutDown is false, the plugin is unloaded because Code::Blocks is being shut down,
	// which means you must not use any of the SDK Managers
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be FALSE...

    if (m_commandio)
    {
        CodeBlocksDockEvent evt(cbEVT_REMOVE_DOCK_WINDOW);
        evt.pWindow = m_commandio;
        Manager::Get()->GetAppWindow()->ProcessEvent(evt);
        m_commandio->Destroy();
    }
    m_commandio = 0;
}

int InterpretedLangs::Configure()
{
	//create and display the configuration dialog for your plugin
	cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("Your dialog title"));
	cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
	if (panel)
	{
		dlg.AttachConfigurationPanel(panel);
		PlaceWindow(&dlg);
		return dlg.ShowModal() == wxID_OK ? 0 : -1;
	}
	return -1;
}

void InterpretedLangs::CreateMenu()
{
    unsigned int j=0; //index to actions
    for(unsigned int i=0;i<m_ic.interps.size()&&i<10;i++) //create at most 10 interpreter submenus
    {
        wxMenu *submenu=new wxMenu();
        unsigned int maxj=j+m_ic.interps[i].actions.size();
        if(maxj>20)
            maxj=20;
        unsigned int jstart=j;
        for(;j<maxj;j++)
            submenu->Append(ID_SubMenu_0+j,m_ic.interps[i].actions[j-jstart].name+_T("..."),_T(""));
        submenu->Append(ID_NoTargMenu_0+i,_T("Run Without Target"),_T(""));
        LangMenu->Append(ID_Menu_0+i,m_ic.interps[i].name,submenu);
    }
    LangMenu->Append(ID_LangMenu_ShowConsole,_T("Toggle I/O console"),_T(""),wxITEM_CHECK);
}


void InterpretedLangs::UpdateMenu()
{
    //delete the old menu items
    if(LangMenu)
    {
        for(unsigned int i=0;i<m_ic.interps.size();i++)
            LangMenu->Destroy(ID_Menu_0+i);
        LangMenu->Destroy(ID_LangMenu_ShowConsole);
        CreateMenu();
    }
}


void InterpretedLangs::BuildMenu(wxMenuBar* menuBar)
{
	//The application is offering its menubar for your plugin,
	//to add any menu items you want...
	//Append any items you need in the menu...
	//NOTE: Be careful in here... The application's menubar is at your disposal.
	LangMenu=new wxMenu;
	CreateMenu();
	int pos = menuBar->FindMenu(_T("Plugins"));
	if(pos!=wxNOT_FOUND)
        menuBar->Insert(pos, LangMenu, _T("Interpreters"));
    else
    {
        delete LangMenu;
        LangMenu=0;
    }
//	NotImplemented(_T("InterpretedLangs::BuildMenu()"));
}

void InterpretedLangs::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
	//Some library module is ready to display a pop-up menu.
	//Check the parameter \"type\" and see which module it is
	//and append any items you need in the menu...
	//TIP: for consistency, add a separator as the first item...
	if(type==mtProjectManager)
	{
	    if(data)
	    {
            if(data->GetKind()==FileTreeData::ftdkFile)
            {
                ProjectFile *f=data->GetProjectFile();
                if(f)
                {
                    wxString name=f->file.GetFullPath();
                    wxString ext=f->file.GetExt();
//                    wxString debugstr = _T("filename: ")+name+_T("\nextension: ")+ext;
//                    wxMessageBox(debugstr);
                    for(unsigned int i=0;i<m_ic.interps.size();i++)
                        if(m_ic.interps[i].extensions.Find(ext)>=0)
                        {
                            m_RunTarget=name;
                            m_interpnum=i;
                            if(m_ic.interps[i].actions.size()>0)
                                menu->AppendSeparator();
                            for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                            {
                                wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                                menu->Append(ID_ContextMenu_0+j,menutext,_T(""));
                            }
                            return;
                        }
                }
            }
	    }
	}
	if(type==mtEditorManager) // also type==mtOpenFilesList - not sure how to find out which file has been right clicked.
	{
        EditorManager* edMan = Manager::Get()->GetEditorManager();
        wxFileName activefile(edMan->GetActiveEditor()->GetFilename());
        wxString name=activefile.GetFullPath();
        wxString ext=activefile.GetExt();
        for(unsigned int i=0;i<m_ic.interps.size();i++)
            if(m_ic.interps[i].extensions.Find(ext)>=0)
            {
                m_RunTarget=name;
                m_interpnum=i;
                if(m_ic.interps[i].actions.size()>0)
                    menu->AppendSeparator();
                for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                {
                    wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                    menu->Append(ID_ContextMenu_0+j,menutext,_T(""));
                }
                return;
            }
	}
//	NotImplemented(_T("InterpretedLangs::BuildModuleMenu()"));
}

