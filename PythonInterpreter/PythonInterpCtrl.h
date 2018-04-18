#ifndef PPCTRL_H
#define PPCTRL_H

#ifndef WX_PRECOMP
    #include <wx/splitter.h>
#endif

#include "xmlrpc_embedder.h"
#include "ShellCtrlBase.h"
#include "editormanager.h"
#include "cbstyledtextctrl.h"

class PythonInterpCtrl;

class PythonCodeCtrl: public cbStyledTextCtrl
{
public:
    PythonCodeCtrl(wxWindow *parent, PythonInterpCtrl *py);
    wxArrayString m_history_commands;
    wxString m_history_working;
    int m_history_position;
private:
    void OnUserInput(wxKeyEvent& ke);
    void OnCharAdded(wxScintillaEvent& ke);
    PythonInterpCtrl *m_pyctrl;
    DECLARE_EVENT_TABLE()
};

class PythonIOCtrl: public wxTextCtrl
{
public:
    PythonIOCtrl(wxWindow *parent, PythonInterpCtrl *py);
    void OnUserInput(wxKeyEvent& ke);
    void LineInputRequest();
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
    friend class PythonIOCtrl;
    public:
        PythonInterpCtrl() { m_pyinterp=NULL; }
        PythonInterpCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr=NULL);
        virtual ~PythonInterpCtrl() { if (m_pyinterp) delete m_pyinterp; }

        virtual long LaunchProcess(const wxString &processcmd, const wxArrayString &options);
        virtual void KillProcess();
        virtual void SyncOutput(int maxchars=1000); //use this to respond to ShellManager request to gather output from the running process for display in the frame
        virtual bool IsDead() {if (m_pyinterp) return m_pyinterp->IsDead(); return true;}

//        long GetPid() {if(m_proc) return m_procid; else return -1;}

        void OnUserInput(wxKeyEvent& ke);
        void OnSize(wxSizeEvent& event);
        void OnPyNotify(XmlRpcResponseEvent& event);
        void OnLineInputRequest(wxCommandEvent& event);
        void OnPyCode(wxCommandEvent& event);
        void OnEndProcess(wxCommandEvent& event);

        bool DispatchCode(const wxString &code);

        bool BreakCode();

        static PortAllocator m_portalloc;

        void stdin_append(const wxString &data);
        wxString stdin_retrieve();

    protected:
        bool RunCode(const wxString &codestr);
        bool Continue();
        bool SendKill();

    private:
        wxString stdin_data;
        PythonIOCtrl *m_ioctrl;
        PythonCodeCtrl *m_codectrl;
        wxSplitterWindow *m_sw;
        XmlRpcInstance *m_pyinterp;
        wxString m_code; //currently running code
        int m_killlevel;
        int m_port;

//        void OnEndProcess(wxProcessEvent &event);
    DECLARE_DYNAMIC_CLASS(PythonInterpCtrl)
    DECLARE_EVENT_TABLE()
};

#endif // PPCTRL_H
