#include "ShellExtensions.h"
#include "se_globals.h"
#ifdef CBIL_TEARAWAY
#include <tearawaynotebook.h>
#endif

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<ShellExtensions> reg(_T("ShellExtensions"));
}

int ID_UpdateUI=wxNewId();
int ID_LangMenu_Settings=wxNewId();
int ID_LangMenu_RunPiped=wxNewId();
int ID_LangMenu_ShowConsole=wxNewId();
int ID_PipedProcess=wxNewId();

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
int ID_ContextMenu_10=wxNewId();
int ID_ContextMenu_11=wxNewId();
int ID_ContextMenu_12=wxNewId();
int ID_ContextMenu_13=wxNewId();
int ID_ContextMenu_14=wxNewId();
int ID_ContextMenu_15=wxNewId();
int ID_ContextMenu_16=wxNewId();
int ID_ContextMenu_17=wxNewId();
int ID_ContextMenu_18=wxNewId();
int ID_ContextMenu_19=wxNewId();
int ID_ContextMenu_20=wxNewId();
int ID_ContextMenu_21=wxNewId();
int ID_ContextMenu_22=wxNewId();
int ID_ContextMenu_23=wxNewId();
int ID_ContextMenu_24=wxNewId();
int ID_ContextMenu_25=wxNewId();
int ID_ContextMenu_26=wxNewId();
int ID_ContextMenu_27=wxNewId();
int ID_ContextMenu_28=wxNewId();
int ID_ContextMenu_29=wxNewId();
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
BEGIN_EVENT_TABLE(ShellExtensions, cbPlugin)
    EVT_MENU_RANGE(ID_ContextMenu_0,ID_ContextMenu_9,ShellExtensions::OnRunTarget)
    EVT_MENU_RANGE(ID_NoTargMenu_0, ID_NoTargMenu_9, ShellExtensions::OnRun)
    EVT_MENU_RANGE(ID_SubMenu_0, ID_SubMenu_20, ShellExtensions::OnRunTarget)
    EVT_MENU(ID_LangMenu_ShowConsole,ShellExtensions::OnShowConsole)
    EVT_UPDATE_UI(ID_LangMenu_ShowConsole, ShellExtensions::OnUpdateUI)
END_EVENT_TABLE()


void ShellExtensions::OnUpdateUI(wxUpdateUIEvent& event)
{
    m_LangMenu->Check(ID_LangMenu_ShowConsole,IsWindowReallyShown(m_shellmgr));
    // allow other UpdateUI handlers to process this event
    // *very* important! don't forget it...
    event.Skip();
}


