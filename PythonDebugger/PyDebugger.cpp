#include "PyDebugger.h"
#include <configurationpanel.h>
#include <wx/regex.h>
#include <cbdebugger_interfaces.h>
#include <cbstyledtextctrl.h>
//#include <watchesdlg.h>

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<PyDebugger> reg(_T("PyDebugger"));
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
BEGIN_EVENT_TABLE(PyDebugger, cbDebuggerPlugin)
    EVT_MENU(ID_LangMenu_DebugSendCommand,PyDebugger::OnSendCommand)
    EVT_END_PROCESS(ID_PipedProcess, PyDebugger::OnTerminatePipedProcess)
    EVT_TIMER(ID_TimerPollDebugger, PyDebugger::OnTimer)
END_EVENT_TABLE()



// Updates the Dialog controls to the stored values for the current interpreter
void PyDebugger::ReadPluginConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("PyDebugger"));
    m_DefaultDebugCmdLine=cfg->Read(_T("debug_cmd_line"),_T(" -u -m pdb "));
    m_DefaultInterpreter=cfg->Read(_T("python_executable"),_T("python")); //TODO: make default command platform specific
    m_PythonFileExtensions=cfg->Read(_T("python_file_extensions"),_T("*.py;*.pyc"));
}

// Retrieve configuration values from the dialog widgets and store them appropriately
//void PyDebugger::WritePluginConfig()
//{
//    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("PyDebugger"));
//    cfg->Write(_T("debug_cmd_line"),m_DefaultDebugCmdLine);
//    cfg->Write(_T("python_executable"),m_DefaultInterpreter);
//    cfg->Write(_T("python_file_extensions"),m_PythonFileExtensions);
//}

void PyDebugger::OnPipedOutput(wxCommandEvent& event)
{
    wxMessageBox(_T("Piped output"));
    wxString msg = event.GetString();
    if (!msg.IsEmpty())
    {
//        Manager::Get()->GetMessageManager()->Log(m_PageIndex, _T("O>>> %s"), msg.c_str());
        wxMessageBox(msg);
    }
}

void PyDebugger::OnIdle(wxIdleEvent& event)
{
}

void PyDebugger::OnStep(wxCommandEvent &event)
{
    Step();
}

void PyDebugger::OnStop(wxCommandEvent &event)
{
    Stop();
}

void PyDebugger::OnNext(wxCommandEvent &event)
{
    Next();
}

void PyDebugger::OnContinue(wxCommandEvent &event)
{
    Continue();
}

void PyDebugger::OnSendCommand(wxCommandEvent &event)
{
    if(!m_DebuggerActive /* || m_TimerPollDebugger.IsRunning() */) //could be unsafe, but allows user to provide program input
        return;
    wxString cmd;
    SendCommandDlg dlg(NULL);
    if(dlg.ShowModal()==wxID_OK)
    {
        wxString cmd=dlg.m_cmd->GetValue();
        if(dlg.m_newline->IsChecked())
            cmd+=_T("\n");
        DispatchCommands(cmd,DBGCMDTYPE_USERCOMMAND,true);
    }
}


// sends a newline delimited string of cmdcount debugger commands
bool PyDebugger::DispatchCommands(const wxString& cmd, int cmdtype, bool poll)
{
    if(m_TimerPollDebugger.IsRunning())
        return false;
    if(cmd.Len()>0)
    {
        char *cmdc=new char[cmd.Len()];
        for(size_t i=0;i<cmd.Len();i++)
        {
            cmdc[i]=cmd[i];
            if(cmdc[i]=='\n')
            {
                PythonCmdDispatchData dd;
                dd.cmdtext=cmd;
                dd.type=cmdtype;
                m_DispatchedCommands.push_back(dd);
                m_DebugCommandCount++;
            }
        }
        m_ostream->Write(cmdc,cmd.Len());
        delete[] cmdc;
    }
    if(poll&&m_DebugCommandCount>0)
    {
        m_TimerPollDebugger.Start(50);
    }
    return true;
}

bool PyDebugger::IsPythonFile(const wxString &file) const
{
    if(WildCardListMatch(m_PythonFileExtensions,file))
        return true;
    return false;
}

