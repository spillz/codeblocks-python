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
        this->code=code;
        this->pctl=pctl;
        break_job=false;
        return;
    }
    PythonInterpCtrl *pctl;
    bool operator()(); //TODO: Handle the break
    wxString code;
    wxMutex break_mutex;
    bool break_job;
};

class PythonCodeCtrl: public wxTextCtrl
{
public:
    PythonCodeCtrl(wxWindow *parent, PythonInterpCtrl *py) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RICH|wxTE_MULTILINE|wxTE_PROCESS_ENTER|wxEXPAND) {m_pyctrl = py;}
    void OnUserInput(wxKeyEvent& ke);
    PythonInterpCtrl *m_pyctrl;
    DECLARE_EVENT_TABLE()
};


namespace
{
ShellCtrlRegistrant<PythonInterpCtrl> reg(_T("Python Interpreter"));
}

class PythonInterpCtrl : public ShellCtrlBase
{
    public:
        PythonInterpCtrl() { m_pyinterp=NULL; }
        PythonInterpCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr=NULL);
        virtual ~PythonInterpCtrl() { if (m_pyinterp) delete m_pyinterp; }

        virtual long LaunchProcess(const wxString &processcmd, const wxArrayString &options);
        virtual void KillProcess();
        virtual void SyncOutput(int maxchars=1000); //use this to respond to ShellManager request to gather output from the running process for display in the frame

//        long GetPid() {if(m_proc) return m_procid; else return -1;}

        void OnUserInput(wxKeyEvent& ke);
        void OnSize(wxSizeEvent& event);
        void OnPyNotify(wxCommandEvent& event);

        bool DispatchCode(const wxString &code);


    protected:
        friend class PyInterpJob;
        void stdin_append(const wxString &data);
        void stdout_append(const wxString &data);
        void stderr_append(const wxString &data);
        wxString stdin_retrieve();
        wxString stdout_retrieve();
        wxString stderr_retrieve();
        bool BreakCode();
        bool RunCode(const wxString &codestr, bool &unfinished);
        bool Continue(bool &unfinished);
        bool SendKill();

    private:
        wxString stdin_data, stdout_data, stderr_data;
        wxMutex io_mutex;
        wxTextCtrl *m_ioctrl, *m_codectrl;
        wxSplitterWindow *m_sw;
        PyInstance *m_pyinterp;


//        void OnEndProcess(wxProcessEvent &event);
    DECLARE_DYNAMIC_CLASS(PythonInterpCtrl)
    DECLARE_EVENT_TABLE()
};


#endif // PPCTRL_H
