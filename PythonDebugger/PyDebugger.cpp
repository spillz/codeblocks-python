#include "PyDebugger.h"
#include <configurationpanel.h>
#include <wx/regex.h>
#include <backtracedlg.h>


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
	// add any events you want to handle here
////    EVT_MENU(ID_LangMenu_Run,PyDebugger::OnRun)
////    EVT_MENU(ID_LangMenu_RunPiped,PyDebugger::OnDebugTarget)
////    EVT_MENU(XRCID("idPyDebuggerMenuDebug"),PyDebugger::OnContinue)
////    EVT_MENU(XRCID("idPyDebuggerMenuNext"),PyDebugger::OnNext)
////    EVT_MENU(XRCID("idPyDebuggerMenuStep"),PyDebugger::OnStep)
////    EVT_MENU(XRCID("idPyDebuggerMenuStop"),PyDebugger::OnStop)
    EVT_MENU(ID_LangMenu_DebugSendCommand,PyDebugger::OnSendCommand)
    EVT_MENU(ID_LangMenu_ShowWatch,PyDebugger::OnViewWatch)
    EVT_MENU(ID_LangMenu_UpdateWatch,PyDebugger::OnUpdateWatch)
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
        wxString sfile=(*itr)->GetFilename();
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
{ //TODO: get this working
    wxString commands;
    wxString watchtext=m_WatchDlg->m_WatchText->GetValue();
    wxRegEx re,re1,re2,re3,re4;
    //Not sure regexes are really required, something simpler would also work...
    re1.Compile(_T("^ *([^\\t]*)(\\t.*)*"),wxRE_ADVANCED); // evaluate expression: syntax "expr = value" expr may contain == so regex must only match on single =
    re2.Compile(_T("^ *([^\\t]*)\\: *(\\t.*)*$"),wxRE_ADVANCED); // evaluate members: syntax "objname:"
    re3.Compile(_T("^ *[\\.\\*]"),wxRE_ADVANCED); // subordinates of object in the form ".varname\tvalue" and errors will be deleted
    re4.Compile(_T("^ *(\\#.*)"),wxRE_ADVANCED); // comment line syntax "# blah blah blah" no command processing

    //pull one line at a time and look for a regex match in each line
    while( watchtext.Len()>0 )
    {
        int watchtextpos=watchtext.Find(_T("\n"));
        wxString watchline;
        if(watchtextpos<0) // not found
        {
            watchline=watchtext;
            watchtext=_("");
        } else
        {
            watchline=(watchtextpos==0?_T(""):watchtext.Mid(0,watchtextpos));
            watchtext=watchtext.Mid(watchtextpos+1);
        }
        wxString expr, cmd;
        if(watchline.Len()==0)
            continue;
        if(re4.Matches(watchline))
        {
            expr=re4.GetMatch(watchline,1);
            expr.Replace(_T("\t"),_T("\001"));
            cmd=_T("pc ")+expr+_T("\n");
            commands+=cmd;
            continue;
        }
        if(re3.Matches(watchline))
            continue;
        if(re2.Matches(watchline)&&re2.GetMatchCount()>=2)
        {
            expr=re2.GetMatch(watchline,1);
            cmd=_T("pm1 ")+expr+_T("\n")+_T("pm2 ")+expr+_T("\n");
            commands+=cmd;
            continue;
        }
        if(re1.Matches(watchline)&&re1.GetMatchCount()>=2)
        {
            expr=re1.GetMatch(watchline,1);
            if(expr==_T(""))
                continue;
            cmd=_T("ps ")+expr+_T("\n");
            commands+=cmd;
            continue;
        }
    }
    return commands;
}