wxString PyDebugger::AssembleBreakpointCommands()
{
    wxString commands;
    for(BPList::iterator itr=m_bplist.begin();itr!=m_bplist.end();itr++)
    {
        wxString sfile=(*itr)->GetLocation();
        if(sfile.Contains(_T(" ")))
        {
            wxFileName f(sfile);
            sfile=f.GetShortPath();
        }
        int line=(*itr)->GetLine();
        wxString cmd=_T("break ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
        commands+=cmd;
    }
    return commands;
}


wxString PyDebugger::AssembleWatchCommands()
{
    wxString commands;

    for (unsigned int i=0;i<m_watchlist.size();++i)
    { //TODO: ITERATE INTO CHILD WATCHES
        PythonWatch::Pointer w=m_watchlist[i];
        wxString s;
        w->GetSymbol(s);
        commands+=_T("ps ")+s+_T("\n");
    }
    return commands;
}

wxString PyDebugger::AssembleAliasCommands()
{
    wxString commands;
    //NB: \001 is the separator character used when parsing in OnTimer

    //Print variables associated with a child
    commands+=_T("alias pm for x in sorted(%1.__dict__): print '%s\\001%s\\001'%(x,type(%1.__dict__[x])),%1.__dict__[x],'\\001',\n");
    //Print variable name, type and value
    commands+=_T("alias ps print '%1\\001',;print str(type(%1))+'\\001',;print %1\n");
    //Print comment
    commands+=_T("alias pc print '%1'\n");

    //Alias for creating tooltip watch output
    commands+=_T("alias pw print '%1',type(%1);print %1\n");
    return commands;
}


void PyDebugger::ClearActiveMarkFromAllEditors()
{
    EditorManager* edMan = Manager::Get()->GetEditorManager();
    for (int i = 0; i < edMan->GetEditorsCount(); ++i)
    {
        cbEditor* ed = edMan->GetBuiltinEditor(i);
        if (ed)
            ed->SetDebugLine(-1);
    }
}

void PyDebugger::OnTimer(wxTimerEvent& event)
{
    bool debugoutputmode=false;
    if (m_pp && m_pp->IsInputAvailable())
    {
        while(m_pp->IsInputAvailable())
        {
            if(!m_istream->Eof())
                m_outbuf+=m_istream->GetC();
            else
                break;
        }
        //TODO: Program could hang if debug output is incorrectly parsed
        m_outdebugbuf=m_outbuf;

        // Loop over the output, looking for parseable output separated by "(Pdb)" prompts

        while(true)
        {
            wxString s=m_outdebugbuf.Mid(m_debugbufpos);
            int pos=s.Find(_T("(Pdb)")); //position in the string of the Pdb prompt returning from the last call
            if(pos==wxNOT_FOUND)
                break;
            wxString logout=s.Mid(0,pos+5);
            wxString exprresult=s.Mid(0,pos);
            size_t start,len;
            m_DebugCommandCount--;
            PythonCmdDispatchData cmd;
            if(!m_DispatchedCommands.empty())
            {
                cmd=m_DispatchedCommands.front();
                m_DispatchedCommands.pop_front();
            }
            m_debugbufpos+=pos+5;
            if(pos==0)
                continue;
            switch(cmd.type)
            {
                case DBGCMDTYPE_WATCHEXPRESSION:
                {
                    exprresult.RemoveLast();
                    exprresult.Replace(_T("\t"),_T("\\t"),true);
                    exprresult.Replace(_T("\n"),_T("\\n"),true);
                    wxString symbol=exprresult.BeforeFirst(_T('\001'));
                    exprresult=exprresult.AfterFirst(_T('\001'));
                    wxString type=exprresult.BeforeFirst(_T('\001'));
                    wxString value=exprresult.AfterFirst(_T('\001'));
                    for(int i=0;i<m_watchlist.size();++i)
                    {
                        cbWatch::Pointer p;
                        wxString s;
                        m_watchlist[i]->GetSymbol(s);
                        if (s==symbol)
                            p=m_watchlist[i];
                        else
                            p=m_watchlist[i]->FindChild(symbol);
                        if(p)
                        {
                            p->SetType(type);
                            p->SetValue(value);
                        }
                    }
                    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
                    dbg_manager.GetWatchesDialog()->UpdateWatches();
                    break;
                }
                case DBGCMDTYPE_WATCHGETCHILDREN:
                {
                    exprresult.RemoveLast();
                    exprresult.Replace(_T("\t"),_T("\\t"),true);
                    exprresult.Replace(_T("\n"),_T("\\n"),true);
                    wxString parentsymbol=cmd.cmdtext.AfterFirst(_T(' '));
                    parentsymbol=parentsymbol.BeforeLast(_T('\n'));

                    //Find the parent watch and remove it's children
                    PythonWatch::Pointer parentwatch;
                    PythonWatchesContainer::iterator it;
                    for(it=m_watchlist.begin();it!=m_watchlist.end();it++)
                    {
                        wxString symbol;
                        (*it)->GetSymbol(symbol);
                        if(symbol==parentsymbol)
                        {
                            parentwatch=*it;
                            break;
                        }
                    }
                    if(it==m_watchlist.end())
                        break;
                    parentwatch->RemoveChildren();

                    //Now add the newly updated children
                    while(exprresult.Len()>0)
                    {
                        wxString symbol=exprresult.BeforeFirst(_T('\001'));
                        exprresult=exprresult.AfterFirst(_T('\001'));
                        wxString type=exprresult.BeforeFirst(_T('\001'));
                        exprresult=exprresult.AfterFirst(_T('\001'));
                        wxString value=exprresult.BeforeFirst(_T('\001'));
                        exprresult=exprresult.AfterFirst(_T('\001'));
                        PythonWatch::Pointer p(new PythonWatch(symbol));
                        p->SetType(type);
                        p->SetValue(value);
                        cbWatch::AddChild(parentwatch,p);
                    }
                    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
                    dbg_manager.GetWatchesDialog()->UpdateWatches();
                    break;
                }
                case DBGCMDTYPE_WATCHTOOLTIP:
                {
                    wxString output;
                    wxString lines=exprresult;
                    if(lines.EndsWith(_T("\n")))
                        lines=lines.Left(lines.Length()-1);
                    int i=0;
                    int def_end=0;
                    while(lines!=_T(""))
                    {
                        if(i==1)
                            def_end=output.Length();
                        if(i>0)
                            output+=_T("\n");
                        wxString line=lines.BeforeFirst(_T('\n'));
                        lines=lines.AfterFirst(_T('\n'));
                        output+=line;
                        i++;
                        if(i>10)
                            break;
                    }
                    if(lines!=_T(""))
                    {
                        if(lines.BeforeLast(_T('\n'))!=_T(""))
                            output+=_T("\n...");
                        output+=_T("\n")+lines.AfterLast(_T('\n'));
                    }
    //                    exprresult.Replace(_T("\t"),_T("\\t"),true);
    //                    exprresult.Replace(_T("\n"),_T("\\n"),true);
                    SetWatchTooltip(output, def_end);
                    break;
                }
                case DBGCMDTYPE_FLOWCONTROL:
                {
                    wxRegEx re;
                    re.Compile(_T("\\> ([^\\n]*)\\((\\d+)\\)([^\\n]*)\\n([^\\n]*)\\n\\Z"),wxRE_ADVANCED); // Group 1 is the file, Group 2 is the line location, Group 3 is the source of the current line
                    if(re.Matches(exprresult))
                    {
                        m_curfile=re.GetMatch(exprresult,1);
                        re.GetMatch(exprresult,2).ToULong(&m_curline);
                        m_changeposition=true;
                    }
                    re.Compile(_T("Uncaught exception. Entering post mortem debugging\\nRunning 'cont' or 'step' will restart the program"),wxRE_ADVANCED);

    //                re.Compile(_T("Traceback \\(most recent call last\\):\\n  File .*\\n(\\w*Error\\:.+?)\n"),wxRE_ADVANCED); //Traceback notification
                    if(re.Matches(exprresult))
                    {
                        wxString err=re.GetMatch(exprresult,1);
    //                    wxMessageBox(_T("Runtime Error During Debug:\n")+err+_T("\n at line ")+wxString().Format(_T("%u"),m_curline)+_T(" in ")+m_curfile);
                        wxMessageBox(_T("Runtime Error During Debug:\nEntering post mortem debug mode...\nYou may inspect variables by updating the watch\n(Select continue/next to restart or Stop to cancel debug)"));
                    }
                    if(!debugoutputmode) // in standard debug mode, only log flow control information (clearer)
                    {
                        m_DebugLog->Append(cmd.cmdtext);
                        m_DebugLog->Append(logout);
                    }
                    break;
                }

                case DBGCMDTYPE_CALLSTACK:
                {
                    wxRegEx re;
                    re.Compile(_T("[> ] \\<string\\>\\(1\\)\\<module\\>\\(\\)\\n"),wxRE_ADVANCED);
                    if(!re.Matches(exprresult))
                        break;
                    size_t start,len;
                    wxString matchstr;
                    re.GetMatch(&start,&len,0);
                    matchstr=exprresult.Mid(start+len);
                    re.Compile(_T("[> ] ([^\\n]*)\\((\\d+)\\)([^\\n]*)\\n\\-\\> ([^\\n]*)\\n"),wxRE_ADVANCED); // Group 1 is the file, Group 2 is the line location, Group 3 is the source of the current line
                    m_stackinfo.frames.clear();
                    int n=0;
                    while(true)
                    {
                        if(!re.Matches(matchstr))
                            break;
                        wxString file=re.GetMatch(matchstr,1);
                        wxString line=re.GetMatch(matchstr,2);
                        wxString Module=re.GetMatch(matchstr,3);
                        wxString Code=re.GetMatch(matchstr,4);
                        bool active=(re.GetMatch(matchstr,0).Left(1)==_T(">"));
                        m_changeposition=true;
                        cb::shared_ptr<cbStackFrame> f=cb::shared_ptr<cbStackFrame>(new cbStackFrame);
                        f->SetFile(file,line);
                        f->SetNumber(n);
                        f->SetSymbol(Module);
                        f->SetAddress(0);
                        m_stackinfo.frames.push_back(f);
                        if(active)
                        {
                            m_stackinfo.activeframe=n;
                            m_curfile=file;
                            line.ToULong(&m_curline);
                        }
                        re.GetMatch(&start,&len,0);
                        matchstr=matchstr.Mid(start+len);
                        n++;
                    }
                    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
                    dbg_manager.GetBacktraceDialog()->Reload();
                    break;
                }
            }

        }
    }
    if(m_DebugCommandCount==0)
    { //TODO: clear debug and program output strings as well
        if(m_changeposition)
        {
            if(m_curline<1)
            {
                wxMessageBox(_T("Invalid line position reported by PDB"));
                return;
            } else
            {
                SyncEditor(m_curfile,m_curline);
            }
        }
        m_changeposition=false;
        m_outbuf=_T("");
        m_bufpos=0;
        m_outdebugbuf=_T("");
        m_debugbufpos=0;
        m_outprogbuf=_T("");
        m_progbufpos=0;
        m_TimerPollDebugger.Stop();
    }

}


bool PyDebugger::IsAttachedToProcess() const
{
//    return false;
    EditorBase *ed=Manager::Get()->GetEditorManager()->GetActiveEditor();
    if(!ed)
        return false;
    wxString s=ed->GetFilename();
    if(!(wxFileName(s).FileExists() && IsPythonFile(s)))
        return false;
    return true;
}


void PyDebugger::OnDebugTarget(wxCommandEvent &event)
{
    if(m_DebuggerActive)
        return;
    if(m_RunTarget==_T("")||!m_RunTargetSelected)
    {
        OnSetTarget(event);
        if(m_RunTarget==_T(""))
            return;
    }
    m_RunTargetSelected=false;
    if(m_RunTarget.Contains(_T(" ")))
    {
        wxFileName f(m_RunTarget);
        m_RunTarget=f.GetShortPath();
    }
    Debug(false);
}

bool PyDebugger::Debug(bool breakOnEntry)
{
// TODO: need to setup path finding for a pdb runner that will be embedded in the plugins resources
//    m_DefaultDebugCmdLine=_T(" -u c:\\codeblockssrc\\cbpythonplugin\\pdb2.py ");
//

// TODO: figure out why debug watch and breakpoints fail after the first debug session - not erasing the command list with clear()???
    if(m_DebuggerActive)
        return 0;
    m_changeposition=false;
    m_DispatchedCommands.clear();
    m_curline=0;
    m_bufpos=0;
    m_debugbufpos=0;
    m_progbufpos=0;
    m_outbuf=_T("");
    m_outdebugbuf=_T("");
    m_outprogbuf=_T("");
    ReadPluginConfig();
    if(!m_RunTarget)
    {
        EditorBase *ed=Manager::Get()->GetEditorManager()->GetActiveEditor();
        if(!ed)
            return false;
        wxString s=ed->GetFilename();
        if(!(wxFileName(s).FileExists() && IsPythonFile(s)))
            return false;
        m_RunTarget=s;
    }

//    m_DebugLog->Clear();
    m_TimerPollDebugger.SetOwner(this, ID_TimerPollDebugger);
    m_pp=new wxProcess(this,ID_PipedProcess);
    m_pp->Redirect();
    wxString target=m_RunTarget;
    wxString olddir=wxGetCwd();
    wxSetWorkingDirectory(wxFileName(m_RunTarget).GetPath());
    target.Replace(_T("\\"),_T("/"),true);
    wxString f=wxFileName(m_DefaultInterpreter).GetShortPath();
    wxString commandln=f + m_DefaultDebugCmdLine+target;
//    cbMessageBox(commandln);
    #ifdef EXPERIMENTAL_PYTHON_DEBUG
//    LogMessage(wxString::Format(_("Launching '%s': %s (in %s)"), consolename.c_str(), commandstr.c_str(), workingdir.c_str()));
//    InterpretedLangs* plugin = Manager::Get()->GetPluginManager()->LoadPlugin(_T("InterpretedLangs"));
//    m_ilplugin->m_shellmgr->LaunchProcess(commandln,_(T"PyDEBUG"),0);
//    m_ilplugin->ShowConsole();
//    ilShellTermEvent e;
//    ProcessEvent(e);
    #else
    m_pid=wxExecute(commandln,wxEXEC_ASYNC,m_pp);
    if(!m_pid)
    {
        wxSetWorkingDirectory(olddir);
        return -1;
    }
    m_ostream=m_pp->GetOutputStream();
    m_istream=m_pp->GetInputStream();
    #endif

    wxSetWorkingDirectory(olddir);

//Send initial stream of commands
    PythonCmdDispatchData dd;
    dd.type=DBGCMDTYPE_FLOWCONTROL;
    dd.cmdtext=_T("Starting New Python Debug Session...");
    m_DispatchedCommands.push_back(dd);//the debug console reports a command prompt on startup, which should be included in the parse count
    m_DebugCommandCount=1;
    wxString acommands=AssembleAliasCommands();
    DispatchCommands(acommands,DBGCMDTYPE_OTHER,false);
    wxString bpcommands=AssembleBreakpointCommands();
    DispatchCommands(bpcommands,DBGCMDTYPE_BREAKPOINT,false);
    wxString wcommands=AssembleWatchCommands();
    DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
    DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true); //where
    m_DebuggerActive=true;
    return 0;
}

