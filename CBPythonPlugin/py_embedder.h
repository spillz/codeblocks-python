#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED

#include <wx/wx.h>
#include <Python.h>
#include <wx/dynarray.h>


class PyJob;
class PyInstance;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_PY_NOTIFY_INTERPRETER, -1)
DECLARE_EVENT_TYPE(wxEVT_PY_NOTIFY_UI, -1)
END_DECLARE_EVENT_TYPES()

//DECLARE_EVENT_MACRO( wxEVT_PY_NOTIFY_INTERPRETER, -1 )
//DECLARE_EVENT_MACRO( wxEVT_PY_NOTIFY_UI, -1 )

// Events sent from the UI to an intepreter
// request for shutdown
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
private:
    PyInstance *instance;
    PyJob *job;
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

//Receiving events
//BEGIN_EVENT_TABLE(MyFrame, wxFrame)
//    EVT_PLOT(ID_MY_WINDOW,  MyFrame::OnPlot)
//END_EVENT_TABLE()
//
//void MyFrame::OnPythonEvent(PyNotifyIntepreterEvent &event)
//{
//    wxPlotCurve *curve = event.GetCurve();
//}

//Sending events
// user code sending the event void MyWindow::SendEvent()
//{
//    PyNotifyIntepreterEvent event( wxEVT_PLOT_ACTION, GetId() );
//    event.SetEventObject( this );
//    event.SetCurve( m_curve );
//    GetEventHandler()->ProcessEvent( event );
//}


// An abstract class for a python job
// the job is defined in the pure virtual method operator()
// The job should try to restrict itself
// to writing to its own non-GUI data members
// for thread safety.
// For intensive tasks, PyJob should
// release the interpreter lock
// from time to time.
class PyJob: public wxThread
{
public:
    virtual void operator()()=0;
    PyJob(wxWindow *p, int id, bool selfdestroy=true):wxThread(wxTHREAD_JOINABLE)
    {
        parent=p;
        this->id=id;
        finished=false;
        started=false;
        killonexit=selfdestroy;
    }
    virtual ~PyJob() {}
protected:
    void Entry()
    {
        PyNotifyUIEvent pe(id,jobowner,PYSTATE_STARTEDJOB);
        ::wxSendEvent(jobowner,pe);
        if(operator())
            pe.SetState(PYSTATE_FINISHEDJOB)
        else
            pe.SetState(PYSTATE_ABORTEDJOB)
        ::wxSendEvent(jobowner,pe);
        Exit();
    }
    PyInstance *jobowner;
    wxWindow *parent;
    int id;
    bool finished;
    bool started;
    bool killonexit;
    friend class PyInstance;
};

WX_DECLARE_LIST(PyJob*, PyJobQueue);

// The interface to an instance of a running interpreter
// The interface maintains a queue of jobs for the interpreter,
// which are run in sequence by a single worker thread.
// Consequently jobs must not interact with live objects on the main thread.
class PyInstance: public wxEvtHandler
{
    PyInstance();
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
};

WX_DECLARE_OBJARRAY(PyInstance, PyInstanceCollection);

// manages the collection of interpreters
class PyMgr
{
    PyMgr();
    ~PyMgr();
    PyInstance *LaunchInterpreter();
private:
    PyInstanceCollection m_Interpreters;
};

#endif //PYEMBEDDER_H_INCLUDED