void ShellExtensions::OnShowConsole(wxCommandEvent& event)
{
    // This toggles display of the console I/O window
    CodeBlocksDockEvent evt(event.IsChecked() ? cbEVT_SHOW_DOCK_WINDOW : cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_shellmgr;
    Manager::Get()->ProcessEvent(evt);
}

void ShellExtensions::ShowConsole()
{
    // This shows the console I/O window
    CodeBlocksDockEvent evt(cbEVT_SHOW_DOCK_WINDOW);
    evt.pWindow = m_shellmgr;
    Manager::Get()->ProcessEvent(evt);
}

void ShellExtensions::HideConsole()
{
    // This hides display of the console I/O window
    CodeBlocksDockEvent evt(cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_shellmgr;
    Manager::Get()->ProcessEvent(evt);
}

void ShellExtensions::OnSettings(wxCommandEvent& event)
{
    cbMessageBox(_T("Settings..."));
}

void ShellExtensions::OnSubMenuSelect(wxUpdateUIEvent& event)
{
    int num=event.GetId()-ID_Menu_0;
    if(num>=0 && num<=9)
    {
        m_interpnum=num;
        if(num==1)
        {
        wxString a;
        a<<_T("Sub menu")<<m_interpnum<<_T(" opened");
        cbMessageBox(a);
        }
    }
}

void ShellExtensions::OnSetTarget(wxCommandEvent& event)
{
    wxString wild(m_wildcard);
    if(wild==_T(""))
#ifdef __WXMSW__
        wild=_T("*.*");
#else
        wild=_T("*");
#endif
    wxFileDialog *fd=new wxFileDialog(NULL,_T("Choose the ShellCommand Target"),_T(""),_T(""),wild,wxOPEN|wxFILE_MUST_EXIST);
    if(fd->ShowModal()==wxID_OK)
    {
        m_RunTarget=fd->GetPath();
    } else
        m_RunTarget=_T("");
    delete fd;
}

void ShellExtensions::OnSetMultiTarget(wxCommandEvent& event)
{
    wxString wild(m_wildcard);
    if(wild==_T(""))
#ifdef __WXMSW__
        wild=_T("*.*");
#else
        wild=_T("*");
#endif
    wxFileDialog *fd=new wxFileDialog(NULL,_T("Choose the ShellCommand Targets"),_T(""),_T(""),wild,wxOPEN|wxFILE_MUST_EXIST|wxMULTIPLE);
    if(fd->ShowModal()==wxID_OK)
    {
        wxArrayString paths;
        fd->GetPaths(paths);
        m_RunTarget=paths[0];
        for(size_t i=1;i<paths.GetCount();i++)
            m_RunTarget+=_T(" ")+paths[i];
    } else
        m_RunTarget=_T("");
    delete fd;
}


void ShellExtensions::OnSetDirTarget(wxCommandEvent& event)
{
    wxDirDialog *dd=new wxDirDialog(NULL,_T("Choose the Target Directory"),_T(""));
    if(dd->ShowModal()==wxID_OK)
    {
        m_RunTarget=dd->GetPath();
    } else
        m_RunTarget=_T("");
    delete dd;
}

void ShellExtensions::OnRunTarget(wxCommandEvent& event)
{
    int ID=event.GetId();
    wxString commandstr;
    wxString consolename;
    wxString workingdir;
    bool windowed=false;
    bool console=false;
    if(ID>=ID_ContextMenu_0&&ID<=ID_ContextMenu_29)
    {
        int actionnum=m_contextvec[ID-ID_ContextMenu_0].a;
        m_interpnum=m_contextvec[ID-ID_ContextMenu_0].i;
        commandstr=m_ic.interps[m_interpnum].actions[actionnum].command;
        consolename=m_ic.interps[m_interpnum].name+_T(" ")+m_ic.interps[m_interpnum].actions[actionnum].name;
        windowed=(m_ic.interps[m_interpnum].actions[actionnum].mode==_("W"));
        console=(m_ic.interps[m_interpnum].actions[actionnum].mode==_("C"));
        workingdir=m_ic.interps[m_interpnum].actions[actionnum].wdir;
    } else
    if(ID>=ID_SubMenu_0&&ID<=ID_SubMenu_20)
    {
        wxMenu *m=m_LangMenu->FindItem(ID)->GetMenu(); // get pointer object to selected item in submenu
        if(m==NULL)
        {
            cbMessageBox(_T("WARNING: Sub menu not found - cancelling command"));
            return;
        }
        // need to figure out which interpeter we're using by matching the pointer to the parent of the selected item with pointers to our menus
        for(m_interpnum=0;m_interpnum<m_ic.interps.size()&&m_interpnum<10;m_interpnum++)
        {
            if(m_LangMenu->FindItem(ID_Menu_0+m_interpnum)->GetSubMenu()==m) //compare pointer to submenu with known submenus and break out of loop if matched
                break;
        }
        if(m_interpnum>=m_ic.interps.size()||m_interpnum>=10)
        {
            cbMessageBox(_T("WARNING: Sub menu not found - cancelling command"));
            return;
        }
        int actionnum=ID-m->FindItemByPosition(0)->GetId();
        commandstr=m_ic.interps[m_interpnum].actions[actionnum].command;
        consolename=m_ic.interps[m_interpnum].name+_T(" ")+m_ic.interps[m_interpnum].actions[actionnum].name;
        windowed=(m_ic.interps[m_interpnum].actions[actionnum].mode==_("W"));
        console=(m_ic.interps[m_interpnum].actions[actionnum].mode==_("C"));
        workingdir=m_ic.interps[m_interpnum].actions[actionnum].wdir;
        m_wildcard=m_ic.interps[m_interpnum].extensions;
        if(m_ic.interps[m_interpnum].actions[actionnum].command.Find(_T("$file"))>0 ||
            m_ic.interps[m_interpnum].actions[actionnum].command.Find(_T("$path"))>0)
        {
            OnSetTarget(event);
            if(!wxFileName::FileExists(m_RunTarget))
            {
                LogMessage(_("ShellExtensions: ")+m_RunTarget+_(" not found"));
                return;
            }
        }
        if(m_ic.interps[m_interpnum].actions[actionnum].command.Find(_T("$dir"))>0)
        {
            OnSetDirTarget(event);
            if(!wxFileName::DirExists(m_RunTarget))
            {
                LogMessage(_("ShellExtensions: ")+m_RunTarget+_(" not found"));
                return;
            }
            if(m_RunTarget==_T(""))
                return;
        }
        if(m_ic.interps[m_interpnum].actions[actionnum].command.Find(_T("$mpaths"))>0)
        {
            OnSetMultiTarget(event);
            if(m_RunTarget==_T(""))
                return;
        }
    }
    else
    {
        LogMessage(wxString::Format(_T("WARNING: Unprocessed ShellCommand Menu Message: ID %i, IDbase %i, IDend %i, num items on menu %i"),ID,ID_ContextMenu_0,ID_ContextMenu_29,(int)m_contextvec.size()));
        return;
    }

    m_RunTarget.Replace(_T("*"),_T(" "));

    bool setdir=true;
    commandstr.Replace(_T("$file"),wxFileName(m_RunTarget).GetShortPath(),false);
    commandstr.Replace(_T("$dir"),wxFileName(m_RunTarget).GetShortPath(),false);
    commandstr.Replace(_T("$path"),wxFileName(m_RunTarget).GetShortPath(),false);
    if(commandstr.Replace(_T("$mpaths"),m_RunTarget,false)>0)
        setdir=false;
    commandstr.Replace(_T("$interpreter"),wxFileName(m_ic.interps[m_interpnum].exec).GetShortPath(),false);
    workingdir.Replace(_T("$parentdir"),wxFileName(m_RunTarget).GetPath(),false);
    workingdir.Replace(_T("$dir"),wxFileName(m_RunTarget).GetPath(),false);


    if(Manager::Get()->GetMacrosManager())
    {
        Manager::Get()->GetMacrosManager()->RecalcVars(0, 0, 0); // hack to force-update macros
        Manager::Get()->GetMacrosManager()->ReplaceMacros(commandstr);
        Manager::Get()->GetMacrosManager()->ReplaceMacros(workingdir);
    }
    wxString olddir=wxGetCwd();
    if(setdir && workingdir!=_T(""))
    {
        if(!wxSetWorkingDirectory(workingdir))
        {
            LogMessage(_T("IntepretedLangs: Can't change to working directory to ")+workingdir);
            return;
        }
    }

    LogMessage(wxString::Format(_("Launching '%s': %s (in %s)"), consolename.c_str(), commandstr.c_str(), workingdir.c_str()));

    if(windowed)
    {
        m_shellmgr->LaunchProcess(commandstr,consolename);
        ShowConsole();
    } else if (console)
    {
        wxString cmdline;
#ifndef __WXMSW__
        // for non-win platforms, use m_ConsoleTerm to run the console app
        wxString term = Manager::Get()->GetConfigManager(_T("app"))->Read(_T("/console_terminal"), DEFAULT_CONSOLE_TERM);
        term.Replace(_T("$TITLE"), _T("'") + consolename + _T("'"));
        cmdline<< term << _T(" ");
        #define CONSOLE_RUNNER "cb_console_runner"
#else
        #define CONSOLE_RUNNER "cb_console_runner.exe"
#endif
        wxString baseDir = ConfigManager::GetExecutableFolder();
        if (wxFileExists(baseDir + wxT("/" CONSOLE_RUNNER)))
            cmdline << baseDir << wxT("/" CONSOLE_RUNNER " ");
        cmdline<<commandstr;

        if(!wxExecute(cmdline))
            cbMessageBox(_T("Command Launch Failed: ")+commandstr);
    } else
    {
        if(!wxExecute(commandstr))
            cbMessageBox(_T("Command Launch Failed: ")+commandstr);
    }
    wxSetWorkingDirectory(olddir);
}

// DEPRECATED - NO LONGER REQUIRED
void ShellExtensions::OnRun(wxCommandEvent& event)
{
    int ID=event.GetId();
    wxString commandstr;
    wxString consolename;
    wxMenu *m=m_LangMenu->FindItem(ID)->GetMenu(); // get pointer object to selected item in submenu
    for(m_interpnum=0;m_interpnum<m_ic.interps.size()&&m_interpnum<10;m_interpnum++)
    {
        if(m_LangMenu->FindItem(ID_Menu_0+m_interpnum)->GetSubMenu()==m) //compare pointer to submenu with known submenus and break out if loop if matched
            break;
    }
    if(m_interpnum>=m_ic.interps.size()||m_interpnum>=10)
    {
        cbMessageBox(_T("Warning: Sub menu not found - cancelling command"));
        return;
    }
    commandstr=m_ic.interps[m_interpnum].exec;
    consolename=m_ic.interps[m_interpnum].name;

    wxExecute(commandstr,wxEXEC_ASYNC);
//    m_shellmgr->LaunchProcess(commandstr,consolename,0);
}

// constructor
ShellExtensions::ShellExtensions()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("ShellExtensions.zip")))
    {
        NotifyMissingFile(_T("ShellExtensions.zip"));
    }
}