void PyDebugger::Continue()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("cont\n"),DBGCMDTYPE_FLOWCONTROL,false);
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PyDebugger::Next()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("next\n"),DBGCMDTYPE_FLOWCONTROL,false);
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PyDebugger::NextInstruction()
{
    Next();
}

void PyDebugger::Step()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("step\n"),DBGCMDTYPE_FLOWCONTROL,false);
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PyDebugger::StepIntoInstruction()
{
    Step();
}

void PyDebugger::StepOut()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("r\n"),DBGCMDTYPE_FLOWCONTROL,false);
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PyDebugger::Break()
{
    if(!m_DebuggerActive)
        return;
    if(IsStopped())
        return;
    if(m_TimerPollDebugger.IsRunning()) //TODO: there is a risk this will kill the process...
    {
        if(wxProcess::Exists(m_pid))
            wxProcess::Kill(m_pid,wxSIGINT);
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}


void PyDebugger::Stop()
{
    if(m_DebuggerActive)
    {
        if(m_TimerPollDebugger.IsRunning()) //TODO: there is a risk this will kill the process...
        {
            if(wxProcess::Exists(m_pid))
                wxProcess::Kill(m_pid,wxSIGKILL);
        } else
            DispatchCommands(_T("exit\n"));
    }
    m_RunTarget=_("");
}

bool PyDebugger::RunToCursor(const wxString& filename, int line, const wxString& line_text)
{
    if(filename!=m_curfile)
        return false;
    if(!m_DebuggerActive)
        return false;
    wxString sfile=filename;
    if(sfile.Contains(_T(" ")))
    {
        wxFileName f(sfile);
        sfile=f.GetShortPath();
    }
    DispatchCommands(_T("tbreak ")+sfile+wxString::Format(_T(":%i\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
    DispatchCommands(wxString::Format(_T("c\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
    wxString wcommands=AssembleWatchCommands();
    DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
    DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    return true;
}

void PyDebugger::SetNextStatement(const wxString& filename, int line)
{
    if(filename!=m_curfile)
        return;
    if(m_DebuggerActive)
    {
        DispatchCommands(wxString::Format(_T("j %i\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,false);
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}


cb::shared_ptr<cbBreakpoint>  PyDebugger::GetBreakpoint(int index)
{
    return m_bplist[index];
}

cb::shared_ptr<const cbBreakpoint>  PyDebugger::GetBreakpoint(int index) const
{
    return m_bplist[index];
}


cb::shared_ptr<cbBreakpoint>  PyDebugger::AddBreakpoint(const wxString& file, int line)
{
    if(!IsPythonFile(file))
        return cb::shared_ptr<cbBreakpoint>();
    for (BPList::iterator itr=m_bplist.begin(); itr!=m_bplist.end(); ++itr)
    {
        if((*itr)->GetLocation()==file && (*itr)->GetLine()==line)
            return cb::shared_ptr<cbBreakpoint>();
    }
    Pointer p(new PyBreakpoint(file,line));
    m_bplist.push_back(p);
    if(m_DebuggerActive) // if the debugger is running already we need to send a message to the interpreter to add the new breakpoint
    {
        wxString sfile=file;
        if(sfile.Contains(_T(" ")))
        {
            wxFileName f(sfile);
            sfile=f.GetShortPath();
        }
        wxString cmd=_T("break ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
        DispatchCommands(cmd,DBGCMDTYPE_BREAKPOINT);
    }
    return p;
}

void PyDebugger::DeleteBreakpoint(cb::shared_ptr<cbBreakpoint> bp)
{
    if(!IsPythonFile(bp->GetLocation()))
        return;
    wxString sfile=bp->GetLocation();
    int line = bp->GetLine();

    for (size_t i=0;i<m_bplist.size();++i)
    {
        if(m_bplist[i]!=bp)
            continue;
//        if(m_bplist[i]->GetFilename()==bp->GetFilename() && m_bplist[i]->GetLine()==bp->GetLine())
        m_bplist.erase(m_bplist.begin()+i);
        if(m_DebuggerActive)
        {
            if(sfile.Contains(_T(" ")))
            {
                wxFileName f(sfile);
                sfile=f.GetShortPath();
            }
            wxString cmd=_T("clear ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
            DispatchCommands(cmd,DBGCMDTYPE_BREAKPOINT);
         }
         return;
    }
}


void PyDebugger::DeleteAllBreakpoints()
{

    for (size_t i=0;i<m_bplist.size();++i)
    {
        cb::shared_ptr<cbBreakpoint> bp=m_bplist[i];
        wxString sfile=bp->GetLocation();
        int line = bp->GetLine();
        m_bplist.erase(m_bplist.begin()+i);
        if(m_DebuggerActive)
        {
            if(sfile.Contains(_T(" ")))
            {
                wxFileName f(sfile);
                sfile=f.GetShortPath();
            }
            wxString cmd=_T("clear ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
            DispatchCommands(cmd,DBGCMDTYPE_BREAKPOINT);
         }
    }
}


cb::shared_ptr<cbWatch> PyDebugger::AddWatch(const wxString& symbol)
{
    PythonWatch::Pointer pwatch(new PythonWatch(symbol));
    m_watchlist.push_back(pwatch);
    cbWatch::AddChild(pwatch,PythonWatch::Pointer(new PythonWatch(_("#child"))));

    if(IsRunning())
        DispatchCommands(_T("ps ")+symbol+_T("\n"),DBGCMDTYPE_WATCHEXPRESSION);
    return pwatch;
}

void PyDebugger::DeleteWatch(cb::shared_ptr<cbWatch> watch)
{
    //this will delete the root node of watch
    //TODO: Why do we need to delete root node?

//    cb::shared_ptr<cbWatch> root_watch = GetRootWatch(watch);
    unsigned int i;
    for (i=0;i<m_watchlist.size();++i)
    {
        if (m_watchlist[i]==watch)
            break;
    }
    if(i==m_watchlist.size())
        return;

    m_watchlist.erase(m_watchlist.begin()+i);
}

bool PyDebugger::HasWatch(cb::shared_ptr<cbWatch> watch)
{
    //cb::shared_ptr<cbWatch> root_watch = GetRootWatch(watch);
    unsigned int i;
    for (i=0;i<m_watchlist.size();++i)
    {
        if (m_watchlist[i]==watch)
            break;
    }
    return i<m_watchlist.size();
}

void PyDebugger::ShowWatchProperties(cb::shared_ptr<cbWatch> watch)
{
}

bool PyDebugger::SetWatchValue(cb::shared_ptr<cbWatch> watch, const wxString &value)
{
    return false;
}

void PyDebugger::ExpandWatch(cb::shared_ptr<cbWatch> watch)
{
    if(IsRunning())
    {
        wxString symbol;
        watch->GetSymbol(symbol);
        DispatchCommands(_T("pm ")+symbol+_T("\n"),DBGCMDTYPE_WATCHGETCHILDREN);
    }
}

void PyDebugger::CollapseWatch(cb::shared_ptr<cbWatch> watch)
{
    if(IsRunning())
    {
        watch->RemoveChildren();
        cbWatch::AddChild(watch,PythonWatch::Pointer(new PythonWatch(_("...members..."))));
    }
}

void PyDebugger::OnWatchesContextMenu(wxMenu &menu, const cbWatch &watch, wxObject *property)
{
}



int PyDebugger::GetStackFrameCount() const
{
    return m_stackinfo.frames.size();
}

cb::shared_ptr<const cbStackFrame> PyDebugger::GetStackFrame(int index) const
{
    return m_stackinfo.frames[index];
}

void PyDebugger::SwitchToFrame(int number)
{
    if(number<0)
        return;
    if(number>=m_stackinfo.frames.size())
    {
        wxMessageBox(_("Frame out of bounds"));
        return;
    }
    int frames_to_move = number - m_stackinfo.activeframe;
    if(m_DebuggerActive) // if the debugger is running already we need to send a message to the interpreter to add the new breakpoint
    {
        if(frames_to_move>0)
        {
            for(int i=0;i<frames_to_move;++i)
            {
                wxString cmd=_T("down\n");
                DispatchCommands(cmd,DBGCMDTYPE_OTHER,false);
            }
            DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
        }
        if(frames_to_move<0)
        {
            for(int i=0;i<-frames_to_move;++i)
            {
                wxString cmd=_T("up\n");
                DispatchCommands(cmd,DBGCMDTYPE_OTHER,false);
            }
            DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
        }
    }
}

int PyDebugger::GetActiveStackFrame() const
{
    return m_stackinfo.activeframe;
}

void PyDebugger::OnRunPiped(wxCommandEvent &event)
{
    m_RunTarget=_T("");
    OnDebugTarget(event);
}

void PyDebugger::OnTerminatePipedProcess(wxProcessEvent &event)
{
//    wxMessageBox(_("Debug Terminated"));
    ClearActiveMarkFromAllEditors();
    m_DispatchedCommands.clear();
    m_DebuggerActive=false;
    m_TimerPollDebugger.Stop();
    delete m_pp;
    m_DebugLog->Append(_T("\n*** SESSION TERMINATED ***"));
}

void PyDebugger::OnSettings(wxCommandEvent& event)
{
    wxMessageBox(_T("Settings..."));
}

void PyDebugger::OnSubMenuSelect(wxUpdateUIEvent& event)
{
}

void PyDebugger::OnSetTarget(wxCommandEvent& event)
{
    //TODO: use default file extensions
    wxFileDialog *fd=new wxFileDialog(NULL,_T("Choose the interpreter Target"),_T(""),_T(""),m_PythonFileExtensions,wxOPEN|wxFILE_MUST_EXIST);
    if(fd->ShowModal()==wxID_OK)
    {
        m_RunTarget=fd->GetPath();
    } else
        m_RunTarget=_T("");
    delete fd;
}

void PyDebugger::OnRunTarget(wxCommandEvent& event)
{
}

void PyDebugger::OnRun(wxCommandEvent& event)
{
    if(!m_RunTargetSelected)
        OnSetTarget(event);
    m_RunTargetSelected=false;
    if(m_RunTarget==_T(""))
        return;
    if(m_RunTarget.Contains(_T(" ")))
    {
        wxFileName f(m_RunTarget);
        m_RunTarget=f.GetShortPath();
    }
    ReadPluginConfig();
    wxString target=m_RunTarget;
    wxString olddir=wxGetCwd();
    wxSetWorkingDirectory(wxFileName(m_RunTarget).GetPath());
    target.Replace(_T("\\"),_T("/"),true);
    wxString commandln=wxFileName(m_DefaultInterpreter).GetShortPath()+_T(" ")+target;
    wxExecute(commandln,wxEXEC_ASYNC,NULL);
    wxSetWorkingDirectory(olddir);
}


// constructor
PyDebugger::PyDebugger() : cbDebuggerPlugin(_T("PyDebugger"),_T("py_debugger"))
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("PyDebugger.zip")))
    {
        NotifyMissingFile(_T("PyDebugger.zip"));
    }
    m_DebuggerActive=false;
    m_RunTargetSelected=false;

}

cbConfigurationPanel* PyDebugger::GetConfigurationPanel(wxWindow* parent)
{
//    MyDialog* dlg = new MyDialog(this, *m_pKeyProfArr, parent,
//        wxT("Keybindings"), mode);

    return new ConfigDialog(parent, this);
}

// destructor
PyDebugger::~PyDebugger()
{

}

void PyDebugger::OnAttachReal()
{
	// do whatever initialization you need for your plugin
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be TRUE...
	// You should check for it in other functions, because if it
	// is FALSE, it means that the application did *not* "load"
	// (see: does not need) this plugin...
    this->ReadPluginConfig();
    this->UpdateConfig();
    m_DebugLog = new TextCtrlLogger(true);

    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_TOOLTIP, new cbEventFunctor<PyDebugger, CodeBlocksEvent>(this, &PyDebugger::OnValueTooltip));

    CodeBlocksLogEvent evtlog(cbEVT_ADD_LOG_WINDOW,m_DebugLog, _("PyDebugger"));
    Manager::Get()->ProcessEvent(evtlog);

    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
    dbg_manager.RegisterDebugger(this);

}

void PyDebugger::OnReleaseReal(bool appShutDown)
{
	// do de-initialization for your plugin
	// if appShutDown is false, the plugin is unloaded because Code::Blocks is being shut down,
	// which means you must not use any of the SDK Managers
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be FALSE...

	//TODO: Handle destruction of any running python debugger processes? (Maybe not necessary...)

    CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW,m_DebugLog);
    Manager::Get()->ProcessEvent(evt);
    m_DebugLog = 0L;

    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
    dbg_manager.UnregisterDebugger(this);

}

void PyDebugger::SetWatchTooltip(const wxString &tip, int definition_length)
{
    EditorManager* edMan = Manager::Get()->GetEditorManager();
    EditorBase* base = edMan->GetActiveEditor();
    cbEditor* ed = base && base->IsBuiltinEditor() ? static_cast<cbEditor*>(base) : 0;
    if (!ed)
        return;
    ed->GetControl()->CallTipShow(m_watch_tooltip_pos, tip);
    ed->GetControl()->CallTipSetHighlight(0,definition_length);

}

void PyDebugger::OnValueTooltip(CodeBlocksEvent& event)
{
    event.Skip();
    if (!m_DebuggerActive)
        return;
    if (!IsStopped())
        return;

    EditorBase* base = event.GetEditor();
    cbEditor* ed = base && base->IsBuiltinEditor() ? static_cast<cbEditor*>(base) : 0;
    if (!ed)
        return;

    if(ed->IsContextMenuOpened())
    {
    	return;
    }

	// get rid of other calltips (if any) [for example the code completion one, at this time we
	// want the debugger value call/tool-tip to win and be shown]
    if(ed->GetControl()->CallTipActive())
    {
    	ed->GetControl()->CallTipCancel();
    }

    const int style = event.GetInt();
    if (style != wxSCI_P_DEFAULT && style != wxSCI_P_OPERATOR && style != wxSCI_P_IDENTIFIER && style != wxSCI_P_CLASSNAME)
        return;

    wxPoint pt;
    pt.x = event.GetX();
    pt.y = event.GetY();
    int pos = ed->GetControl()->PositionFromPoint(pt);
    int start = ed->GetControl()->WordStartPosition(pos, true);
    int end = ed->GetControl()->WordEndPosition(pos, true);
    while(ed->GetControl()->GetCharAt(start-1)==_T('.'))
        start=ed->GetControl()->WordStartPosition(start-2, true);
    wxString token;
    if (start >= ed->GetControl()->GetSelectionStart() &&
        end <= ed->GetControl()->GetSelectionEnd())
    {
        token = ed->GetControl()->GetSelectedText();
    }
    else
        token = ed->GetControl()->GetTextRange(start,end);
    if (token.IsEmpty())
        return;

    wxString cmd;
    cmd+=_T("pw ")+token+_T("\n");
    DispatchCommands(cmd,DBGCMDTYPE_WATCHTOOLTIP,true);
    m_watch_tooltip_pos=pos;
}


int PyDebugger::Configure()
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

void PyDebugger::CreateMenu()
{
    //TODO: This should all disappear and use native SDK implementations
//    LangMenu->Append(ID_LangMenu_Run,_T("Python &Run..."),_T(""));
//    LangMenu->AppendSeparator();
//    LangMenu->Append(ID_LangMenu_RunPiped,_T("Start &Debug..."),_T(""));
//    LangMenu->Append(XRCID("idPyDebuggerMenuDebug"),_T("&Continue"),_T(""));
//    LangMenu->Append(XRCID("idPyDebuggerMenuNext"),_T("&Next"),_T(""));
//    LangMenu->Append(XRCID("idPyDebuggerMenuStep"),_T("Step &Into"),_T(""));
//    LangMenu->Append(XRCID("idPyDebuggerMenuStop"),_T("&Stop"),_T(""));
    LangMenu->Append(ID_LangMenu_DebugSendCommand,_T("&Send Debugger Command"),_T(""));
//    LangMenu->Append(ID_LangMenu_ShowWatch,_T("Toggle &Watch"),_T(""),wxITEM_CHECK);
//    LangMenu->Append(ID_LangMenu_UpdateWatch,_T("&Update Watch"),_T(""));
}


void PyDebugger::UpdateMenu()
{
}


void PyDebugger::SetupToolsMenu(wxMenu& menuBar)
{
    //TODO: This should all be deleted once native implementations are in place
	//The application is offering its menubar for your plugin,
	//to add any menu items you want...
	//Append any items you need in the menu...
	//NOTE: Be careful in here... The application's menubar is at your disposal.
	LangMenu=new wxMenu;
	CreateMenu();
	//int pos = menuBar.FindMenu(_("P&lugins"));
	int pos=-1;
	if(pos!=wxNOT_FOUND)
        menuBar.AppendSubMenu(LangMenu, _("P&ython Debugger"));
    else
    {
        delete LangMenu;
        LangMenu=0;
    }
}

void PyDebugger::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
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
                    if(IsPythonFile(name))
                    {
                        m_RunTarget=name;
                        m_RunTargetSelected=true;
                        menu->AppendSeparator();
//                        menu->Append(ID_LangMenu_Run,_T("Python Run"),_T(""));
                        menu->Append(ID_LangMenu_RunPiped,_T("Python Debug"),_T(""));
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
        if(IsPythonFile(name))
        {
            m_RunTarget=name;
            m_RunTargetSelected=true;
            menu->AppendSeparator();
//            menu->Append(ID_LangMenu_Run,_T("Python Run"),_T(""));
            menu->Append(ID_LangMenu_RunPiped,_T("Python Debug"),_T(""));

        }
	}
	if(type==mtUnknown) // also type==mtOpenFilesList - not sure how to find out which file has been right clicked.
	{
        if(data->GetKind()==FileTreeData::ftdkFile)  //right clicked on folder in file explorer
        {
            wxFileName f(data->GetFolder());
            wxString filename=f.GetFullPath();
            wxString name=f.GetFullName();
//            cbMessageBox(filename+_T("  ")+name);
            if(IsPythonFile(name))
            {
                m_RunTarget=filename;
                m_RunTargetSelected=true;
                menu->AppendSeparator();
//            menu->Append(ID_LangMenu_Run,_T("Python Run"),_T(""));
                menu->Append(ID_LangMenu_RunPiped,_T("Python Debug"),_T(""));

            }
        }
	}
}

