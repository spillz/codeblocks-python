#ifndef PPCTRL_H
#define PPCTRL_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "py_embedder.h"

#include <sdk.h>
#include "../ShellExtensions/ShellCtrlBase.h"

class PythonInterpCtrl;

class PyInterpJob: public PyJob
{
public:
    PyInterpJob(wxString code, PyInstance *pyinst, PythonInterpCtrl *pctl, wxWindow *w, int id=wxID_ANY, bool selfdestroy=true) : PyJob(pyinst, w, id, selfdestroy)
    {
        m_code=code;
        this->pctl=pctl;
        m_break_job=false;
        return;
    }
    PythonInterpCtrl *pctl;
    bool operator()(); // main thread loop
    void Break(); // called outside of thread to send break signal to the running interpreter
    wxString m_code;
    wxMutex m_break_mutex;
    bool m_break_job;
};

class PythonCodeCtrl: public wxTextCtrl
{
public:
    PythonCodeCtrl(wxWindow *parent, PythonInterpCtrl *py) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, /*wxTE_RICH|*/ wxTE_MULTILINE|wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxEXPAND) {m_pyctrl = py;}
    void OnUserInput(wxKeyEvent& ke);
    PythonInterpCtrl *m_pyctrl;
    DECLARE_EVENT_TABLE()
};

class PythonIOCtrl: public wxTextCtrl
{
public:
    PythonIOCtrl(wxWindow *parent, PythonInterpCtrl *py)
        : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_MULTILINE|wxTE_READONLY|wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxEXPAND)
        {m_pyctrl = py; m_line_entry_mode=false;}
    void OnUserInput(wxKeyEvent& ke);
    void OnLineInputRequest(wxCommandEvent& e);
    void OnTextChange(wxCommandEvent& e);
    PythonInterpCtrl *m_pyctrl;
    long m_line_entry_point;
    bool m_line_entry_mode;
    DECLARE_EVENT_TABLE()
};


// Allocates ports for xmlrpc connections (ensures unique port allocations)
// TODO: define port as an object to automate release
class PortAllocator: public std::map<int, bool>
{
public:
    PortAllocator(const wxString &portlist=_T("9000;9001;9002"))
    {
        SetPorts(portlist);
    }
    void SetPorts(wxString portlist)
    { //TODO: If new ports overlap current then use current state of the port
        this->clear();
        wxString portstr=portlist.BeforeFirst(';');
        while(portstr!=_(""))
        {
            long port;
            if(portstr.ToLong(&port))
                if(port>0)
                    (*this)[port]=false;
            portlist=portlist.AfterFirst(';');
            portstr=portlist.BeforeFirst(';');
        }
    }
    wxString GetPorts()
    {
        wxString portlist;
        for(PortAllocator::iterator it=this->begin();it!=this->end();it++)
            portlist+=wxString::Format(_T("%i;"),it->first);
        return portlist;
    }
    int RequestPort() //return the first free port
    {
        for(PortAllocator::iterator it=this->begin();it!=this->end();it++)
            if(!(it->second))
            {
                it->second=true;
                return it->first;
            }
        return -1; //no ports available
    }
    bool ReleasePort(int port) //release port, returns false if not found
    {
        PortAllocator::iterator it=this->find(port);
        if(it==PortAllocator::end())
            return false;
        it->second=false;
        return true;
    }
};

namespace
{
ShellCtrlRegistrant<PythonInterpCtrl> reg(_T("Python Interpreter"));
}

class PythonInterpCtrl : public ShellCtrlBase
{
    public:
        PythonInterpCtrl() { m_pyinterp=NULL; m_input_cond=new wxCondition(m_input_mutex); }
        PythonInterpCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr=NULL);
        virtual ~PythonInterpCtrl() { if (m_pyinterp) delete m_pyinterp; delete m_input_cond;}

        virtual long LaunchProcess(const wxString &processcmd, const wxArrayString &options);
        virtual void KillProcess();
        virtual void SyncOutput(int maxchars=1000); //use this to respond to ShellManager request to gather output from the running process for display in the frame
        virtual bool IsDead() {if (m_pyinterp) return m_pyinterp->IsDead(); return true;}

//        long GetPid() {if(m_proc) return m_procid; else return -1;}

        void OnUserInput(wxKeyEvent& ke);
        void OnSize(wxSizeEvent& event);
        void OnPyNotify(wxCommandEvent& event);
        void OnLineInputRequest(wxCommandEvent& event);
        void OnPyCode(wxCommandEvent& event);
        void OnPyJobDone(wxCommandEvent& event);
        void OnPyJobAbort(wxCommandEvent& event);
        void OnEndProcess(wxCommandEvent &ce);

        bool DispatchCode(const wxString &code);

        bool BreakCode();

        static PortAllocator m_portalloc;

        void stdin_append(const wxString &data);
        void stdout_append(const wxString &data);
        void stderr_append(const wxString &data);
        wxString stdin_retrieve();
        wxString stdout_retrieve();
        wxString stderr_retrieve();

        wxMutex m_input_mutex;
        wxCondition *m_input_cond;

    protected:
        friend class PyInterpJob;
        bool RunCode(const wxString &codestr, int &unfinished);
        bool Continue(int &unfinished, bool &line_input_request);
        bool SendKill();

    private:
        wxString stdin_data, stdout_data, stderr_data;
        wxMutex io_mutex;
        PythonIOCtrl *m_ioctrl;
        PythonCodeCtrl *m_codectrl;
        wxSplitterWindow *m_sw;
        PyInstance *m_pyinterp;
        wxString m_code; //currently running code
        int m_killlevel;
        int m_port;

//        void OnEndProcess(wxProcessEvent &event);
    DECLARE_DYNAMIC_CLASS(PythonInterpCtrl)
    DECLARE_EVENT_TABLE()
};

#endif // PPCTRL_H
