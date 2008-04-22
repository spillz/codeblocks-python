#include "py_embedder.h"
#include <wx/listimpl.cpp>
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(PyInstanceCollection);
WX_DEFINE_LIST(PyJobQueue);

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// classes PyNotifyIntepreterEvent and PyNotifyUIEvent
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_INTERPRETER )

PyNotifyIntepreterEvent::PyNotifyIntepreterEvent(int id) : wxEvent(id, wxEVT_PY_NOTIFY_INTERPRETER)
{
}

DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI )

PyNotifyUIEvent::PyNotifyUIEvent(int id, PyInstance *inst, JobStates js) : wxEvent(id, wxEVT_PY_NOTIFY_UI)
{
    instance=inst;
    jobstate=js;
    job=NULL;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// class PyJob
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

PyJob::PyJob(PyInstance *pyinst, wxWindow *p, int id, bool selfdestroy):
wxThread(wxTHREAD_JOINABLE)
{
    parent=p;
    this->id=id;
    finished=false;
    started=false;
    killonexit=selfdestroy;
    // need to call a Python method to notify it of this thread using the API...
}

PyJob::~PyJob()
{
}

void *PyJob::Entry()
{
    PyNotifyUIEvent pe(id,pyinst,PYSTATE_STARTEDJOB);
    ::wxPostEvent(pyinst,pe);
    if((*this)())
        pe.SetState(PYSTATE_FINISHEDJOB);
    else
        pe.SetState(PYSTATE_ABORTEDJOB);
    ::wxPostEvent(pyinst,pe);
    Exit();
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// class PyInstance
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(PyInstance, wxEvtHandler)
    EVT_PY_NOTIFY_UI(wxID_ANY, PyInstance::OnJobNotify)
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
    // kill python process if still running
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

void PyInstance::OnJobNotify(PyNotifyUIEvent &event)
{
    if(event.jobstate==PYSTATE_ABORTEDJOB||event.jobstate==PYSTATE_FINISHEDJOB)
    {
        event.job=m_queue.GetFirst()->GetData();
        event.job->Wait();
        if(event.job->killonexit)
        {
            delete event.job;
            event.job=NULL;
        }
        m_queue.DeleteNode(m_queue.GetFirst());
    }
    ::wxPostEvent(event.parent,event);
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
    PyInstance *p=new PyInstance(_("localhost"),8000);
    if(!p)
        return p;
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

void exec_pycode(const char* code)
{
//  PyRun_SimpleString(code);
}

void exec_interactive_interpreter(int argc, char** argv)
{
//  Py_Initialize();
//  Py_Main(argc, argv);
//  Py_Finalize();
}

void process_expression(char* filename,int num,char** exp)
{
//    FILE*       exp_file;
//    // Initialize a global variable for
//    // display of expression results
//    PyRun_SimpleString("x = 0");
//    // Open and execute the file of
//    // functions to be made available
//    // to user expressions
//    exp_file = fopen(filename, "r");
////    PyRun_SimpleFile(exp_file, exp);
//    // Iterate through the expressions
//    // and execute them
//    while(num--) {
//        PyRun_SimpleString(*exp++);
//        PyRun_SimpleString("print x");
//    }
}

//int main2(int argc, char** argv)
//{
//    Py_Initialize();
//    if(argc != 3) {
//        printf("Usage: \%s FILENAME EXPRESSION+\n");
//        return 1;
//    }
//    process_expression(argv[1], argc - 1, argv + 2);
//    return 0;
//}
//
//int main1(int argc, char** argv)
//{
////    cout << "Starting python shell..." << endl;
//    exec_interactive_interpreter(argc, argv);
//
//    return 0;
//}
