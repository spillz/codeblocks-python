#include "PyEvents.h"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// classes PyNotifyIntepreterEvent and PyNotifyUIEvent
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DEFINE_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_STARTED)
DEFINE_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_FINISHED)
DEFINE_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_ABORTED)
DEFINE_EVENT_TYPE(wxEVT_PY_NOTIFY_UI_NOTIFY)
DEFINE_EVENT_TYPE(wxEVT_PY_PROC_END)


DEFINE_EVENT_TYPE(wxEVT_PY_NOTIFY_INTERPRETER );

PyNotifyIntepreterEvent::PyNotifyIntepreterEvent(int id) : wxEvent(id, wxEVT_PY_NOTIFY_INTERPRETER)
{
}

DEFINE_EVENT_TYPE( wxEVT_PY_NOTIFY_UI );

PyNotifyUIEvent::PyNotifyUIEvent(int id, PyInstance *inst, wxWindow *window, JobStates js) : wxEvent(id, wxEVT_PY_NOTIFY_UI)
{
    instance=inst;
    jobstate=js;
    job=NULL;
    parent=window;
}