wxString PyDebugger::AssembleAliasCommands()
{
    wxString commands;
    //Print instance variables (usage "pi classInst")
    commands+=_T("alias pm1 print '%1: \\001',;print type(%1)\n");
    commands+=_T("alias pm2 for k in %1.__dict__.keys(): print '...',k,'\\001',type(k),'\\001',%1.__dict__[k],'\\002',\n");
    //Print variable name, type and value
    commands+=_T("alias ps print '%1 \\001',;print type(%1), '\\001', %1\n");
    //Print comment
    commands+=_T("alias pc print '%1'\n");
    //NB: \001 is the separator character used when parsing in OnTimer
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
        char buf0[10001];;
        for(int i=0;i<10001;i++)
            buf0[i]=0;
        m_istream->Read(buf0,10000);
        wxString m_latest=wxString::FromAscii(buf0);
        m_outbuf+=m_latest;

        //Disentangle debug output from program output - debug output is wrapped in <&CB_PDB  CB_PDB&>
        //TODO: Program could hang if debug output is incorrectly parsed
        m_outdebugbuf=m_outbuf;
//        wxRegEx re_debugparse;
//        re_debugparse.Compile(_T("(.*)\\<\\&CB_PDB(.*)CB_PDB\\&\\>"),wxRE_ADVANCED);
//        while(re_debugparse.Matches(m_outbuf.Mid(m_bufpos)))
//        {
//            if(re_debugparse.GetMatchCount()<=2)
//                return;
//            wxString debugbuf_add;
//            debugbuf_add=re_debugparse.GetMatch(m_outbuf.Mid(m_bufpos),2);
//            m_outdebugbuf+=debugbuf_add;
//            wxString progbuf_add;
//            progbuf_add=re_debugparse.GetMatch(m_outbuf.Mid(m_bufpos),1);
//            m_outprogbuf+=progbuf_add;
//        }

        // Count the number of commands that have executed so far
        wxRegEx reprompt;
        reprompt.Compile(_T("(.*?)\\(Pdb\\)\\s+"),wxRE_ADVANCED); //the presence of a command prompt signals a command has finished executing
        //m_debugbufpos=0; //Should not need to start at 0 each time.
        while(reprompt.Matches(m_outdebugbuf.Mid(m_debugbufpos)))
        {
            size_t start,len;
            m_DebugCommandCount--;
            PythonCmdDispatchData cmd;
            if(!m_DispatchedCommands.empty())
            {
                cmd=m_DispatchedCommands.front();
                m_DispatchedCommands.pop_front();
            }
            wxString logout=reprompt.GetMatch(m_outdebugbuf.Mid(m_debugbufpos),0);
            wxString exprresult=reprompt.GetMatch(m_outdebugbuf.Mid(m_debugbufpos),1);
            if(cmd.type==DBGCMDTYPE_WATCHEXPRESSION)
            {
                // exprresult;
                if(reprompt.GetMatchCount()>1)
                {
                    exprresult.Replace(_T("\t"),_T("\\t"),true);
                    exprresult.Replace(_T("\n"),_T("\\n"),true);
                    exprresult.Replace(_T(" \001 "),_T("\t"),true);
                    exprresult.Replace(_T(" \002"),_T("\n"),true);
                    if(exprresult.Mid(exprresult.Len()-2,2)==_T("\\n"))
                        exprresult=exprresult.Mid(0,exprresult.Len()-2);
                    m_watchstr+=exprresult+_T("\n");
//                    m_WatchDlg->UpdateVariable(exprresult); //TODO: implement this
                }
            }
            if(cmd.type==DBGCMDTYPE_FLOWCONTROL)
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
            }
            if(cmd.type==DBGCMDTYPE_CALLSTACK)
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
//                    unsigned long line;
//                    re.GetMatch(exprresult,2).ToULong(&line);
                    wxString Module=re.GetMatch(matchstr,3);
                    wxString Code=re.GetMatch(matchstr,4);
                    bool active=(re.GetMatch(matchstr,0).Left(1)==_T(">"));
                    m_changeposition=true;
                    cbStackFrame f;
                    f.SetFile(file,line);
                    f.SetNumber(n);
                    f.SetSymbol(Module);
                    f.SetAddress(0);
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
            }

            if(debugoutputmode) //in debug output mode, dump everything to the debugger log
            {
                m_DebugLog->Append(cmd.cmdtext);
                m_DebugLog->Append(logout);
            } else
            {
                if(cmd.type==DBGCMDTYPE_USERCOMMAND)
                {
                    m_DebugLog->Append(cmd.cmdtext);
                    m_DebugLog->Append(logout);
                }
            }
            reprompt.GetMatch(&start,&len);
            m_debugbufpos+=start+len;

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
            if(m_watchstr!=_T(""))
                m_WatchDlg->m_WatchText->SetValue(m_watchstr);
            m_watchstr=_T("");
        }
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
    m_watchstr=_T("");
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
    wxString commandln=wxFileName(m_DefaultInterpreter).GetShortPath()+m_DefaultDebugCmdLine+target;
