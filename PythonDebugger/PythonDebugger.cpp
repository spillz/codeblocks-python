#include "PythonDebugger.h"
#include <configurationpanel.h>
#include <wx/regex.h>
#include <cbdebugger_interfaces.h>
#include <cbstyledtextctrl.h>
//#include <watchesdlg.h>

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<PythonDebugger> reg(_T("PythonDebugger"));
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
int ID_LangMenu_ShowWatch=wxNewId();
int ID_LangMenu_UpdateWatch=wxNewId();

int ID_PipedProcess=wxNewId();
int ID_TimerPollDebugger=wxNewId();

//assign menu IDs to correspond with toolbar buttons
int ID_LangMenu_RunPiped = wxNewId();//XRCID("idPyDebuggerMenuDebug");

// events handling
BEGIN_EVENT_TABLE(PythonDebugger, cbDebuggerPlugin)
    EVT_END_PROCESS(ID_PipedProcess, PythonDebugger::OnTerminatePipedProcess)
    EVT_TIMER(ID_TimerPollDebugger, PythonDebugger::OnTimer)
END_EVENT_TABLE()


void PythonDebugger::SendCommand(const wxString& cmd, bool debugLog)
{
    wxString scmd = cmd;
    if(!m_DebuggerActive) //could be unsafe, but allows user to provide program input
        return;
    if(!scmd.EndsWith(_T("\n")))
        scmd+=_T("\n");
    DispatchCommands(scmd,DBGCMDTYPE_USERCOMMAND,true);
}

bool PythonDebugger::SupportsFeature(cbDebuggerFeature::Flags f)
{
    switch(f)
    {
        case cbDebuggerFeature::Breakpoints:
        case cbDebuggerFeature::Callstack:
        case cbDebuggerFeature::Watches:
        case cbDebuggerFeature::ValueTooltips:
//        case cbDebuggerFeature::Threads: //enable for rpdb2
        case cbDebuggerFeature::RunToCursor:
        case cbDebuggerFeature::SetNextStatement:
            return true;
        default:
            return false;
    }
    return true;
}


// sends a newline delimited string of cmdcount debugger commands
bool PythonDebugger::DispatchCommands(const wxString& cmd, int cmdtype, bool poll)
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


static wxString PythonFileExtensions=wxT("*.py;*.pyc");

bool PythonDebugger::IsPythonFile(const wxString &file) const
{
    if(WildCardListMatch(PythonFileExtensions,file))
        return true;
    return false;
}

