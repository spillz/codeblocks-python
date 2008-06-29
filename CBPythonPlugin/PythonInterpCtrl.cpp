#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/regex.h>
#include "PythonInterpCtrl.h"
#include <globals.h>

//DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI )


////////////////////////////////////// PythonInterpJob /////////////////////////////////////////////

bool PyInterpJob::operator()()
{
    // talk to m_client
//    wxMessageBox(_("entered operator..."));
    bool unfinished=false;
    if(!pctl->RunCode(m_code,unfinished))
        return false; //TODO: maybe retry before failing out completely
    wxCommandEvent pe(wxEVT_PY_NOTIFY_UI_NOTIFY,0);
    ::wxPostEvent(parent,pe);
    bool break_called=false;
    while(unfinished)
    {
        m_break_mutex.Lock();
        if(m_break_job && !break_called)
        {
            m_break_mutex.Unlock();
            if(pctl->BreakCode())
                break_called=true;
        } else
            m_break_mutex.Unlock();
        if(!pctl->Continue(unfinished))
            return false; //TODO: maybe retry before failing out completely
//        PyNotifyUIEvent pe(id,pyinst,parent,PYSTATE_NOTIFY);
        ::wxPostEvent(parent,pe);
        // sleep for some period of time
        if(unfinished)
            Sleep(50); //TODO: Make the sleep period user-defined
    }
    wxCommandEvent pef(wxEVT_PY_NOTIFY_UI_FINISHED,0);
    ::wxPostEvent(parent,pef);
    return true;
}


void PyInterpJob::Break()
{
    m_break_mutex.Lock();
    m_break_job=true;
    m_break_mutex.Unlock();
}

////////////////////////////////////// PythonCodeCtrl //////////////////////////////////////////////

BEGIN_EVENT_TABLE(PythonCodeCtrl, wxTextCtrl)
    EVT_CHAR(PythonCodeCtrl::OnUserInput)
END_EVENT_TABLE()

void PythonCodeCtrl::OnUserInput(wxKeyEvent& ke)
{
    if(ke.GetModifiers()==wxMOD_CONTROL)
    {
//        wxMessageBox(_T("control pressed"));
//        wxMessageBox(wxString::Format(_("Key: %i"),ke.GetKeyCode()));
        if(ke.GetKeyCode()==4)
        {
            m_pyctrl->DispatchCode(GetValue());
//            if(m_pyctrl->DispatchCode(GetValue()))
//                ChangeValue(_T(""));
            return;
        }
        if(ke.GetKeyCode()==5)
        {
            m_pyctrl->BreakCode();
            return;
        }
    }
    if(!ke.HasModifiers())
    {
        if(ke.GetKeyCode()==WXK_RETURN)
        {
            m_pyctrl->DispatchCode(GetValue());
//            if(m_pyctrl->DispatchCode(GetValue()))
//                ChangeValue(_T(""));
            return;
        }
    }
    ke.Skip();
}



////////////////////////////////////// PythonInterpCtrl /////////////////////////////////////////////
int ID_PROC=wxNewId();


IMPLEMENT_DYNAMIC_CLASS(PythonInterpCtrl, wxPanel)

BEGIN_EVENT_TABLE(PythonInterpCtrl, wxPanel)
    EVT_CHAR(PythonInterpCtrl::OnUserInput)
//    EVT_PY_NOTIFY_UI(PythonInterpCtrl::OnPyNotify)
//    EVT_END_PROCESS(ID_PROC, PythonInterpCtrl::OnEndProcess)
//    EVT_LEFT_DCLICK(PythonInterpCtrl::OnDClick)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_NOTIFY, PythonInterpCtrl::OnPyNotify)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_STARTED, PythonInterpCtrl::OnPyNotify)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_FINISHED, PythonInterpCtrl::OnPyJobDone)
    EVT_COMMAND(0, wxEVT_PY_PROC_END, PythonInterpCtrl::OnEndProcess)
    EVT_SIZE    (PythonInterpCtrl::OnSize)
END_EVENT_TABLE()



PythonInterpCtrl::PythonInterpCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr) : ShellCtrlBase(parent, id, name, shellmgr)
{
    m_killlevel=0;
    m_port=0;
    m_pyinterp=NULL;
    m_sw=new wxSplitterWindow(this, wxID_ANY);
    m_ioctrl=new wxTextCtrl(m_sw, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_MULTILINE|wxTE_READONLY|wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxEXPAND);
    m_codectrl=new PythonCodeCtrl(m_sw, this);
    m_codectrl->AppendText(_("print 'Python'"));
    int sash_pos=parent->GetClientSize().GetHeight()/5;
    if(sash_pos<20)
        sash_pos=0;
    m_sw->SplitHorizontally(m_codectrl, m_ioctrl, sash_pos);
    m_sw->SetMinimumPaneSize(20);
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    bs->Add(m_sw, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);
    SetSizer(bs);
}

PortAllocator PythonInterpCtrl::m_portalloc;

