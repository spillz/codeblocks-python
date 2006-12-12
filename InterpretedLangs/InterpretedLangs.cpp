#include <sdk.h> // Code::Blocks SDK
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
int ID_PipedProcess=wxNewId();
int ID_TimerPollDebugger=wxNewId();
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
//    EVT_END_PROCESS(wxID_ANY, InterpretedLangs::OnTerminatePipedProcess)
//    EVT_PIPEDPROCESS_STDOUT(ID_PipedProcess, InterpretedLangs::OnPipedOutput)
//    EVT_IDLE(InterpretedLangs::OnIdle)
//    EVT_TIMER(ID_TimerPollDebugger, InterpretedLangs::OnTimer)
END_EVENT_TABLE()

void InterpretedLangs::OnPipedOutput(wxCommandEvent& event)
{
    wxString msg = event.GetString();
    if (!msg.IsEmpty())
    {
//        Manager::Get()->GetMessageManager()->Log(m_PageIndex, _T("O>>> %s"), msg.c_str());
        wxMessageBox(msg);
    }
}

//void InterpretedLangs::OnIdle(wxIdleEvent& event)
//{
//    if (m_pp && ((PipedProcess*)m_pp)->HasInput())
//        event.RequestMore();
//    else
//        event.Skip();
//}

void InterpretedLangs::OnTimer(wxTimerEvent& event)
{
    wxWakeUpIdle();
}


//void InterpretedLangs::OnRunPiped(wxCommandEvent &event)
//{
//    m_TimerPollDebugger.SetOwner(this, ID_TimerPollDebugger);
//    m_pp=new PipedProcess((void **)&m_pp,this,ID_PipedProcess);
//    m_pp->Launch(_T("C:/Python24/python.exe"),25);
//    m_istream=m_pp->GetInputStream();
//    m_ostream=m_pp->GetOutputStream();
//}

//void InterpretedLangs::OnTerminatePipedProcess(wxProcessEvent &event)
//{
//    delete m_pp;
//}

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
    wxExecute(commandstr,wxEXEC_ASYNC,NULL);
}

void InterpretedLangs::OnRun(wxCommandEvent& event)
{
    int ID=event.GetId();
    wxString commandstr;
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
    commandstr=wxFileName(m_ic.interps[m_interpnum].exec).GetShortPath();
    wxExecute(commandstr,wxEXEC_ASYNC,NULL);
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

}

void InterpretedLangs::OnRelease(bool appShutDown)
{
	// do de-initialization for your plugin
	// if appShutDown is false, the plugin is unloaded because Code::Blocks is being shut down,
	// which means you must not use any of the SDK Managers
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be FALSE...
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
}


void InterpretedLangs::UpdateMenu()
{
    //delete the old menu items
    if(LangMenu)
    {
        for(unsigned int i=0;i<m_ic.interps.size();i++)
            LangMenu->Destroy(ID_Menu_0+i);
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


/*
// ----------------------------------------------------------------------------
// Piped Process for the Interpreter
// ----------------------------------------------------------------------------

// Not sure whether to do message handling here or in the InterpretedLangs class...

class InterpreterProcess: public wxProcess
{
public:
    InterpreterProcess(MyFrame *parent, const wxString& cmd, , const wxString& input)
        : wxProcess(parent), m_cmd(cmd)
    {
        m_parent = parent;
        Redirect();
    }
    virtual bool HasInput();
    virtual void OnTerminate(int pid, int status);
private:
    wxString m_input;
    InterpretedLangs *m_parent;
    wxString m_cmd;

}

bool InterpreterProcess::HasInput()
{
    if ( !m_input.empty() )
    {
        wxTextOutputStream os(*GetOutputStream());
        os.WriteString(m_input);

        CloseOutput();
        m_input.clear();

        // call us once again - may be we'll have output
        return true;
    }

    bool hasInput = false;

    if ( IsInputAvailable() )
    {
        wxTextInputStream tis(*GetInputStream());

        // this assumes that the output is always line buffered
        wxString msg;
        msg << m_cmd << _T(" (stdout): ") << tis.ReadLine();

        m_parent->GetLogListBox()->Append(msg);

        hasInput = true;
    }

    if ( IsErrorAvailable() )
    {
        wxTextInputStream tis(*GetErrorStream());

        // this assumes that the output is always line buffered
        wxString msg;
        msg << m_cmd << _T(" (stderr): ") << tis.ReadLine();

        m_parent->GetLogListBox()->Append(msg);

        hasInput = true;
    }

    return hasInput;
}

void InterpreterProcess::OnTerminate(int pid, int status)
{
    if ( !m_input.empty() )
    {
        wxTextOutputStream os(*GetOutputStream());
        os.WriteString(m_input);

        CloseOutput();
        m_input.clear();

        // call us once again - may be we'll have output
        return true;
    }

    // show the rest of the output
    while ( HasInput() )
        ;

    m_parent->OnProcessTerminated(this);

    wxLogStatus(m_parent, _T("Process %u ('%s') terminated with exit code %d."),
                pid, m_cmd.c_str(), status);

    // we're not needed any more
    delete this;
}

// ----------------------------------------------------------------------------
// End of Piped Process for the Interpreter
// ----------------------------------------------------------------------------


*/