cbConfigurationPanel* ShellExtensions::GetConfigurationPanel(wxWindow* parent)
{
//    MyDialog* dlg = new MyDialog(this, *m_pKeyProfArr, parent,
//        wxT("Keybindings"), mode);

    return new ConfigDialog(parent, this);
}

// destructor
ShellExtensions::~ShellExtensions()
{

}

void ShellExtensions::OnAttach()
{
	// do whatever initialization you need for your plugin
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be TRUE...
	// You should check for it in other functions, because if it
	// is FALSE, it means that the application did *not* "load"
	// (see: does not need) this plugin...

	m_ic.ReadConfig();

	m_pipeoutput=true;

    m_shellmgr = new ShellManager(Manager::Get()->GetAppWindow());

    CodeBlocksDockEvent evt(cbEVT_ADD_DOCK_WINDOW);
    evt.name = _T("Shells");
    evt.title = _T("Shells");
    evt.pWindow = m_shellmgr;
    evt.dockSide = CodeBlocksDockEvent::dsFloating;
    evt.desiredSize.Set(400, 300);
    evt.floatingSize.Set(400, 300);
    evt.minimumSize.Set(200, 150);
    Manager::Get()->ProcessEvent(evt);

    m_fe=new FileExplorer(Manager::Get()->GetAppWindow());
    Manager::Get()->GetProjectManager()->GetNotebook()->AddPage(m_fe,_T("Files"));
}