wxString PythonDebugger::AssembleBreakpointCommands()
{
    wxString commands;
    for(BPList::iterator itr=m_bplist.begin();itr!=m_bplist.end();itr++)
    {
        wxString sfile=(*itr)->GetLocation();
//        if(sfile.Contains(_T(" ")))
//        {
//            wxFileName f(sfile);
//            sfile=f.GetShortPath();
//        }
        int line=(*itr)->GetLine();
        wxString cmd=_T("break ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
        commands+=cmd;
    }
    return commands;
}


void PythonDebugger::DispatchWatchCommands()
{
    wxString command;

    for (unsigned int i=0;i<m_watchlist.size();++i)
    { //TODO: ITERATE INTO CHILD WATCHES
        PythonWatch::Pointer w=m_watchlist[i];
        wxString s;
        w->GetSymbol(s);
        command=_T("ps ")+s+_T("\n");
        DispatchCommands(command,DBGCMDTYPE_WATCHEXPRESSION,false);
        if (w->IsExpanded())
        {
            command=_T("pm ")+s+_T("\n");
            DispatchCommands(command,DBGCMDTYPE_WATCHGETCHILDREN,false);
        }
    }
    if (m_locals_watch->IsExpanded())
    {
        command=_T("pl *Locals:\n");
        DispatchCommands(command,DBGCMDTYPE_WATCHGETCHILDREN,false);
    }
}

wxString PythonDebugger::AssembleAliasCommands()
{
    wxString commands;
    //NB: \001 is the separator character used when parsing in OnTimer
    //Print variables associated with a child
    commands+=_T("alias pm for __x in sorted(%1.__dict__): print '%s\\001%s\\001'%(__x,type(%1.__dict__[__x])),str(%1.__dict__[__x])[:1200],'\\001',\n");
    //Print all local variables
    commands+=_T("alias pl for __x in sorted(locals()): print '%s\\001%s\\001'%(__x,type(locals()[__x])),str(locals()[__x])[:1200],'\\001',\n");
    //Print variable name, type and value
    commands+=_T("alias ps print '%1\\001',;print str(type(%1))+'\\001',;print str(%1)[:1200]\n");
    //Print comment
    commands+=_T("alias pc print '%1'\n");
    //TODO: Print function arguments too (currently included in locals)

    //Alias for creating tooltip watch output
    commands+=_T("alias pw print '%1',type(%1);print str(%1)[:1200]\n");
    return commands;
}

void PythonDebugger::RequestUpdate(DebugWindows window)
{
//    switch (window)
//    {
//        case Backtrace:
//            RunCommand(CMD_BACKTRACE);
//            break;
//        case CPURegisters:
//            RunCommand(CMD_REGISTERS);
//            break;
//        case Disassembly:
//            RunCommand(CMD_DISASSEMBLE);
//            break;
//        case ExamineMemory:
//            RunCommand(CMD_MEMORYDUMP);
//            break;
//        case Threads:
//            RunCommand(CMD_RUNNINGTHREADS);
//            break;
//        case Watches:
//            if (IsWindowReallyShown(Manager::Get()->GetDebuggerManager()->GetWatchesDialog()->GetWindow()))
//            {
//                if (!m_locals_watch)
//                {
//                    m_locals_watch = PythonWatch::Pointer(new PythonWatch(_T("*Locals:")));
//                    cbWatch::AddChild(m_locals_watch,PythonWatch::Pointer(new PythonWatch(_("#child"))));
//                    Manager::Get()->GetDebuggerManager()->GetWatchesDialog()->AddSpecialWatch(m_locals_watch,false);
//                }
//            }
//            break;
//        default:
//            break;
//    }

}



void PythonDebugger::ClearActiveMarkFromAllEditors()
{
    EditorManager* edMan = Manager::Get()->GetEditorManager();
    for (int i = 0; i < edMan->GetEditorsCount(); ++i)
    {
        cbEditor* ed = edMan->GetBuiltinEditor(i);
        if (ed)
            ed->SetDebugLine(-1);
    }
}

void RemoveMissingChildren(cb::shared_ptr<cbWatch> parent, const std::set<wxString> &knownsyms)
{
    for (int i=0; i<parent->GetChildCount(); ++i)
    {
        cb::shared_ptr<cbWatch> p(parent->GetChild(i));
        wxString s;
        p->GetSymbol(s);
        if (knownsyms.find(s)==knownsyms.end())
        {
            parent->RemoveChild(i);
            --i;
            continue;
        }
    }
}

void PythonDebugger::OnTimer(wxTimerEvent& event)
{
    bool debugoutputmode=false;
    if (m_pp && m_pp->IsInputAvailable())
    {
        while(m_pp->IsInputAvailable())
        {
            int c;
            if (m_istream->CanRead())
                c = m_istream->GetC();
            if (m_istream->LastRead()>0)
                m_outbuf += c;
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
                    wxString symbol=exprresult.BeforeFirst(_T('\001')).Trim(false);
                    exprresult=exprresult.AfterFirst(_T('\001'));
                    wxString type=exprresult.BeforeFirst(_T('\001')).Trim().Trim(false);
                    wxString value=exprresult.AfterFirst(_T('\001'));
                    for(size_t i=0;i<m_watchlist.size();++i)
                    {
                        cb::shared_ptr<cbWatch> p;
                        wxString s;
                        m_watchlist[i]->GetSymbol(s);
                        if (s==symbol)
                            p=m_watchlist[i];
                        else
                            p=m_watchlist[i]->FindChild(symbol);
                        if(p)
                        {
                            wxString oldval,oldtype;
                            p->GetValue(oldval);
                            p->GetType(oldtype);
                            p->MarkAsChanged(oldval!=value || oldtype!=type);
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
                    PythonWatch::Pointer parentwatch, pw;
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
                    {
                        if (parentsymbol == _("*Locals:"))
                            parentwatch = m_locals_watch;
                        else
                            break;
                    }
//                    parentwatch->RemoveChildren();
//                    parentwatch->MarkAsChanged(true);

                    //Now add the newly updated children
                    std::set<wxString> foundsyms;
                    foundsyms.insert(_T("*Modules:"));
                    foundsyms.insert(_T("*Classes:"));
                    foundsyms.insert(_T("*Functions:"));
                    while (exprresult.Len()>0)
                    {
                        wxString symbol=exprresult.BeforeFirst(_T('\001')).Trim(false);
                        exprresult=exprresult.AfterFirst(_T('\001'));
                        wxString type=exprresult.BeforeFirst(_T('\001')).Trim(false).Trim();
                        exprresult=exprresult.AfterFirst(_T('\001'));
                        wxString value=exprresult.BeforeFirst(_T('\001'));
                        exprresult=exprresult.AfterFirst(_T('\001'));
                        foundsyms.insert(symbol);
                        cb::shared_ptr<cbWatch> p = parentwatch->FindChild(symbol);
                        pw = parentwatch;
                        if (parentwatch == m_locals_watch)
                        {
                            if(!p && type == _("<type 'module'>"))
                            {
                                p = m_modules_watch->FindChild(symbol);
                                pw = m_modules_watch;
                            }
                            if(!p && type == _("<type 'classobj'>"))
                            {
                                p = m_classes_watch->FindChild(symbol);
                                pw = m_classes_watch;
                            }
                            if(!p && type == _("<type 'function'>"))
                            {
                                p = m_functions_watch->FindChild(symbol);
                                pw = m_functions_watch;
                            }
                        }
                        if(!p)
                        {
                            p = cb::shared_ptr<cbWatch>(new PythonWatch(symbol));
                            cbWatch::AddChild(pw,p);
                        }
                        wxString oldval,oldtype;
                        p->GetValue(oldval);
                        p->GetType(oldtype);
                        p->MarkAsChanged(oldval!=value || oldtype!=type);
                        p->SetType(type);
                        p->SetValue(value);
                    }
                    RemoveMissingChildren(parentwatch, foundsyms);
                    if (parentwatch == m_locals_watch)
                    {
                        RemoveMissingChildren(m_functions_watch, foundsyms);
                        RemoveMissingChildren(m_classes_watch, foundsyms);
                        RemoveMissingChildren(m_modules_watch, foundsyms);
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
                        wxMessageBox(_T("Runtime Error During Debug:\nEntering post mortem debug mode...\nYou may inspect variables by updating the watch\n(Select continue/next to restart or Stop to cancel debug)\n\nError:\n")+err);
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


bool PythonDebugger::IsAttachedToProcess() const
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


bool PythonDebugger::Debug(bool breakOnEntry)
{
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
    if(!m_RunTarget)
    {
        cbEditor *ed=Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
        if(!ed)
            return false;
        wxString s=ed->GetFilename();
        if(!ed->GetControl()->GetLexer()==wxSCI_LEX_PYTHON)
        {
            if(!(wxFileName(s).FileExists() && IsPythonFile(s)))
                return false;
        }
        m_RunTarget=s;
    }

    m_TimerPollDebugger.SetOwner(this, ID_TimerPollDebugger);
    m_pp=new wxProcess(this,ID_PipedProcess);
    m_pp->Redirect();
    wxString target=m_RunTarget;
    wxString olddir=wxGetCwd();
    wxSetWorkingDirectory(wxFileName(m_RunTarget).GetPath());
    target.Replace(_T("\\"),_T("/"),true);

    PyDebuggerConfiguration &cfg =  GetActiveConfigEx();
    wxString commandln = cfg.GetCommandLine(cfg.GetState());
    commandln.Replace(wxT("$target"),target);

    //TODO:
    //read args for this target from config
    //commandln.Replace(wxT("$args"),args);

    Manager::Get()->GetLogManager()->Log(_T("Running python debugger with command line\n")+commandln);
    m_pid=wxExecute(commandln,wxEXEC_ASYNC,m_pp);
    if(!m_pid)
    {
        wxSetWorkingDirectory(olddir);
        return -1;
    }
    m_ostream=m_pp->GetOutputStream();
    m_istream=m_pp->GetInputStream();

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
    DispatchWatchCommands();
    DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true); //where
    m_DebuggerActive=true;

    CodeBlocksLogEvent evtlog(cbEVT_SWITCH_TO_LOG_WINDOW,m_DebugLog);
    Manager::Get()->ProcessEvent(evtlog);

    return 0;
}

void PythonDebugger::Continue()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("cont\n"),DBGCMDTYPE_FLOWCONTROL,false);
        DispatchWatchCommands();
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PythonDebugger::Next()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("next\n"),DBGCMDTYPE_FLOWCONTROL,false);
        DispatchWatchCommands();
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PythonDebugger::NextInstruction()
{
    Next();
}

void PythonDebugger::Step()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("step\n"),DBGCMDTYPE_FLOWCONTROL,false);
        DispatchWatchCommands();
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PythonDebugger::StepIntoInstruction()
{
    Step();
}

void PythonDebugger::StepOut()
{
    if(m_DebuggerActive)
    {
        DispatchCommands(_T("r\n"),DBGCMDTYPE_FLOWCONTROL,false);
        DispatchWatchCommands();
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}

void PythonDebugger::Break()
{
    if(!m_DebuggerActive)
        return;
    if(IsStopped())
        return;
    if(m_TimerPollDebugger.IsRunning()) //TODO: there is a risk this will kill the process...
    {
        if(wxProcess::Exists(m_pid))
            wxProcess::Kill(m_pid,wxSIGINT);
        DispatchWatchCommands();
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}


void PythonDebugger::Stop()
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

bool PythonDebugger::RunToCursor(const wxString& filename, int line, const wxString& line_text)
{
//    if(filename!=m_curfile)
//        return false;
    if(!m_DebuggerActive)
        return false;
    wxString sfile=filename;
//    if(sfile.Contains(_T(" ")))
//    {
//        wxFileName f(sfile);
//        sfile=f.GetShortPath();
//    }
    DispatchCommands(_T("tbreak ")+sfile+wxString::Format(_T(":%i\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
    DispatchCommands(wxString::Format(_T("c\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
    DispatchWatchCommands();
    DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    return true;
}

void PythonDebugger::SetNextStatement(const wxString& filename, int line)
{
    if(filename!=m_curfile)
        return;
    if(m_DebuggerActive)
    {
        DispatchCommands(wxString::Format(_T("j %i\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
        DispatchWatchCommands();
        DispatchCommands(_T("w\n"),DBGCMDTYPE_CALLSTACK,true);
    }
}


cb::shared_ptr<cbBreakpoint>  PythonDebugger::GetBreakpoint(int index)
{
    return m_bplist[index];
}

cb::shared_ptr<const cbBreakpoint>  PythonDebugger::GetBreakpoint(int index) const
{
    return m_bplist[index];
}


cb::shared_ptr<cbBreakpoint>  PythonDebugger::AddBreakpoint(const wxString& file, int line)
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
//        if(sfile.Contains(_T(" ")))
//        {
//            wxFileName f(sfile);
//            sfile=f.GetShortPath();
//        }
        wxString cmd=_T("break ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
        DispatchCommands(cmd,DBGCMDTYPE_BREAKPOINT);
    }
    return p;
}

void PythonDebugger::DeleteBreakpoint(cb::shared_ptr<cbBreakpoint> bp)
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
//            if(sfile.Contains(_T(" ")))
//            {
//                wxFileName f(sfile);
//                sfile=f.GetShortPath();
//            }
            wxString cmd=_T("clear ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
            DispatchCommands(cmd,DBGCMDTYPE_BREAKPOINT);
         }
         return;
    }
}


void PythonDebugger::DeleteAllBreakpoints()
{

    for (size_t i=0;i<m_bplist.size();++i)
    {
        cb::shared_ptr<cbBreakpoint> bp=m_bplist[i];
        wxString sfile=bp->GetLocation();
        int line = bp->GetLine();
        m_bplist.erase(m_bplist.begin()+i);
        if(m_DebuggerActive)
        {
//            if(sfile.Contains(_T(" ")))
//            {
//                wxFileName f(sfile);
//                sfile=f.GetShortPath();
//            }
            wxString cmd=_T("clear ")+sfile+_T(":")+wxString::Format(_T("%i"),line)+_T("\n");
            DispatchCommands(cmd,DBGCMDTYPE_BREAKPOINT);
         }
    }
}


cb::shared_ptr<cbWatch> PythonDebugger::AddWatch(const wxString& symbol)
{
    wxString sym(symbol);
    sym = sym.Trim().Trim(false);
    PythonWatch::Pointer pwatch(new PythonWatch(sym));
    m_watchlist.push_back(pwatch);
    cbWatch::AddChild(pwatch,PythonWatch::Pointer(new PythonWatch(_("#child"))));

    if(IsRunning())
        DispatchCommands(_T("ps ")+sym+_T("\n"),DBGCMDTYPE_WATCHEXPRESSION);
    return pwatch;
}

void PythonDebugger::DeleteWatch(cb::shared_ptr<cbWatch> watch)
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

bool PythonDebugger::HasWatch(cb::shared_ptr<cbWatch> watch)
{
    //cb::shared_ptr<cbWatch> root_watch = GetRootWatch(watch);
    unsigned int i;
    for (i=0;i<m_watchlist.size();++i)
    {
        if (m_watchlist[i]==watch)
            return true;
    }
    return watch == m_functions_watch
        || watch == m_classes_watch
        || watch == m_modules_watch
        || watch == m_locals_watch;
}

void PythonDebugger::ShowWatchProperties(cb::shared_ptr<cbWatch> watch)
{
}

bool PythonDebugger::SetWatchValue(cb::shared_ptr<cbWatch> watch, const wxString &value)
{
    return false;
}

void PythonDebugger::ExpandWatch(cb::shared_ptr<cbWatch> watch)
{
    if(IsRunning())
    {
        if (watch == m_locals_watch)
            DispatchCommands(_T("pl *Locals:\n"),DBGCMDTYPE_WATCHGETCHILDREN);
        else
        {
            wxString symbol;
            watch->GetSymbol(symbol);
            DispatchCommands(_T("pm ")+symbol+_T("\n"),DBGCMDTYPE_WATCHGETCHILDREN);
        }
    }
}

void PythonDebugger::CollapseWatch(cb::shared_ptr<cbWatch> watch)
{
    if(IsRunning())
    {
//        watch->RemoveChildren();
//        cbWatch::AddChild(watch,PythonWatch::Pointer(new PythonWatch(_("...members..."))));
    }
}

void PythonDebugger::UpdateWatch(cb::shared_ptr<cbWatch> watch)
{
    if(IsRunning())
    {
//        watch->RemoveChildren(); //TODO: Update instead of removing children

        wxString symbol;
        watch->GetSymbol(symbol);
        DispatchCommands(_T("ps ")+symbol+_T("\n"),DBGCMDTYPE_WATCHEXPRESSION);
    }
}

void PythonDebugger::OnWatchesContextMenu(wxMenu &menu, const cbWatch &watch, wxObject *property)
{
}



int PythonDebugger::GetStackFrameCount() const
{
    return m_stackinfo.frames.size();
}

cb::shared_ptr<const cbStackFrame> PythonDebugger::GetStackFrame(int index) const
{
    return m_stackinfo.frames[index];
}

void PythonDebugger::SwitchToFrame(int number)
{
    if(number<0)
        return;
    if(number>=(int)m_stackinfo.frames.size())
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

int PythonDebugger::GetActiveStackFrame() const
{
    return m_stackinfo.activeframe;
}

//void PythonDebugger::OnRunPiped(wxCommandEvent &event)
//{
//    m_RunTarget=_T("");
//    OnDebugTarget(event);
//}

void PythonDebugger::OnTerminatePipedProcess(wxProcessEvent &event)
{
//    wxMessageBox(_("Debug Terminated"));
    ClearActiveMarkFromAllEditors();
    m_DispatchedCommands.clear();
    m_DebuggerActive=false;
    m_TimerPollDebugger.Stop();
    delete m_pp;
    m_DebugLog->Append(_T("\n*** SESSION TERMINATED ***"));
    MarkAsStopped();
}

// constructor
PythonDebugger::PythonDebugger() : cbDebuggerPlugin(_T("PythonDebugger"),_T("py_debugger"))
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("PythonDebugger.zip")))
    {
        NotifyMissingFile(_T("PythonDebugger.zip"));
    }
    m_DebuggerActive=false;
    m_RunTargetSelected=false;

}

cbConfigurationPanel* PythonDebugger::GetConfigurationPanel(wxWindow* parent)
{
//    MyDialog* dlg = new MyDialog(this, *m_pKeyProfArr, parent,
//        wxT("Keybindings"), mode);

    return NULL;//new ConfigDialog(parent, this);
}

// destructor
PythonDebugger::~PythonDebugger()
{

}

void PythonDebugger::OnAttachReal()
{
	// do whatever initialization you need for your plugin
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be TRUE...
	// You should check for it in other functions, because if it
	// is FALSE, it means that the application did *not* "load"
	// (see: does not need) this plugin...

    m_DebugLog = new TextCtrlLogger(true);
    CodeBlocksLogEvent evtlog(cbEVT_ADD_LOG_WINDOW,m_DebugLog, _("PythonDebugger"));
    Manager::Get()->ProcessEvent(evtlog);

    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
    dbg_manager.RegisterDebugger(this);

    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_TOOLTIP, new cbEventFunctor<PythonDebugger, CodeBlocksEvent>(this, &PythonDebugger::OnValueTooltip));

    m_locals_watch = PythonWatch::Pointer(new PythonWatch(_T("*Locals:")));
    m_locals_watch->MarkAsChanged(false);
    Manager::Get()->GetDebuggerManager()->GetWatchesDialog()->AddSpecialWatch(m_locals_watch,true);

    m_functions_watch = PythonWatch::Pointer(new PythonWatch(_T("*Functions:")));
    m_functions_watch->MarkAsChanged(false);
    cbWatch::AddChild(m_locals_watch,m_functions_watch);

    m_classes_watch = PythonWatch::Pointer(new PythonWatch(_T("*Classes:")));
    m_classes_watch->MarkAsChanged(false);
    cbWatch::AddChild(m_locals_watch,m_classes_watch);

    m_modules_watch = PythonWatch::Pointer(new PythonWatch(_T("*Modules:")));
    m_modules_watch->MarkAsChanged(false);
    cbWatch::AddChild(m_locals_watch,m_modules_watch);

    m_locals_watch->Expand(true);
    Manager::Get()->GetDebuggerManager()->GetWatchesDialog()->UpdateWatches();
}

void PythonDebugger::OnReleaseReal(bool appShutDown)
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

//    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
//    dbg_manager.UnregisterDebugger(this);
    Manager::Get()->GetDebuggerManager()->GetWatchesDialog()->RemoveWatch(m_locals_watch);
    m_locals_watch = PythonWatch::Pointer();
    m_functions_watch = PythonWatch::Pointer();
    m_classes_watch = PythonWatch::Pointer();
}

void PythonDebugger::SetWatchTooltip(const wxString &tip, int definition_length)
{
    EditorManager* edMan = Manager::Get()->GetEditorManager();
    EditorBase* base = edMan->GetActiveEditor();
    cbEditor* ed = base && base->IsBuiltinEditor() ? static_cast<cbEditor*>(base) : 0;
    if (!ed)
        return;
    ed->GetControl()->CallTipShow(m_watch_tooltip_pos, tip);
    ed->GetControl()->CallTipSetHighlight(0,definition_length);

}

void PythonDebugger::OnValueTooltip(CodeBlocksEvent& event)
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

