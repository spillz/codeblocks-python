#include "py_embedder.h"
#include <wx/listimpl.cpp>
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(PyInstanceCollection);
WX_DEFINE_LIST(PyJobQueue);

using namespace std;

int ID_PY_PROC=wxNewId();



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// class PyJob
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


//IMPLEMENT_DYNAMIC_CLASS(PyJob, wxThread)

PyJob::PyJob(PyInstance *pyinst, wxWindow *p, int id, bool selfdestroy):
wxThread(wxTHREAD_JOINABLE)
{
    parent=p;
    this->id=id;
    finished=false;
    started=false;
    killonexit=selfdestroy;
}

void PyJob::Abort()
{
    if(this->IsAlive())
        this->Kill();
}

PyJob::~PyJob()
{
    Abort();
}

//void *PyJob::Entry()
//{
//    wxMessageBox(_("entered thread"));
//    PyNotifyUIEvent pe(id,pyinst,parent,PYSTATE_STARTEDJOB);
//    ::wxPostEvent(pyinst,pe);
//    if((*this)())
//        pe.SetState(PYSTATE_FINISHEDJOB);
//    else
//        pe.SetState(PYSTATE_ABORTEDJOB);
//    ::wxPostEvent(pyinst,pe);
//    Exit();
//    return NULL;
//}

void *PyJob::Entry()
{
//    wxMutexGuiEnter();
    wxCommandEvent pe(wxEVT_PY_NOTIFY_UI_STARTED,0);
    parent->AddPendingEvent(pe);
//    wxMutexGuiLeave();
    if((*this)())
    {
        wxCommandEvent pe(wxEVT_PY_NOTIFY_UI_FINISHED,0);
        pyinst->AddPendingEvent(pe);
    }
    else
    {
        wxCommandEvent pe(wxEVT_PY_NOTIFY_UI_ABORTED,0);
        pyinst->AddPendingEvent(pe);
    }
    Exit();
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// class PyInstance
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//IMPLEMENT_DYNAMIC_CLASS(PyInstance, wxEvtHandler)

BEGIN_EVENT_TABLE(PyInstance, wxEvtHandler)
//    EVT_PY_NOTIFY_UI(PyInstance::OnJobNotify)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_FINISHED, PyInstance::OnJobNotify)
    EVT_COMMAND(0, wxEVT_PY_NOTIFY_UI_ABORTED, PyInstance::OnJobNotify)
END_EVENT_TABLE()

//PyInstance::PyInstance()
//{
//    // create python process
//    // create xmlrpc client
//    // get & store methods from the xmlrpc server
//    // set running state flag
//}

PyInstance::~PyInstance()
{
    // check if any running jobs, kill them
    // delete the client
    delete m_client;
    // kill python process if still running
}

PyInstance::PyInstance(const wxString &processcmd, const wxString &hostaddress, int port)
{
  m_port=port;
  m_proc=NULL;
  m_proc_dead=true;
  m_hostaddress=hostaddress;
  m_paused=false;
  m_proc_killlevel=0;
  // Launch process
  LaunchProcess(processcmd); //TODO: The command for the interpreter process should come from the manager (and be stored in a config file)
  // Setup XMLRPC client and use introspection API to look up the supported methods
  m_client = new XmlRpc::XmlRpcClient(hostaddress.char_str(), port);
  XmlRpc::XmlRpcValue noArgs, result;
  if (m_client->execute("system.listMethods", noArgs, result))
    std::cout << "\nMethods:\n " << result << "\n\n";
  else
    std::cout << "Error calling 'listMethods'\n\n";
}

long PyInstance::LaunchProcess(const wxString &processcmd)
{
    if(!m_proc_dead)
        return -1;
    if(m_proc) //this should never happen
        m_proc->Detach(); //self cleanup
    m_proc=new wxProcess(this,ID_PY_PROC);
    m_proc->Redirect();
    m_proc_id=wxExecute(processcmd,wxEXEC_ASYNC,m_proc);
    if(m_proc_id>0)
    {
        m_proc_dead=false;
        m_proc_killlevel=0;
    }
    return m_proc_id;
}

PyInstance::PyInstance(const PyInstance &copy)
{
    m_paused=copy.m_paused;
    m_queue=copy.m_queue;
    m_hostaddress=copy.m_hostaddress;
    m_port=copy.m_port;
    m_proc=copy.m_proc;
    m_proc_id=copy.m_proc_id;
    m_proc_killlevel=copy.m_proc_killlevel;
    m_proc_dead=copy.m_proc_dead;
}

bool PyInstance::Exec(const wxString &method, XmlRpc::XmlRpcValue &inarg, XmlRpc::XmlRpcValue &result)
{
    wxMutexLocker ml(exec_mutex);
    return m_client->execute(method.mb_str(), inarg, result);
}

void PyInstance::KillProcess(bool force)
{
    if(m_proc_dead)
        return;
    long pid=GetPid();
    if(m_proc_killlevel==0)
    {
        m_proc_killlevel=1;
        if(wxProcess::Exists(pid))
            wxProcess::Kill(pid,wxSIGTERM);
        return;
    }
    if(m_proc_killlevel==1)
    {
        if(wxProcess::Exists(pid))
        {
//                cbMessageBox(_T("Forcing..."));
            wxProcess::Kill(pid,wxSIGKILL);
        }
    }
}

bool PyInstance::AddJob(PyJob *job)
{
    if(job->Create()!=wxTHREAD_NO_ERROR)
        return false;
    m_queue.Append(job);
    if(m_paused)
        return true;
    PyJob *newjob=m_queue.GetFirst()->GetData();
    if(!newjob->finished && !newjob->started)
    {
        newjob->started=true;
        newjob->Run();
    }
    return true;
}

void PyInstance::OnJobNotify(wxCommandEvent &event)
{
//    if(event.jobstate==PYSTATE_ABORTEDJOB||event.jobstate==PYSTATE_FINISHEDJOB)
//    {
    PyJob *job=m_queue.GetFirst()->GetData();
    job->Wait();
    if(job->killonexit)
    {
        delete job;
        job=NULL;
    }
    m_queue.DeleteNode(m_queue.GetFirst());
//    }
//    if(event.parent)
//        ::wxPostEvent(event.parent,event);
    if(m_paused)
        return;
    wxPyJobQueueNode *node=m_queue.GetFirst();
    if(!node)
        return;
    PyJob *newjob=node->GetData();
    if(!newjob->finished && !newjob->started)
    {
        newjob->started=true;
        newjob->Run();
    }
}

void PyInstance::EvalString(char *str, bool wait)
{
//    PyRun_SimpleString(str);
}

PyMgr::PyMgr()
{
}

PyMgr::~PyMgr()
{
    m_Interpreters.Empty();
}

PyInstance *PyMgr::LaunchInterpreter()
{
    PyInstance *p=new PyInstance(_("python interp.py"),_("localhost"),8000);
    if(p)
        m_Interpreters.Add(p);
    return p;
}

PyMgr &PyMgr::Get()
{
    if (theSingleInstance.get() == 0)
      theSingleInstance.reset(new PyMgr);
    return *theSingleInstance;
}

std::auto_ptr<PyMgr> PyMgr::theSingleInstance;

