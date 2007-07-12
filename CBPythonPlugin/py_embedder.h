#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED

#include <wx/wx.h>
#include <Python.h>
#include <wx/dynarray.h>

// Events sent from the UI to an intepreter
class PyNotifyIntepreterEvent: public wxEvent
{
public:
};

// Events sent from the thread interacting with the python interpreter back to the UI.
class PyNotifyUIEvent: public wxEvent
{
public:

};

DECLARE_EVENT_MACRO( wxEVT_PY_NOTIFY_INTERPRETER, -1 )
DECLARE_EVENT_MACRO( wxEVT_PY_NOTIFY_UI, -1 )

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

/*
// code implementing the event type and the event class (move to the .cpp file)

DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_INTERPRETER )
PyNotifyIntepreterEvent::PyNotifyIntepreterEvent( ...
DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI )
*/



// An abstract class for a python job
class PyJob
{
    virtual void operator()()=0;
};

WX_DECLARE_LIST(PyJob, PyJobQueue);

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
    void Kill(bool force=false;) {}
    void AddJob(PyJob *job);
    PyThreadState *tstate;
    wxThread *m_thread;
    PyJobQueue m_queue;
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
