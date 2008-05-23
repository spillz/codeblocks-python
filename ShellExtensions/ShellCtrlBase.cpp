#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/regex.h>
#include "ShellCtrlBase.h"
#include <globals.h>

// The global instance of the shell registry
ShellRegistry GlobalShellRegistry;

// Unique IDs for Timer and Shell Manager messages
int ID_SHELLPOLLTIMER=wxNewId();
int ID_SHELLMGR=wxNewId();

////////////////////////////////////// ShellManager /////////////////////////////////////////////

BEGIN_EVENT_TABLE(ShellManager, wxPanel)
    EVT_CHAR(ShellManager::OnUserInput)
    EVT_TIMER(ID_SHELLPOLLTIMER, ShellManager::OnPollandSyncOutput)
    EVT_FLATNOTEBOOK_PAGE_CLOSING(ID_SHELLMGR, ShellManager::OnPageClosing)
END_EVENT_TABLE()


void ShellManager::OnPageClosing(wxFlatNotebookEvent& event)
{
    ShellCtrlBase* sh = GetPage(event.GetSelection());
    //    LOGSTREAM << wxString::Format(_T("OnPageClosing(): ed=%p, title=%s\n"), eb, eb ? eb->GetTitle().c_str() : _T(""));
    if (!QueryClose(sh))
        event.Veto();
//    event.Skip(); // allow others to process it too
}


bool ShellManager::QueryClose(ShellCtrlBase* sh)
{
    if(!sh)
        return true;
    if(!sh->IsDead())
    {
        wxString msg(_("Process \"")+sh->GetName()+_("\" is still running...\nDo you want to kill it?"));
        switch (cbMessageBox(msg, _("Kill process?"), wxICON_QUESTION | wxYES_NO))
        {
        case wxID_YES:
            sh->KillProcess();
            return false;
        case wxID_NO:
            return false;
        }
    }
    return true;
}


long ShellManager::LaunchProcess(const wxString &processcmd, const wxString &name, const wxString &type, const wxArrayString &options)
{
    int id=wxNewId();
    ShellCtrlBase *shell=GlobalShellRegistry.CreateControl(type,m_nb,id,name,this);
    if(!shell)
    {
        cbMessageBox(wxString::Format(_T("Console type %s not found in registry."),type.c_str()));
        return -1;
    }
    long procid=shell->LaunchProcess(processcmd,options);
    if(procid>0)
    {
        if(!m_synctimer.IsRunning())
            m_synctimer.Start(100);
    }
    else
    {
        cbMessageBox(_T("process launch failed."));
        delete shell; //TODO: GlobalShellRegistry.FreeControl() ???
        return -1;
    }
    m_nb->AddPage(shell,name);
    m_nb->SetSelection(m_nb->GetPageCount()-1);
    shell->Show();
    return procid;
}

ShellCtrlBase *ShellManager::GetPage(size_t i)
{
    return (ShellCtrlBase*)m_nb->GetPage(i);
}

ShellCtrlBase *ShellManager::GetPage(const wxString &name)
{
    for(int i=0;i<m_nb->GetPageCount();i++)
    {
        ShellCtrlBase *sh=GetPage(i);
        if(name==sh->GetName())
            return sh;
    }
    return NULL;
}


// Forecefully kill the process
void ShellManager::KillProcess(int id)
{
}

void ShellManager::KillWindow(int id)
{
}

size_t ShellManager::GetTermNum(ShellCtrlBase *term)
{
    for(int i=0;i<m_nb->GetPageCount();i++)
    {
        ShellCtrlBase *shell=GetPage(i);
        if(shell==term)
            return i;
    }
    return m_nb->GetPageCount();
}

int ShellManager::NumAlive()
{
    int count=0;
    for(int i=0;i<m_nb->GetPageCount();i++)
        count+=!GetPage(i)->IsDead();
    return count;
}


void ShellManager::OnShellTerminate(ShellCtrlBase *term)
{
    size_t i=GetTermNum(term);
    m_nb->SetPageText(i,_T("[DONE]")+m_nb->GetPageText(i));
    if(NumAlive()==0)
        m_synctimer.Stop();
}


void ShellManager::OnPollandSyncOutput(wxTimerEvent& te)
{
    for(int i=0;i<m_nb->GetPageCount();i++)
    {
        GetPage(i)->SyncOutput();
    }
}

void ShellManager::OnUserInput(wxKeyEvent& ke)
{ //TODO: This shouldn't be necessary as individual pages will have the focus
//    ShellCtrlBase *sh=(ShellCtrlBase*)m_nb->GetCurrentPage();
//    sh->OnUserInput(ke);
}

ShellManager::ShellManager(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL/* | wxCLIP_CHILDREN*/)
{
    m_synctimer.SetOwner(this, ID_SHELLPOLLTIMER);
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    m_nb = new wxFlatNotebook(this, ID_SHELLMGR, wxDefaultPosition, wxDefaultSize, wxFNB_X_ON_TAB|wxFNB_NO_X_BUTTON);
    bs->Add(m_nb, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);
    SetSizer(bs);
}

ShellManager::~ShellManager()
{
    //dtor
    //All of the subwindows owned by this panel will be destroyed automatically
}
