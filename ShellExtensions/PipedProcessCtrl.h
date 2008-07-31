#ifndef PPCTRL_H
#define PPCTRL_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/process.h>
#include <wx/wxFlatNotebook/wxFlatNotebook.h>

#include <sdk.h>
#include "ShellCtrlBase.h"

class PipedProcessCtrl;

namespace
{
ShellCtrlRegistrant<PipedProcessCtrl> reg(_T("Piped Process Control"));
}

class PipedTextCtrl: public wxTextCtrl
{
public:
    PipedTextCtrl(wxWindow *parent, PipedProcessCtrl *pp) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, /*wxTE_RICH|*/ wxTE_RICH|wxTE_MULTILINE|wxTE_READONLY|wxTE_PROCESS_ENTER|wxEXPAND) {m_pp = pp;}
    void OnDClick(wxMouseEvent& e);
    PipedProcessCtrl *m_pp;
    DECLARE_EVENT_TABLE()
};


class PipedProcessCtrl : public ShellCtrlBase
{
    public:
        PipedProcessCtrl() {}
        PipedProcessCtrl(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr=NULL);
        virtual ~PipedProcessCtrl() {if (m_proc) {if (!m_dead) {m_proc->Detach();} } }
        void ParseLinks(int lineno, int lastline);
        long LaunchProcess(const wxString &processcmd, const wxArrayString &options); //bool ParseLinks=true, bool LinkClicks=true, const wxString &LinkRegex=LinkRegexDefault
        void KillProcess();
        void KillWindow();
        bool IsDead() {return m_dead;}
        long GetPid() {if(m_proc) return m_procid; else return -1;}
        void SyncOutput(int maxchars=1000);
        void OnUserInput(wxKeyEvent& ke);
        void OnDClick(wxMouseEvent &e);
        void OnSize(wxSizeEvent& event);
        bool ParsesLinks(wxString &LinkRegex) {LinkRegex=m_linkregex;return m_parselinks;}
        void ParseLinks(bool parselinks=true, wxString LinkRegex=LinkRegexDefault) {m_parselinks=parselinks;}
        static wxString LinkRegexDefault;
    private:
        PipedTextCtrl *m_textctrl;
        wxProcess *m_proc;
        long m_procid;
        wxOutputStream *m_ostream;
        wxInputStream *m_istream;
        wxInputStream *m_estream;
        void OnEndProcess(wxProcessEvent &event);
        wxString m_lateststreamdata;
        wxString m_latesterrstreamdata;
        int m_killlevel;
        int m_exitcode;
        wxString m_linkregex;
        bool m_parselinks;
        bool m_linkclicks;
        bool m_dead;
    DECLARE_DYNAMIC_CLASS(wxPanel)
    DECLARE_EVENT_TABLE()
};


#endif // PPCTRL_H
