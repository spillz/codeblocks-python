#include <sdk.h> // Code::Blocks SDK
#include <cbstyledtextctrl.h>
#include <configurationpanel.h>
#include "codechecker.h"


#define CHECK_MARKER        6
#define CHECK_STYLE         wxSCI_MARK_MINUS


inline void LogMessage(const wxString &msg)
{ Manager::Get()->GetLogManager()->Log(msg); }


// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<CodeChecker> reg(_T("CodeChecker"));
}


// events handling
BEGIN_EVENT_TABLE(CodeChecker, cbPlugin)
    // add any events you want to handle here
    EVT_TIMER(wxID_ANY,CodeChecker::OnQueueTimer)
    EVT_END_PROCESS(wxID_ANY,CodeChecker::OnProcessTerminate)
END_EVENT_TABLE()

// constructor
CodeChecker::CodeChecker()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("codechecker.zip")))
    {
        NotifyMissingFile(_T("codechecker.zip"));
    }
}

// destructor
CodeChecker::~CodeChecker()
{
}

void CodeChecker::OnAttach()
{
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_SAVE, new cbEventFunctor<CodeChecker, CodeBlocksEvent>(this, &CodeChecker::OnAnalyze));
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_OPEN, new cbEventFunctor<CodeChecker, CodeBlocksEvent>(this, &CodeChecker::OnAnalyze));
    m_process=new AsyncProcess(this);
    LangData ld;
    ld.command=_T("python -m py_compile $file");
    ld.regexp=_T("File \"([^\\n]*)\", line (\\d*)\n(.*)");
    m_commands[wxSCI_LEX_PYTHON]=ld;
    LogMessage(_("Attached Code Checker"));
    m_queuetimer.SetOwner(this);
}

void CodeChecker::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    delete m_process;
}

void CodeChecker::OnQueueTimer(wxTimerEvent &e)
{
    if(m_processqueue.empty())
        return;
    wxString cmd=m_commands[m_processqueue.begin()->lang].command;
    cmd.Replace(_("$file"),m_processqueue.begin()->file);
    if(m_process->Exec(cmd)!=0)
        m_processqueue.pop_front();
}

void CodeChecker::OnProcessTerminate(wxProcessEvent &e)
{
    wxString out=m_process->GetStdout();
    wxString err=m_process->GetStderr();
    LogMessage(_("Syntax Check Results"));
    LogMessage(out);
    LogMessage(err);
    //TODO: retrieve a handle to the editor control
    //TODO: Parse stdout/stderr
    //TODO: Define the marker (if not defined already)
//    control->MarkerDefine(CHECK_MARKER, CHECK_STYLE);
//    control->MarkerSetForeground(CHECK_MARKER, wxColour(0xFF, 0xA0, 0xA0));
//    control->MarkerSetBackground(CHECK_MARKER, wxColour(0xA0, 0xA0, 0xA0));
//
//    control->MarkerDeleteAll(CHECK_MARKER);
//    if(!control->LineHasMarker(CHECK_MARKER,linenum))
//        control->MarkerAdd(CHECK_MARKER,linenum));
    m_processqueue.pop_front();
}

void CodeChecker::OnAnalyze(CodeBlocksEvent &e)
{
    LogMessage(_("Code Checker: Open or change"));
    cbEditor *ed=(cbEditor *)e.GetEditor();
    if(!ed->IsBuiltinEditor())
    {
        LogMessage(_("Code Checker: Not built in editor"));
        return;
    }
    int lexer=ed->GetControl()->GetLexer();
    if(m_commands.find(lexer)==m_commands.end())
    {
        LogMessage(_("Code Checker: Lexer not found"));
        return;
    }
    ProcessQueueItems p;
    p.lang=lexer;
    p.file=ed->GetFilename();
    ProcessQueue::iterator it=m_processqueue.begin();
    for(;it!=m_processqueue.end();++it)
        if(it->file==p.file)
            break;
    if(it==m_processqueue.begin() || it==m_processqueue.end())
    {
        m_processqueue.push_back(p);
        if(m_processqueue.size()==1)
            m_queuetimer.Start(1000,true);
        LogMessage(_("Code Checker: Request queued"));
    }
}

int CodeChecker::Configure()
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

void CodeChecker::BuildMenu(wxMenuBar* menuBar)
{
    //The application is offering its menubar for your plugin,
    //to add any menu items you want...
    //Append any items you need in the menu...
    //NOTE: Be careful in here... The application's menubar is at your disposal.
    NotImplemented(_T("CodeChecker::BuildMenu()"));
}

void CodeChecker::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    //Some library module is ready to display a pop-up menu.
    //Check the parameter \"type\" and see which module it is
    //and append any items you need in the menu...
    //TIP: for consistency, add a separator as the first item...
    NotImplemented(_T("CodeChecker::BuildModuleMenu()"));
}

bool CodeChecker::BuildToolBar(wxToolBar* toolBar)
{
    //The application is offering its toolbar for your plugin,
    //to add any toolbar items you want...
    //Append any items you need on the toolbar...
    NotImplemented(_T("CodeChecker::BuildToolBar()"));

    // return true if you add toolbar items
    return false;
}
