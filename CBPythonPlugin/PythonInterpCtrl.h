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
    PyInterpJob(wxString code, PyInstance *pyinst, PythonInterpCtrl *pctl, wxWindow *p, int id=wxID_ANY, bool selfdestroy=true) : PyJob(pyinst, p, id, selfdestroy)
    {
        this->pctl=pctl;
        return;
    }
    PythonInterpCtrl *pctl;
    bool operator()();
    wxString code;
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
        PyInterpJob *m_job;
        wxTextCtrl *m_textctrl;
        PyInstance *m_pyinterp;


//        void OnEndProcess(wxProcessEvent &event);
    DECLARE_DYNAMIC_CLASS(wxPanel)
    DECLARE_EVENT_TABLE()
};


#endif // PPCTRL_H
