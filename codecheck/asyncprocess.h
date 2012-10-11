#ifndef ASYNCPROCESS_H_INCLUDED
#define ASYNCPROCESS_H_INCLUDED

#include <wx/wx.h>
#include <wx/process.h>


//Runs a child process asynchronously with output + error redirected, posting a message to the parent when complete
class AsyncProcess: public wxEvtHandler
{
public:
    AsyncProcess(wxEvtHandler *parent)
    {
        m_parent=parent;
        m_exec_proc=NULL;
        return;
    }
    ~AsyncProcess();
    int Exec(const wxString &command);
    wxString GetStdout() {return m_exec_output;}
    wxString GetStderr() {return m_exec_err;}
    void OnExecTerminate(wxProcessEvent &e);
    void OnExecTimer(wxTimerEvent &e);
private:
    wxProcess *m_exec_proc;
    wxInputStream *m_exec_stream, *m_exec_errstream;
    int m_exec_proc_id;
    wxTimer *m_exec_timer;
    wxString m_exec_output, m_exec_err;
    void ReadStream(bool all=false);
    wxEvtHandler *m_parent;
    DECLARE_EVENT_TABLE()
};

#endif //ASYNCPROCESS_H_INCLUDED