void ShellExtensions::OnRelease(bool appShutDown)
{
	// do de-initialization for your plugin
	// if appShutDown is false, the plugin is unloaded because Code::Blocks is being shut down,
	// which means you must not use any of the SDK Managers
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be FALSE...

    if (m_shellmgr) //remove the Shell Terminals Notebook from its dockable window and delete it
    {
        CodeBlocksDockEvent evt(cbEVT_REMOVE_DOCK_WINDOW);
        evt.pWindow = m_shellmgr;
        Manager::Get()->ProcessEvent(evt);
        m_shellmgr->Destroy();
    }
    m_shellmgr = 0;
    if (m_fe) //remove the File Explorer from the Managment pane and delete it.
    {
        int idx = Manager::Get()->GetProjectManager()->GetNotebook()->GetPageIndex(m_fe);
        if (idx != -1)
            Manager::Get()->GetProjectManager()->GetNotebook()->RemovePage(idx);
        m_fe->Destroy();
    }
    m_fe = 0;
}

int ShellExtensions::Configure()
{
	//create and display the configuration dialog for your plugin
	cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("ShellCommand Settings"));
	cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
	if (panel)
	{
		dlg.AttachConfigurationPanel(panel);
		PlaceWindow(&dlg);
		return dlg.ShowModal() == wxID_OK ? 0 : -1;
	}
	return -1;
}

