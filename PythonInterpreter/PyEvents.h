#ifndef PYEVENTS_H_INCLUDED
#define PYEVENTS_H_INCLUDED

#include <wx/wx.h>

/////////////////////////////////////////////////////////////////////////////////////
// Python Instance Events
/////////////////////////////////////////////////////////////////////////////////////

class PyJob;
class PyInstance;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_INTERPRETER, -1)
//DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CORE,wxEVT_PY_NOTIFY_INTERPRETER, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI, -1)
//DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CORE,wxEVT_PY_NOTIFY_UI, -1)

// SIMPLE wxCommandEvent DERIVED CUSTOM EVENTS THAT USE THE BUILTIN EVT_COMMAND EVENT TABLE ENTRY
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_STARTED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_FINISHED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_ABORTED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_NOTIFY, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_PY_PROC_END, -1)
END_DECLARE_EVENT_TYPES()




//// TO INTERCEPT THE wxCommandEvent CUSTOM EVENTS:
//
//BEGIN_EVENT_TABLE(MyFrame, wxFrame)
//  EVT_MENU    (wxID_EXIT, MyFrame::OnExit)
//  // ....
//  EVT_COMMAND  (ID_MY_WINDOW, wxEVT_PY_NOTIFY_UI_STARTED, MyFrame::OnMyEvent)
//END_EVENT_TABLE()
//
//void MyFrame::OnMyEvent( wxCommandEvent &event )
//{
//    // do something
//    wxString text = event.GetText();
//}
//
//
//// TO SEND THE wxCommandEvent CUSTOM EVENT:
//
//void MyWindow::SendEvent()
//{
//    wxCommandEvent event( wxEVT_PY_NOTIFY_UI_STARTED, GetId() );
//    event.SetEventObject( this );
//    // Give it some contents
//    event.SetText( wxT("Hallo") );
//    // Send it
//    GetEventHandler()->ProcessEvent( event );
//}




// Events sent from the UI to an intepreter request for shutdown
class PyNotifyIntepreterEvent: public wxEvent
{
public:
    PyNotifyIntepreterEvent(int id = -1);
    PyNotifyIntepreterEvent(const PyNotifyIntepreterEvent& c) : wxEvent(c) { }
    wxEvent *Clone() const { return new PyNotifyIntepreterEvent(*this); }
    ~PyNotifyIntepreterEvent() {}
};

enum JobStates {PYSTATE_STARTEDJOB, PYSTATE_FINISHEDJOB, PYSTATE_ABORTEDJOB, PYSTATE_NOTIFY};


// Events sent from the thread interacting with the python interpreter back to the UI.
// indicating job completion, interpreter shutdown etc
class PyNotifyUIEvent: public wxEvent
{
    friend class PyInstance;
public:
    PyNotifyUIEvent(int id=-1): wxEvent(id,wxEVT_PY_NOTIFY_UI) {}
    PyNotifyUIEvent(int id, PyInstance *instance, wxWindow *parent, JobStates jobstate);
    PyNotifyUIEvent(const PyNotifyUIEvent& c) : wxEvent(c)
    {
        jobstate = c.jobstate;
        job= c.job;
        instance= c.instance;
        parent= c.parent;
    }
    wxEvent *Clone() const { return new PyNotifyUIEvent(*this); }
    ~PyNotifyUIEvent() {}
    PyJob *GetJob() {return job;}
    PyInstance *GetInterpreter();
    void SetState(JobStates s) {jobstate=s;}
    JobStates GetState() {return jobstate;}
protected:
    JobStates jobstate;
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

#define EVT_PY_NOTIFY_UI(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_PY_NOTIFY_UI, -1, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( PyNotifyUIEventFunction, & fn ), (wxObject *) NULL ),



#endif //PYEVENTS_H_INCLUDED