long PythonInterpCtrl::LaunchProcess(const wxString &processcmd, const wxArrayString &options) // bool ParseLinks, bool LinkClicks, const wxString &LinkRegex
{
    if(!IsDead())
        return -1;
    //wxMessageBox(m_portalloc.GetPorts());
    m_port=m_portalloc.RequestPort();
    if(m_port<0)
        return -1;
    //TODO: get the command and working dir from the
#ifdef __WXMSW__
    wxString cmd=_T("interp.py ")+wxString::Format(_T(" %i"),m_port);
    wxString python=_T("\\python");
    wxString interp=_T("\\interp.py");
#else
    wxString cmd=_T("python interp.py ")+wxString::Format(_T(" %i"),m_port);
    wxString python=_T("/python");
    wxString interp=_T("/interp.py");
#endif
    wxString gpath = ConfigManager::GetDataFolder(true)+python;
    wxString lpath = ConfigManager::GetDataFolder(false)+python;
    bool global=false,local=false;
    if(wxFileName::FileExists(gpath+interp))
    {
        wxSetWorkingDirectory(gpath);
        global=true;
    }
    if(wxFileName::FileExists(lpath+interp))
    {
        wxSetWorkingDirectory(lpath);
        local=true;
    }
    if(!global&&!local) //No interpreter script found, return failure.
        return -2; //TODO: Return meaningful messages (or at least use the codeblocks logger)
    m_pyinterp=new PyInstance(cmd,_T("localhost"),m_port,this);
    //TODO: Perform any initial communication with the running python process...
    return 1;
}

void PythonInterpCtrl::KillProcess()
{
    if(IsDead())
        return;
    if(m_killlevel==0)
    {
        SendKill();
        m_killlevel=1;
        return;
    }
    if(m_killlevel==1)
    {
        m_pyinterp->KillProcess();
        return;
    }

//    if(m_killlevel==0) //some process will complete if we send EOF. TODO: make sending EOF a separate option
//    {
//        m_proc->CloseOutput();
//        m_killlevel=1;
//        return;
//    }
//    long pid=GetPid();
//    if(m_killlevel==0)
//    {
//        m_killlevel=1;
//        if(wxProcess::Exists(pid))
//            wxProcess::Kill(pid,wxSIGTERM);
//        return;
//    }
//    if(m_killlevel==1)
//    {
//        if(wxProcess::Exists(pid))
//        {
//            cbMessageBox(_T("Forcing..."));
//            wxProcess::Kill(pid,wxSIGKILL);
//        }
//    }
}

bool PythonInterpCtrl::DispatchCode(const wxString &code)
{
    //TODO: check to see if a job is already running
//    wxCommandEvent ce(wxEVT_PY_NOTIFY_UI_STARTED,0);
//    wxPostEvent(this,ce);
//    bool unfin;
//    if(this->RunCode(_T("print 'abc'"),unfin))
//    {
//        wxMessageBox(_T("ran code print 'abc'"));
//        if(unfin)
//            wxMessageBox(_T("not finito"));
//    } else
//        wxMessageBox(_T("failed to print 'abc'"));
    if(m_pyinterp->IsJobRunning())
        return false;
    m_pyinterp->AddJob(new PyInterpJob(wxString(code.c_str()),m_pyinterp,this,m_ioctrl));
    return true;
}


void PythonInterpCtrl::OnPyNotify(wxCommandEvent& event)
{
    m_ioctrl->AppendText(stdout_retrieve());
    m_ioctrl->AppendText(stderr_retrieve());
}

void PythonInterpCtrl::OnPyJobDone(wxCommandEvent& event)
{
    m_codectrl->ChangeValue(_T(""));
}


void PythonInterpCtrl::OnEndProcess(wxCommandEvent &ce)
{
    m_ioctrl->AppendText(stdout_retrieve());
    m_ioctrl->AppendText(stderr_retrieve());
    m_portalloc.ReleasePort(m_port);
    if(m_shellmgr)
        m_shellmgr->OnShellTerminate(this);
}

void PythonInterpCtrl::SyncOutput(int maxchars)
{
//    if(!m_proc)
//        return;
//    bool oneshot=true;
//    if(maxchars<=0)
//    {
//        maxchars=20000;
//        oneshot=false;
//    }
//    int lineno=m_textctrl->GetNumberOfLines()-1;
//    if(lineno<0)
//        lineno=0;
//    while(m_proc->IsInputAvailable())
//    {
//        char buf0[maxchars+1];
//        for(int i=0;i<maxchars+1;i++)
//            buf0[i]=0;
//        m_istream->Read(buf0,maxchars);
//        wxString m_latest=wxString::FromAscii(buf0);
//        m_textctrl->AppendText(m_latest);
//        if(oneshot)
//            break;
//    }
//    if(m_proc->IsErrorAvailable())
//    {
//        wxTextAttr ta(wxColour(255,0,0));
//        wxTextAttr oldta=m_textctrl->GetDefaultStyle();
//        m_textctrl->SetDefaultStyle(ta);
//        while(m_proc->IsErrorAvailable())
//        {
//            char buf0[maxchars+1];
//            for(int i=0;i<maxchars+1;i++)
//                buf0[i]=0;
//            m_estream->Read(buf0,maxchars);
//            wxString m_latest=wxString::FromAscii(buf0);
//            m_textctrl->AppendText(m_latest);
//            if(oneshot)
//                break;
//        }
//        m_textctrl->SetDefaultStyle(oldta);
//    }
//    if(m_parselinks)
//        ParseLinks(lineno,m_textctrl->GetNumberOfLines());
}

