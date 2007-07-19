#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dynarray.h>

#include <memory>
#include <Python.h>

class PyJob;
class PyInstance;

/////////////////////////////////////////////////////////////////////////////////////
// Python Interpreter Events
/////////////////////////////////////////////////////////////////////////////////////

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_PY_NOTIFY_INTERPRETER, -1)
DECLARE_EVENT_TYPE(wxEVT_PY_NOTIFY_UI, -1)
END_DECLARE_EVENT_TYPES()

// Events sent from the UI to an intepreter request for shutdown
class PyNotifyIntepreterEvent: public wxEvent
{
public:
    PyNotifyIntepreterEvent(int id);
    PyNotifyIntepreterEvent(const PyNotifyIntepreterEvent& c) : wxEvent(c) { }
    wxEvent *Clone() const { return new PyNotifyIntepreterEvent(*this); }
    ~PyNotifyIntepreterEvent() {}
};

enum JobStates {PYSTATE_STARTEDJOB, PYSTATE_FINISHEDJOB, PYSTATE_ABORTEDJOB};

// Events sent from the thread interacting with the python interpreter back to the UI.
// indicating job completion, interpreter shutdown etc
class PyNotifyUIEvent: public wxEvent
{
    friend class PyInstance;
public:
    PyNotifyUIEvent(int id, PyInstance *instance, JobStates jobstate);
    PyNotifyUIEvent(const PyNotifyUIEvent& c) : wxEvent(c)
    {
        jobstate = c.jobstate;
        instance= c.instance;
    }
    wxEvent *Clone() const { return new PyNotifyUIEvent(*this); }
    ~PyNotifyUIEvent() {}
    PyJob *GetJob() {return job;}
    PyInstance *GetInterpreter();
    void SetState(JobStates s) {jobstate=s;}
    JobStates jobstate;
protected:
    PyInstance *instance;
    PyJob *job;
    wxWindow *parent;
};

typedef void (wxEvtHandler::*PyNotifyIntepreterEventFunction)(PyNotifyIntepreterEvent&);
typedef void (wxEvtHandler::*PyNotifyUIEventFunction)(PyNotifyUIEvent&);

#define EVT_PY_NOTIFY_INTERPRETER(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_PY_NOTIFY_INTERPRETER, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) \
    wxStaticCastEvent( PyNotifyIntepreterEventFunction, & fn ), (wxObject *) NULL ),

#define EVT_PY_NOTIFY_UI(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_PY_NOTIFY_UI, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) \
    wxStaticCastEvent( PyNotifyUIEventFunction, & fn ), (wxObject *) NULL ),


/////////////////////////////////////////////////////////////////////////////////////
// PyJob: An abstract class for a python job to be run by an interpreter instance.
// The job is defined in the pure virtual method operator(). The job should try to
// restrict itself to writing to its own non-GUI data members for thread safety.
// For intensive tasks, PyJob should release the interpreter lock from time to time.
// The PyJob starts in a released state. before making any API calls, call Lock
// you must release the lock with Release before returning.
/////////////////////////////////////////////////////////////////////////////////////
class PyJob: public wxThread
{
public:
    virtual bool operator()();
    PyJob(wxWindow *p, int id, bool selfdestroy=true);
    void Lock();
    void Release();
    virtual ~PyJob();
protected:
    virtual void *Entry();
    PyInstance *jobowner;
    wxWindow *parent;
    int id;
    bool finished;
    bool started;
    bool killonexit;
    friend class PyInstance;
};

WX_DECLARE_LIST(PyJob, PyJobQueue);

/////////////////////////////////////////////////////////////////////////////////////
// PyInstance: The interface to an instance of a running interpreter
// The interface maintains a queue of jobs for the interpreter,
// which are run in sequence by a single worker thread.
// Consequently jobs must not interact with live objects on the main thread.
/////////////////////////////////////////////////////////////////////////////////////
class PyInstance: public wxEvtHandler
{
public:
    PyInstance();
    PyInstance(const PyInstance &copy)
    {
        tstate=copy.tstate;
        m_queue=copy.m_queue;
    }
    ~PyInstance();
    void Lock();
    void Release();
    void EvalString(char *str, bool wait=true);
    void Kill(bool force=false) {}
    bool AddJob(PyJob *job);
    void OnJobNotify(PyNotifyUIEvent &event);
    void PauseJobs();
    PyThreadState *tstate;
    PyJobQueue m_queue;
    bool m_paused;
    //void AttachExtension(); //attach a python extension table as an import for this interpreter
    DECLARE_EVENT_TABLE();
};

WX_DECLARE_OBJARRAY(PyInstance, PyInstanceCollection);

/////////////////////////////////////////////////////////////////////////////////////
// PyMgr: manages the collection of interpreters
/////////////////////////////////////////////////////////////////////////////////////
class PyMgr
{
public:
    PyInstance *LaunchInterpreter();
    static PyMgr &Get();
    ~PyMgr();
protected:
    PyMgr();
private:
    PyInstanceCollection m_Interpreters;
    static std::auto_ptr<PyMgr> theSingleInstance;
};


#endif //PYEMBEDDER_H_INCLUDED
