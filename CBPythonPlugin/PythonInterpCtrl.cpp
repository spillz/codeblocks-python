#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/regex.h>
#include "PythonInterpCtrl.h"
#include <globals.h>

DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_CODEOK, -1)
DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI_CODEOK )

DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_INPUT, -1)
DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI_INPUT )

////////////////////////////////////// PythonInterpJob /////////////////////////////////////////////

bool PyInterpJob::operator()()
{
    // talk to m_client
//    wxMessageBox(_("entered operator..."));
    int status=0;
    if(!pctl->RunCode(m_code,status))
        return false; //TODO: maybe retry before failing out completely
    if(status>=0)
    {
        wxCommandEvent pe(wxEVT_PY_NOTIFY_UI_CODEOK,0);
        ::wxPostEvent(parent,pe);
    }
    wxCommandEvent pe(wxEVT_PY_NOTIFY_UI_NOTIFY,0);
    ::wxPostEvent(parent,pe);
    bool break_called=false;
    while(status>0)
    {
        m_break_mutex.Lock();
        if(m_break_job && !break_called)
        {
            m_break_mutex.Unlock();
            if(pctl->BreakCode())
                break_called=true;
        } else
            m_break_mutex.Unlock();
        bool line_input_request;
        if(!pctl->Continue(status,line_input_request))
            return false; //TODO: maybe retry before failing out completely
//        PyNotifyUIEvent pe(id,pyinst,parent,PYSTATE_NOTIFY);
        ::wxPostEvent(parent,pe);
        if(line_input_request)
        {
            pctl->m_input_mutex.Lock();
            wxCommandEvent ie(wxEVT_PY_NOTIFY_UI_INPUT,0);
            ::wxPostEvent(parent,ie);
            pctl->m_input_cond->Wait();
            pctl->m_input_mutex.Unlock();
        }
        // sleep for some period of time
        else
            if(status>0)
                Sleep(50); //TODO: Make the sleep period user-defined
    }
    if(status==0)
    {
        wxCommandEvent pef(wxEVT_PY_NOTIFY_UI_FINISHED,0);
        ::wxPostEvent(parent,pef);
    } else
    {
        wxCommandEvent pef(wxEVT_PY_NOTIFY_UI_ABORTED,0);
        ::wxPostEvent(parent,pef);
    }
    return true;
}

void PyInterpJob::Break()
{
    m_break_mutex.Lock();
    m_break_job=true;
    m_break_mutex.Unlock();
}

////////////////////////////////////// PythonIOCtrl //////////////////////////////////////////////

BEGIN_EVENT_TABLE(PythonIOCtrl, wxTextCtrl)
    EVT_CHAR(PythonIOCtrl::OnUserInput)
//    EVT_TEXT(wxID_ANY, PythonIOCtrl::OnTextChange)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_INPUT, PythonIOCtrl::OnLineInputRequest)
END_EVENT_TABLE()

void PythonIOCtrl::OnUserInput(wxKeyEvent& ke)
{
    if(ke.GetModifiers()==wxMOD_CONTROL)
    {
//        wxMessageBox(_T("control pressed"));
//        wxMessageBox(wxString::Format(_("Key: %i"),ke.GetKeyCode()));
//        if(ke.GetKeyCode()==4)
//        {
//            m_pyctrl->DispatchCode(GetValue()); // would need to retrieve the code control's value
////            if(m_pyctrl->DispatchCode(GetValue()))
////                ChangeValue(_T(""));
//            return;
//        }
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
            if(!ke.ShiftDown())
            {
                if(m_line_entry_mode)
                {
                    m_line_entry_mode=false;
                    this->SetEditable(false);
                    wxString line;
                    if(m_line_entry_point<GetLastPosition())
                        line=GetRange(m_line_entry_point,GetLastPosition());
                    line.Replace(_T("\n"),_T("")); //will cause major problems if there is more than one line feed returned here, so we remove them (TODO: fix on server side?? server should treat buffered lines as new input without errors)
                    line.Append(_T("\n"));
                    m_pyctrl->stdin_append(line);
                    m_pyctrl->m_input_cond->Signal();
                    m_pyctrl->m_input_mutex.Unlock();
                    return;
                }
            }
            return;
        }
    }
    ke.Skip();
}

void PythonIOCtrl::OnTextChange(wxCommandEvent &e)
{
//    if(m_line_entry_mode && this->GetInsertionPoint()<m_line_entry_point)
//        return;
    e.Skip(true);
}