void ShellExtensions::CreateMenu()
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
        {
            wxString tail;
            if(m_ic.interps[i].actions[j-jstart].command.Find(_T("$file"))>0||
                m_ic.interps[i].actions[j-jstart].command.Find(_T("$dir"))>0||
                m_ic.interps[i].actions[j-jstart].command.Find(_T("$path"))>0||
                m_ic.interps[i].actions[j-jstart].command.Find(_T("$mpaths"))>0)
                tail=_T("...");
            submenu->Append(ID_SubMenu_0+j,m_ic.interps[i].actions[j-jstart].name+tail,_T(""));
        }
        m_LangMenu->Append(ID_Menu_0+i,m_ic.interps[i].name,submenu);
    }
    m_LangMenu->Append(ID_LangMenu_ShowConsole,_T("Toggle I/O console"),_T(""),wxITEM_CHECK);
}

void ShellExtensions::UpdateMenu()
{
    //delete the old menu items
    if(m_LangMenu)
    {
        for(unsigned int i=0;i<m_ic.interps.size();i++)
            m_LangMenu->Destroy(ID_Menu_0+i);
        m_LangMenu->Destroy(ID_LangMenu_ShowConsole);
        CreateMenu();
    }
}

void ShellExtensions::BuildMenu(wxMenuBar* menuBar)
{
	//The application is offering its menubar for your plugin,
	//to add any menu items you want...
	//Append any items you need in the menu...
	//NOTE: Be careful in here... The application's menubar is at your disposal.
	m_LangMenu=new wxMenu;
	CreateMenu();
	int pos = menuBar->FindMenu(_T("Plugins"));
	if(pos!=wxNOT_FOUND)
        menuBar->Insert(pos, m_LangMenu, _T("E&xtensions"));
    else
    {
        delete m_LangMenu;
        m_LangMenu=0;
    }
//	NotImplemented(_T("ShellExtensions::BuildMenu()"));
}