void PythonInterpCtrl::OnSize(wxSizeEvent& event)
{
    m_sw->SetSize(event.GetSize());
}

void PythonInterpCtrl::OnUserInput(wxKeyEvent& ke)
{
    if(!IsDead())
    {
        ke.Skip();
        return;
    }
//    if(ke.GetModifiers()==wxMOD_CONTROL && ke.GetKeyCode()==WXK_RETURN)
//    {
//        PyInterpJob m_job(m_codectrl->GetValue(),m_pyinterp,this);
//        m_pyinterp->AddJob(m_job);
//        m_codectrl->ChangeValue(_T(""));
//        wxMessageBox(_T("command dispatched"));
//    }
//    wxMessageBox(_T("key captured"));

//    char* kc1=new char[2];
//    kc1[0]=ke.GetKeyCode()%256;
//    kc1[1]=0;
//    if(kc1[0]=='\r')
//    {
//        kc1[0]='\n';
//    }
//    wxChar kc2=ke.GetUnicodeKey();
//    wxString buf(kc2);
//    //TODO: queue the key press for dispatch to the interpreter
//    m_ioctrl->AppendText(kc2);
//    m_ioctrl->SetInsertionPointEnd();
}

void PythonInterpCtrl::stdin_append(const wxString &data)
{ //asynchronously dispatch data to python interpreter's stdin
    wxMutexLocker ml(io_mutex);
    stdin_data+=data;
}

void PythonInterpCtrl::stdout_append(const wxString &data)
{ //asynchronously dispatch data to python interpreter's stdin
    wxMutexLocker ml(io_mutex);
    stdout_data+=data;
}

void PythonInterpCtrl::stderr_append(const wxString &data)
{ //asynchronously dispatch data to python interpreter's stdin
    wxMutexLocker ml(io_mutex);
    stderr_data+=data;
}

wxString PythonInterpCtrl::stdin_retrieve()
{
    wxMutexLocker ml(io_mutex);
    wxString s(stdin_data);
    stdin_data=_T("");
    return s;
}

wxString PythonInterpCtrl::stdout_retrieve()
{
    wxMutexLocker ml(io_mutex);
    wxString s(stdout_data);
    stdout_data=_T("");
    return s;
}

wxString PythonInterpCtrl::stderr_retrieve()
{
    wxMutexLocker ml(io_mutex);
    wxString s(stderr_data);
    stderr_data=_T("");
    return s;
}

bool PythonInterpCtrl::RunCode(const wxString &codestr, bool &unfinished)
{
    XmlRpc::XmlRpcValue args, result;
    args[0]=codestr.utf8_str();
    args[1]=stdin_retrieve().utf8_str();
    int returncode=0;
    if(m_pyinterp->Exec(_("run_code"), args, result))
    {
        returncode=result[0];
        unfinished=returncode>0;
        std::string r1=result[1];
        stdout_append(wxString::FromUTF8(r1.c_str()));
        std::string r2=result[2];
        stderr_append(wxString::FromUTF8(r2.c_str()));
        return true;
    }
    unfinished=false;
    return false;
}

bool PythonInterpCtrl::Continue(bool &unfinished)
{
    XmlRpc::XmlRpcValue args, result;
    args[0]=stdin_retrieve().utf8_str();
    int returncode=0;
    if(m_pyinterp->Exec(_("cont"), args, result))
    {
        returncode=result[0];
        unfinished=returncode>0;
        std::string r1=result[1];
        stdout_append(wxString::FromUTF8(r1.c_str()));
        std::string r2=result[2];
        stderr_append(wxString::FromUTF8(r2.c_str()));
        return true;
    }
    unfinished=false;
    return false;
}

bool PythonInterpCtrl::BreakCode()
{
    m_pyinterp->Break();
    return true;
    XmlRpc::XmlRpcValue args, result;
    if(m_pyinterp->Exec(_("break_code"), args, result))
    {
        //TODO: evaluate result -- if it not true, there was no code running
        wxMessageBox(_("break sent"));
        return true;
    }
    wxMessageBox(_("break not sent"));
    return false;
}

bool PythonInterpCtrl::SendKill()
{
    XmlRpc::XmlRpcValue args, result;
    if(m_pyinterp->Exec(_("end"), args, result))
    {
        return true;
    }
    return false;
}

