#include "XmlRpcEvents.h"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// classes PyNotifyIntepreterEvent and PyNotifyUIEvent
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DEFINE_EVENT_TYPE(wxEVT_XMLRPC_STARTED)
DEFINE_EVENT_TYPE(wxEVT_XMLRPC_FINISHED)
DEFINE_EVENT_TYPE(wxEVT_XMLRPC_ABORTED)
DEFINE_EVENT_TYPE(wxEVT_XMLRPC_NOTIFY)
DEFINE_EVENT_TYPE(wxEVT_XMLRPC_RESPONSE)
DEFINE_EVENT_TYPE(wxEVT_XMLRPC_PROC_END)


XmlRpcResponseEvent::XmlRpcResponseEvent(int id, XmlRpcInstance *inst, wxEvtHandler *window, JobStates js, XmlRpc::XmlRpcValue val) : wxEvent(id, wxEVT_XMLRPC_RESPONSE)
{
    instance=inst;
    jobstate=js;
    job=NULL;
    parent=window;
    value=val;
}
