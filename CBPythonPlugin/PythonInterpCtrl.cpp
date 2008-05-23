#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/regex.h>
#include "PythonInterpCtrl.h"
#include <globals.h>

////////////////////////////////////// PythonInterpCtrl /////////////////////////////////////////////
int ID_PROC=wxNewId();


BEGIN_EVENT_TABLE(PythonInterpCtrl, wxPanel)
    EVT_CHAR(PythonInterpCtrl::OnUserInput)
//    EVT_END_PROCESS(ID_PROC, PythonInterpCtrl::OnEndProcess)
//    EVT_LEFT_DCLICK(PythonInterpCtrl::OnDClick)
    EVT_SIZE    (PythonInterpCtrl::OnSize)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(PythonInterpCtrl, wxPanel)


PythonInterpCtrl::PythonInterpCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr) : ShellCtrlBase(parent, id, name, shellmgr)
{
    m_pyinterp=NULL;
    m_textctrl=new wxTextCtrl(this, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_MULTILINE|wxTE_READONLY|wxTE_PROCESS_ENTER|wxEXPAND);
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    bs->Add(m_textctrl, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);
    SetSizer(bs);
}


//void PythonInterpCtrl::OnEndProcess(wxProcessEvent &event)
//{
//    m_exitcode=event.GetExitCode();
//    SyncOutput(-1); //read any left over output TODO: while loop to handle extremely large amount of output
//    m_dead=true;
//    delete m_proc;
//    m_proc=NULL;
//    m_killlevel=0;
//    if(m_shellmgr)
//        m_shellmgr->OnShellTerminate(this);
//}

long PythonInterpCtrl::LaunchProcess(const wxString &processcmd, const wxArrayString &options) // bool ParseLinks, bool LinkClicks, const wxString &LinkRegex
{
    if(!m_dead || m_pyinterp)
        return -1;
    m_pyinterp=new PyInstance(_T("localhost"),8000);
    return 1;
}

void PythonInterpCtrl::KillProcess()
{
    if(m_dead)
        return;
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
    m_textctrl->SetSize(event.GetSize());
}

void PythonInterpCtrl::OnUserInput(wxKeyEvent& ke)
{
    if(m_dead)
    {
        ke.Skip();
        return;
    }
    char* kc1=new char[2];
    kc1[0]=ke.GetKeyCode()%256;
    kc1[1]=0;
    if(kc1[0]=='\r')
    {
        kc1[0]='\n';
    }
    wxChar kc2=ke.GetUnicodeKey();
    wxString buf(kc2);
    //TODO: queue the key press for dispatch to the interpreter
    m_textctrl->AppendText(kc2);
    m_textctrl->SetInsertionPointEnd();
}
