#ifndef PYEVENTS_H_INCLUDED
#define PYEVENTS_H_INCLUDED

#include <wx/wx.h>
#include "XmlRpc.h"

/////////////////////////////////////////////////////////////////////////////////////
// XmlRpc Instance Events
/////////////////////////////////////////////////////////////////////////////////////

class XmlRpcJob;
class XmlRpcInstance;

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxEVT_XMLRPC_RESPONSE, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_XMLRPC_STARTED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_XMLRPC_FINISHED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_XMLRPC_ABORTED, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_XMLRPC_NOTIFY, -1)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_XMLRPC_PROC_END, -1)
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





enum JobStates {XMLRPC_STATE_STARTEDJOB, XMLRPC_STATE_FINISHEDJOB, XMLRPC_STATE_ABORTEDJOB, XMLRPC_STATE_NOTIFY, XMLRPC_STATE_ERROR, XMLRPC_STATE_RESPONSE, XMLRPC_STATE_REQUEST_FAILED};


// Events sent from the thread interacting with the python interpreter back to the UI.
// indicating job completion, interpreter shutdown etc
class XmlRpcResponseEvent: public wxEvent
{
    friend class XmlRpcInstance;
public:
    XmlRpcResponseEvent(int id=-1): wxEvent(id,wxEVT_XMLRPC_RESPONSE) {}
    XmlRpcResponseEvent(int id, XmlRpcInstance *instance, wxEvtHandler *parent, JobStates jobstate, XmlRpc::XmlRpcValue value);
    XmlRpcResponseEvent(const XmlRpcResponseEvent& c) : wxEvent(c)
    {
        jobstate = c.jobstate;
        job= c.job;
        instance= c.instance;
        parent= c.parent;
        value=c.value;
    }
    wxEvent *Clone() const { return new XmlRpcResponseEvent(*this); }
    ~XmlRpcResponseEvent() {}
    XmlRpcJob *GetJob() {return job;}
    XmlRpcInstance *GetInterpreter();
    XmlRpc::XmlRpcValue GetResponse() {return value;}
    void SetState(JobStates s) {jobstate=s;}
    JobStates GetState() {return jobstate;}
protected:
    JobStates jobstate;
    XmlRpcInstance *instance;
    XmlRpcJob *job;
    wxEvtHandler *parent;
    XmlRpc::XmlRpcValue value;
};


typedef void (wxEvtHandler::*XmlRpcResponseEventFunction)(XmlRpcResponseEvent&);

#define EVT_XMLRPC_RESPONSE(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_XMLRPC_RESPONSE, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( XmlRpcResponseEventFunction, & fn ), (wxObject *) NULL ),



#endif //PYEVENTS_H_INCLUDED

