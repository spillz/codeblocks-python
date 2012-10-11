#include "asyncprocess.h"
#include <wx/txtstrm.h>


DEFINE_EVENT_TYPE(wxEVT_NOTIFY_UPDATE_TREE)
DEFINE_EVENT_TYPE(wxEVT_NOTIFY_EXEC_REQUEST)

int ID_EXEC_TIMER=wxNewId();

BEGIN_EVENT_TABLE(AsyncProcess, wxEvtHandler)
    EVT_TIMER(ID_EXEC_TIMER, AsyncProcess::OnExecTimer)
    EVT_END_PROCESS(wxID_ANY, AsyncProcess::OnExecTerminate)
END_EVENT_TABLE()

AsyncProcess::~AsyncProcess()
{
    if(m_exec_proc)
    {
        m_exec_timer->Stop();
        delete m_exec_timer;
        m_exec_proc->Detach();
    }
}


int AsyncProcess::Exec(const wxString &command)
{
    int exitcode=0;
    m_exec_output.Empty();
    m_exec_err.Empty();
    m_exec_proc=new wxProcess(this);
    m_exec_proc->Redirect();
    m_exec_proc_id=wxExecute(command,wxEXEC_ASYNC,m_exec_proc);
    if(m_exec_proc_id==0)
        return 1;
    m_exec_timer=new wxTimer(this,ID_EXEC_TIMER);
    m_exec_timer->Start(100,true);
    return exitcode;
}

void AsyncProcess::OnExecTerminate(wxProcessEvent &e)
{
    ReadStream(true);
    m_exec_timer->Stop();
    delete m_exec_timer;
    delete m_exec_proc;
    m_exec_proc=NULL;
    m_parent->AddPendingEvent(e);
}

void AsyncProcess::OnExecTimer(wxTimerEvent &e)
{
    if(m_exec_proc)
        ReadStream();
}

void AsyncProcess::ReadStream(bool all)
{
    m_exec_timer->Stop();
    m_exec_stream=m_exec_proc->GetInputStream();
    wxTextInputStream tis(*m_exec_stream);
    wxStopWatch sw;
    while(m_exec_proc->IsInputAvailable())
    {
        m_exec_output+=_("\n")+tis.ReadLine();
        if(!all && sw.Time()>30)
            break;
    }

    m_exec_errstream=m_exec_proc->GetErrorStream();
    wxTextInputStream tes(*m_exec_errstream);
    while(m_exec_proc->IsErrorAvailable())
    {
        m_exec_err+=_("\n")+tes.ReadLine();
        if(!all && sw.Time()>30)
            break;
    }

    if(!all)
    {
        m_exec_timer->Start(150,true);
    }
}