void ShellExtensions::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
	//Some library module is ready to display a pop-up menu.
	//Check the parameter \"type\" and see which module it is
	//and append any items you need in the menu...
	//TIP: for consistency, add a separator as the first item...
	m_contextvec.clear();
	if(type==mtProjectManager)
	{
	    if(data)
	    {
            if(data->GetKind()==FileTreeData::ftdkFile)
            {
                ProjectFile *f=data->GetProjectFile();
                if(f)
                {
                    wxString filename=f->file.GetFullPath();
                    wxString name=f->file.GetFullName();
                    size_t sep_pos=menu->GetMenuItemCount();
                    size_t added=0;
                    for(unsigned int i=0;i<m_ic.interps.size();i++)
                        if(WildCardListMatch(m_ic.interps[i].extensions,name))
                        {
                            m_RunTarget=filename;
                            for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                            {
                                if(m_ic.interps[i].actions[j].command.Find(_T("$file"))>=0)
                                {
                                    wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                                    m_contextvec.push_back(ShellCommandMenuRef(i,j));
                                    menu->Append(ID_ContextMenu_0+added,menutext,_T(""));
                                    added++;
                                }
                            }
                        }
                    if(added>0)
                        menu->InsertSeparator(sep_pos);
                }
            }
	    }
	}
	if(type==mtEditorManager) // also type==mtOpenFilesList - not sure how to find out which file has been right clicked.
	{
        EditorManager* edMan = Manager::Get()->GetEditorManager();
        wxFileName activefile(edMan->GetActiveEditor()->GetFilename());
        wxString filename=activefile.GetFullPath();
        wxString name=activefile.GetFullName();
        size_t sep_pos=menu->GetMenuItemCount();
        size_t added=0;
        for(unsigned int i=0;i<m_ic.interps.size();i++)
            if(WildCardListMatch(m_ic.interps[i].extensions,name))
            {
                m_RunTarget=filename;
                for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                {
                    if(m_ic.interps[i].actions[j].command.Find(_T("$file"))>=0)
                    {
                        wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                        m_contextvec.push_back(ShellCommandMenuRef(i,j));
                        menu->Append(ID_ContextMenu_0+added,menutext,_T(""));
                        added++;
                    }
                }
            }
        if(added>0)
            menu->InsertSeparator(sep_pos);
	}
    if(type==mtUnknown) //Assuming file explorer -- fileexplorer fills the filetreedata with ftdkFile or ftdkFolder as "kind", the folder is the full path of the entry
	    if(data)
	    {
            size_t sep_pos=menu->GetMenuItemCount();
            size_t added=0;
            if(data->GetKind()==FileTreeData::ftdkFile)  //right clicked on folder in file explorer
            {
                wxFileName f(data->GetFolder());
                wxString filename=f.GetFullPath();
                wxString name=f.GetFullName();
                for(unsigned int i=0;i<m_ic.interps.size();i++)
                    if(WildCardListMatch(m_ic.interps[i].extensions,name))
                    {
                        m_RunTarget=filename;
                        for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                        {
                            if(m_ic.interps[i].actions[j].command.Find(_T("$file"))>=0 ||
                                m_ic.interps[i].actions[j].command.Find(_T("$path"))>=0 ||
                                m_ic.interps[i].actions[j].command.Find(_T("$mpaths"))>=0)
                            {
                                wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                                m_contextvec.push_back(ShellCommandMenuRef(i,j));
                                menu->Append(ID_ContextMenu_0+added,menutext,_T(""));
                                added++;
                            }
                        }
                    }
            }
            if(data->GetKind()==FileTreeData::ftdkFolder) //right clicked on folder in file explorer
            {
                wxFileName f(data->GetFolder());
                wxString filename=f.GetFullPath();
                wxString name=f.GetFullName();
                for(unsigned int i=0;i<m_ic.interps.size();i++)
                    if(WildCardListMatch(m_ic.interps[i].extensions,name))
                    {
                        m_RunTarget=filename;
                        for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                        {
                            if(m_ic.interps[i].actions[j].command.Find(_T("$dir"))>=0 ||
                                m_ic.interps[i].actions[j].command.Find(_T("$path"))>=0 ||
                                m_ic.interps[i].actions[j].command.Find(_T("$mpaths"))>=0)
                            {
                                wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                                m_contextvec.push_back(ShellCommandMenuRef(i,j));
                                menu->Append(ID_ContextMenu_0+added,menutext,_T(""));
                                added++;
                            }
                        }
                    }
            }
            if(data->GetKind()==FileTreeData::ftdkVirtualGroup) //right clicked on multiple selections in file explorer
            {
                wxString paths=data->GetFolder(); //get folder contains a space separated list of the files/directories selected
                for(unsigned int i=0;i<m_ic.interps.size();i++)
                {
                    bool match=true; // all selected items must have names that match the wildcard for this grouping
                    wxString pathlist=paths;
                    wxString ipath=paths.BeforeFirst('*'); // '*' separated list
                    if(m_ic.interps[i].extensions!=_T(""));
                        while(match && pathlist!=_T(""))
                        {
                            wxString name=wxFileName(ipath).GetFullName();
                            if(ipath!=_T("") && !WildCardListMatch(m_ic.interps[i].extensions,ipath))
                                match=false;
                            pathlist=pathlist.AfterFirst('*');
                            ipath=pathlist.BeforeFirst('*');
                        }
                    if(match)
                    {
                        m_RunTarget=paths;
                        //TODO: need a m_TargetParent to allow the FileExplorer to define the parent of a selection (usually the root of the fileexplorer view?)
                        for(unsigned int j=0;j<m_ic.interps[i].actions.size();j++)
                        {
                            if(m_ic.interps[i].actions[j].command.Find(_T("$mpaths"))>=0)
                            {
                                wxString menutext=m_ic.interps[i].name+_T(" ")+m_ic.interps[i].actions[j].name;
                                m_contextvec.push_back(ShellCommandMenuRef(i,j));
                                menu->Append(ID_ContextMenu_0+added,menutext,_T(""));
                                added++;
                            }
                        }
                    }
                }
            }
            if(added>0)
                menu->InsertSeparator(sep_pos);
	    }
//	NotImplemented(_T("ShellExtensions::BuildModuleMenu()"));
}

