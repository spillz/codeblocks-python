#include "py_embedder.h"
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(PyJobQueue);

using namespace std;


// code implementing the event types and classes (move to the .cpp file)
DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_INTERPRETER )

PyNotifyIntepreterEvent::PyNotifyIntepreterEvent(int id) : wxEvent(id, wxEVT_PY_NOTIFY_INTERPRETER) {}


DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI )

PyNotifyUIEvent::PyNotifyUIEvent(int id, PyInstance *inst, JobStates js) : wxEvent(id, wxEVT_PY_NOTIFY_UI)
{
    instance=inst;
    jobstate=js;
    job=NULL;
}





BEGIN_EVENT_TABLE(PyInstance, wxEvtHandler)
    EVT_PY_NOTIFY_UI(wxID_ANY, PyInstance::OnJobNotify)
END_EVENT_TABLE()


PyInstance::PyInstance()
{
    PyEval_AcquireLock();
    tstate=Py_NewInterpreter();
    PyEval_ReleaseLock();
}

PyInstance::~PyInstance()
{
    PyEval_AcquireLock();
    Py_EndInterpreter(tstate);
    PyEval_ReleaseLock();
}

bool PyInstance::AddJob(PyJob *job)
{
    if(job->Create()!=wxTHREAD_NO_ERROR)
        return false;
    m_queue.Append(job);
    if(m_paused)
        return;
    PyJob *newjob=m_queue.GetFirst()->GetData();
    if(!newjob->finished && !newjob->started)
    {
        newjob->started=true;
        newjob->Run();
    }
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
    ::wxSendEvent(event.parent,pe);
    if(m_paused)
        return;
    wxNode node=m_queue.GetFirst();
    if(!node)
        return;
    PyJob *newjob=node->GetData();
    if(!newjob->finished && !newjob->started)
    {
        newjob->started=true;
        newjob->Run();
    }
}


// Not sure about this locking stuff...
void PyInstance::Lock()
{
    PyEval_AcquireThread(tstate);
}

void PyInstance::EvalString(char *str, bool wait)
{
    PyRun_SimpleString(str);
}

void PyInstance::Release()
{
    PyEval_ReleaseThread(tstate);
}

PyMgr::PyMgr()
{
    Py_Initialize();// need to init threads?
}

PyMgr::~PyMgr()
{
    Py_Finalize();
}



void exec_pycode(const char* code)
{
  PyRun_SimpleString(code);
}


void exec_interactive_interpreter(int argc, char** argv)
{
  Py_Initialize();
  Py_Main(argc, argv);
  Py_Finalize();
}


void process_expression(char* filename,int num,char** exp)
{
    FILE*       exp_file;
    // Initialize a global variable for
    // display of expression results
    PyRun_SimpleString("x = 0");
    // Open and execute the file of
    // functions to be made available
    // to user expressions
    exp_file = fopen(filename, "r");
//    PyRun_SimpleFile(exp_file, exp);
    // Iterate through the expressions
    // and execute them
    while(num--) {
        PyRun_SimpleString(*exp++);
        PyRun_SimpleString("print x");
    }
}
int main2(int argc, char** argv)
{
    Py_Initialize();
    if(argc != 3) {
        printf("Usage: \%s FILENAME EXPRESSION+\n");
        return 1;
    }
    process_expression(argv[1], argc - 1, argv + 2);
    return 0;
}

int main1(int argc, char** argv)
{
//	cout << "Starting python shell..." << endl;
	exec_interactive_interpreter(argc, argv);

	return 0;
}