//    cbMessageBox(commandln);
    #ifdef EXPERIMENTAL_PYTHON_DEBUG
//    LogMessage(wxString::Format(_("Launching '%s': %s (in %s)"), consolename.c_str(), commandstr.c_str(), workingdir.c_str()));
//    InterpretedLangs* plugin = Manager::Get()->GetPluginManager()->LoadPlugin(_T("InterpretedLangs"));
//    m_ilplugin->m_shellmgr->LaunchProcess(commandln,_(T"PyDEBUG"),0);
//    m_ilplugin->ShowConsole();
//    ilShellTermEvent e;
//    ProcessEvent(e);
    #else
    if(!wxExecute(commandln,wxEXEC_ASYNC,m_pp))
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


void PyDebugger::Stop()
{
    if(m_DebuggerActive)
    {
        if(m_TimerPollDebugger.IsRunning()) //TODO: there is a risk this will kill the process...
        {
            long pid = m_pp->GetPid();
            if(wxProcess::Exists(pid))
                wxProcess::Kill(pid,wxSIGKILL);
//            char cmd=3; // send a ctrl-c message
//            m_ostream->Write(&cmd,1);
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
    DispatchCommands(wxString::Format(_T("until %i\n"),line),DBGCMDTYPE_FLOWCONTROL,false);
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


cbBreakpoint* PyDebugger::GetBreakpoint(int index)
{
    return m_bplist[index].get();
}

const cbBreakpoint* PyDebugger::GetBreakpoint(int index) const
{
    return m_bplist[index].get();
}


cbBreakpoint* PyDebugger::AddBreakpoint(const wxString& file, int line)
{
    if(!IsPythonFile(file))
        return NULL;
    for (BPList::iterator itr=m_bplist.begin(); itr!=m_bplist.end(); ++itr)
    {
        if((*itr)->GetFilename()==file && (*itr)->GetLine()==line)
            return NULL;
    }
    Pointer p(new cbBreakpoint(file,line));
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
    return p.get();
}

void PyDebugger::DeleteBreakpoint(cbBreakpoint *bp)
{
    if(!IsPythonFile(bp->GetFilename()))
        return;
    wxString sfile=bp->GetFilename();
    int line = bp->GetLine();

    for (size_t i=0;i<m_bplist.size();++i)
    {
        if(m_bplist[i].get()!=bp)
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
        cbBreakpoint *bp=m_bplist[i].get();
        wxString sfile=bp->GetFilename();
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


int PyDebugger::GetStackFrameCount() const
{
    return m_stackinfo.frames.size();
}

const cbStackFrame& PyDebugger::GetStackFrame(int index) const
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
    wxMessageBox(_("Debug Terminated"));
    m_DebugLog->Append(_T("\n*** SESSION TERMINATED ***"));
    ClearActiveMarkFromAllEditors();
    m_DebuggerActive=false;
    m_TimerPollDebugger.Stop();
    delete m_pp;
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


void PyDebugger::OnViewWatch(wxCommandEvent& event)
{
    // This toggles display of watch
    CodeBlocksDockEvent evt(event.IsChecked() ? cbEVT_SHOW_DOCK_WINDOW : cbEVT_HIDE_DOCK_WINDOW);
    evt.pWindow = m_WatchDlg;
    Manager::Get()->ProcessEvent(evt);

    //TODO: Any required watch processing before watch is shown
//    if (event.IsChecked())
//        DoWatches();
}


void PyDebugger::OnUpdateWatch(wxCommandEvent& event)
{
    if(m_DebuggerActive)
    {
        wxString wcommands=AssembleWatchCommands();
        DispatchCommands(wcommands,DBGCMDTYPE_WATCHEXPRESSION,true);
    }
}



// constructor
PyDebugger::PyDebugger()
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
    m_WatchDlg=NULL;

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
//    m_HasDebugLog = Manager::Get()->GetConfigManager(_T("debugger"))->ReadBool(_T("debug_log"), false);
//    MessageManager* msgMan = Manager::Get()->GetMessageManager();
    m_DebugLog = new TextCtrlLogger(true);
//    m_DebugLogPageIndex = msgMan->AddLog(m_DebugLog, _("PyDebugger"));
    m_WatchDlg = new DebuggerWatch(Manager::Get()->GetAppWindow(), this);

    CodeBlocksLogEvent evtlog(cbEVT_ADD_LOG_WINDOW,m_DebugLog, _("PyDebugger"));
    Manager::Get()->ProcessEvent(evtlog);
    CodeBlocksDockEvent evt(cbEVT_ADD_DOCK_WINDOW);
    evt.name = _T("Python Watch");
    evt.title = _T("Python Watch");
    evt.pWindow = m_WatchDlg;
    evt.dockSide = CodeBlocksDockEvent::dsFloating;
    evt.desiredSize.Set(150, 250);
    evt.floatingSize.Set(150, 250);
    evt.minimumSize.Set(150, 150);
    Manager::Get()->ProcessEvent(evt);
    DebuggerManager &dbg_manager = *Manager::Get()->GetDebuggerManager();
    dbg_manager.RegisterDebugger(this, _T("python"));
    // set log image
    // wxString prefix = ConfigManager::GetDataFolder() + _T("/images/");
    //bmp = cbLoadBitmap(prefix + _T("contents_16x16.png"), wxBITMAP_TYPE_PNG);
    //Manager::Get()->GetMessageManager()->SetLogImage(m_pDbgLog, bmp);

}

void PyDebugger::OnReleaseReal(bool appShutDown)
{
	// do de-initialization for your plugin
	// if appShutDown is false, the plugin is unloaded because Code::Blocks is being shut down,
	// which means you must not use any of the SDK Managers
	// NOTE: after this function, the inherited member variable
	// m_IsAttached will be FALSE...

	//TODO: Handle destruction of any running python processes? (Maybe not necessary...)
	//TODO: Destroy any other objects that aren't cleaned up by the framework?

    if (m_WatchDlg)
    {
        CodeBlocksDockEvent evt(cbEVT_REMOVE_DOCK_WINDOW);
        evt.pWindow = m_WatchDlg;
        Manager::Get()->ProcessEvent(evt);
        m_WatchDlg->Destroy();
    }
    m_WatchDlg = 0L;

    CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW,m_DebugLog);
    Manager::Get()->ProcessEvent(evt);
    m_DebugLog = 0L;

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
    LangMenu->Append(ID_LangMenu_ShowWatch,_T("Toggle &Watch"),_T(""),wxITEM_CHECK);
    LangMenu->Append(ID_LangMenu_UpdateWatch,_T("&Update Watch"),_T(""));
}


void PyDebugger::UpdateMenu()
{
}


void PyDebugger::BuildMenu(wxMenuBar* menuBar)
{
    //TODO: This should all be deleted once native implementations are in place
	//The application is offering its menubar for your plugin,
	//to add any menu items you want...
	//Append any items you need in the menu...
	//NOTE: Be careful in here... The application's menubar is at your disposal.
	LangMenu=new wxMenu;
	CreateMenu();
	int pos = menuBar->FindMenu(_T("Plugins"));
	if(pos!=wxNOT_FOUND)
        menuBar->Insert(pos, LangMenu, _T("P&yDebug"));
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