void PythonIOCtrl::OnLineInputRequest(wxCommandEvent &e)
{
    if(!m_line_entry_mode)
    {
        m_pyctrl->m_input_mutex.Lock();
        m_line_entry_mode=true;
        m_line_entry_point=this->GetLastPosition();
        this->SetSelection(m_line_entry_point,m_line_entry_point);
        this->SetEditable(true);
    }
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
            if(!ke.ShiftDown())
            {
                m_pyctrl->DispatchCode(GetValue());
                return;
            } else
            {
                ke.Skip();
            }
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
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_CODEOK, PythonInterpCtrl::OnPyCode)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_INPUT, PythonInterpCtrl::OnLineInputRequest)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_STARTED, PythonInterpCtrl::OnPyNotify)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_FINISHED, PythonInterpCtrl::OnPyJobDone)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_ABORTED, PythonInterpCtrl::OnPyJobAbort)
    EVT_COMMAND(0, wxEVT_PY_PROC_END, PythonInterpCtrl::OnEndProcess)

    EVT_SIZE    (PythonInterpCtrl::OnSize)
END_EVENT_TABLE()

PythonInterpCtrl::PythonInterpCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr) : ShellCtrlBase(parent, id, name, shellmgr)
{
    m_killlevel=0;
    m_port=0;
    m_pyinterp=NULL;
    m_input_cond=new wxCondition(m_input_mutex);
    m_sw=new wxSplitterWindow(this, wxID_ANY);
    m_ioctrl=new PythonIOCtrl(m_sw, this);//new wxTextCtrl(m_sw, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_MULTILINE|wxTE_READONLY|wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxEXPAND);
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
    wxString cmd=_T("cmd /c interp.py ")+wxString::Format(_T(" %i"),m_port); //TODO: this could have process destruction issues on earlier version of wxWidgets (kills cmd, but not python)
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
}

bool PythonInterpCtrl::DispatchCode(const wxString &code)
{
    if(m_pyinterp->IsJobRunning())
        return false;
    m_pyinterp->AddJob(new PyInterpJob(wxString(code.c_str()),m_pyinterp,this,m_ioctrl));
    m_code=code;
    return true;
}

void PythonInterpCtrl::OnPyNotify(wxCommandEvent& event)
{
    if(m_ioctrl->m_line_entry_mode)
        return;
    m_ioctrl->AppendText(stdout_retrieve());
    m_ioctrl->AppendText(stderr_retrieve());
}

void PythonInterpCtrl::OnLineInputRequest(wxCommandEvent& event)
{
    m_ioctrl->ProcessEvent(event);
}


void PythonInterpCtrl::OnPyCode(wxCommandEvent& event)
{
    wxTextAttr oldta=m_ioctrl->GetDefaultStyle();
    wxTextAttr ta=oldta;
    wxFont f=ta.GetFont();
    f.SetWeight(wxFONTWEIGHT_BOLD);
    ta.SetFont(f);
    m_ioctrl->SetDefaultStyle(ta);
    m_ioctrl->AppendText(m_code);
    m_ioctrl->SetDefaultStyle(oldta);
    m_ioctrl->AppendText(_T("\n"));
}

void PythonInterpCtrl::OnPyJobDone(wxCommandEvent& event)
{
    m_codectrl->ChangeValue(_T(""));
    m_code=_T("");
}

void PythonInterpCtrl::OnPyJobAbort(wxCommandEvent& event)
{
    m_codectrl->AppendText(_T("\n"));
    m_code=_T("");
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

bool PythonInterpCtrl::RunCode(const wxString &codestr, int &status)
{
    XmlRpc::XmlRpcValue args, result;
    args[0]=codestr.utf8_str();
    args[1]=stdin_retrieve().utf8_str();
    if(m_pyinterp->Exec(_("run_code"), args, result))
    {
        status=result[0];
        std::string r1=result[1];
        stdout_append(wxString::FromUTF8(r1.c_str()));
        std::string r2=result[2];
        stderr_append(wxString::FromUTF8(r2.c_str()));
        return true;
    }
    status=-4;
    return false;
}

bool PythonInterpCtrl::Continue(int &status, bool &line_input_request)
{
    XmlRpc::XmlRpcValue args, result;
    args[0]=stdin_retrieve().utf8_str();
    if(m_pyinterp->Exec(_("cont"), args, result))
    {
        status=result[0];
        std::string r1=result[1];
        stdout_append(wxString::FromUTF8(r1.c_str()));
        std::string r2=result[2];
        stderr_append(wxString::FromUTF8(r2.c_str()));
        line_input_request=result[3];
        return true;
    }
    status=-4;
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

