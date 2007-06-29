#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/regex.h>
#include "ShellCtrl.h"
#include "globals.h"

////////////////////////////////////// ShellTermCtrl /////////////////////////////////////////////
int ID_SHELLPOLLTIMER=wxNewId();
int ID_PROC=wxNewId();
int ID_SHELLMGR=wxNewId();


BEGIN_EVENT_TABLE(ShellTermCtrl, wxTextCtrl)
    EVT_CHAR(ShellTermCtrl::OnUserInput)
    EVT_END_PROCESS(ID_PROC, ShellTermCtrl::OnEndProcess)
    EVT_LEFT_DCLICK(ShellTermCtrl::OnDClick)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(ShellTermCtrl, wxTextCtrl)

ShellTermCtrl::ShellTermCtrl(wxWindow* parent,wxWindowID id, ShellManager *shellmgr,
    const wxString& name,const wxString& value,const wxPoint& pos,const wxSize& size,long style)
    : wxTextCtrl(parent, id, value, pos, size, style)
{
    m_shellmgr=shellmgr;
    m_name=name;
    m_dead=true;
    m_proc=NULL;
    m_killlevel=0;
}


void ShellTermCtrl::OnEndProcess(wxProcessEvent &event)
{
    m_exitcode=event.GetExitCode();
    ReadStream(-1); //read any left over output TODO: while loop to handle extremely large amount of output
    m_dead=true;
    delete m_proc;
    m_proc=NULL;
    m_killlevel=0;
    if(m_shellmgr)
        m_shellmgr->OnShellTerminate(this);
}

long ShellTermCtrl::LaunchProcess(wxString processcmd, int stderrmode)
{
    if(!m_dead)
        return -1;
    if(m_proc) //this should never happen
        m_proc->Detach(); //self cleanup
    m_proc=new wxProcess(this,ID_PROC);
    m_proc->Redirect();
    m_procid=wxExecute(processcmd,wxEXEC_ASYNC,m_proc);
    if(m_procid>0)
    {
        m_ostream=m_proc->GetOutputStream();
        m_istream=m_proc->GetInputStream();
        m_estream=m_proc->GetErrorStream();
        m_dead=false;
        m_killlevel=0;
    }
    return m_procid;
}

void ShellTermCtrl::KillProcess()
{
    if(m_dead)
        return;
//    if(m_killlevel==0) //some process will complete if we send EOF. TODO: make sending EOF a separate option
//    {
//        m_proc->CloseOutput();
//        m_killlevel=1;
//        return;
//    }
    long pid=GetPid();
    if(m_killlevel==0)
    {
        m_killlevel=1;
        if(wxProcess::Exists(pid))
            wxProcess::Kill(pid,wxSIGTERM);
        return;
    }
    if(m_killlevel==1)
    {
        if(wxProcess::Exists(pid))
        {
            cbMessageBox(_T("Forcing..."));
            wxProcess::Kill(pid,wxSIGKILL);
        }
    }
}

static wxString linkregex=_T("[\"']?([^'\",\\s:;*?]+?)[\"']?[\\s]*(\\:|\\(|\\[|\\,?\\s*[Ll]ine)?\\s*(\\d*)");
static wxString linkregex2=_T("(\\d*)\\s*(\\:|\\)|\\]|of|in)\\s*[^:;*?\\r\\n\\s]*");


void ShellTermCtrl::ReadStream(int maxchars)
{
    if(!m_proc)
        return;
    bool oneshot=true;
    if(maxchars<=0)
    {
        maxchars=20000;
        oneshot=false;
    }
    int lineno=GetNumberOfLines()-1;
    if(lineno<0) lineno=0;
    while(m_proc->IsInputAvailable())
    {
        char buf0[maxchars+1];
        for(int i=0;i<maxchars+1;i++)
            buf0[i]=0;
        m_istream->Read(buf0,maxchars);
        wxString m_latest=wxString::FromAscii(buf0);
        AppendText(m_latest);
        if(oneshot)
            break;
    }
    if(m_proc->IsErrorAvailable())
    {
        wxTextAttr ta(wxColour(255,0,0));
        wxTextAttr oldta=GetDefaultStyle();
        SetDefaultStyle(ta);
        while(m_proc->IsErrorAvailable())
        {
            char buf0[maxchars+1];
            for(int i=0;i<maxchars+1;i++)
                buf0[i]=0;
            m_estream->Read(buf0,maxchars);
            wxString m_latest=wxString::FromAscii(buf0);
            AppendText(m_latest);
            if(oneshot)
                break;
        }
        SetDefaultStyle(oldta);
    }


    wxTextAttr ta(wxColour(0,180,0));
    wxTextAttr oldta=GetDefaultStyle();
    SetDefaultStyle(ta);
    int lastline=GetNumberOfLines();
    wxRegEx re(linkregex,wxRE_ADVANCED|wxRE_NEWLINE);
    while(lineno<lastline)
    {
        int col=0;
        wxString text=GetLineText(lineno);
        wxString file;
        while(re.Matches(text))
        {
            size_t start,len;
            if(re.GetMatch(&start,&len,0))
            {
                size_t fstart, flen;
                if(re.GetMatch(&fstart,&flen,1))
                    file=text.Mid(fstart,flen);
                wxFileName f(file);
                if(f.FileExists())
                {
                    int changepos=XYToPosition(col+start,lineno);
                    SetStyle(changepos,changepos+len,ta);
                }
            }
            col+=start+len;
            text=text.Mid(start+len);
        }
        lineno++;
    }
    SetDefaultStyle(oldta);
}

void ShellTermCtrl::OnUserInput(wxKeyEvent& ke)
{
    if(m_dead)
    {
        ke.Skip();
        return;
    }
    char* kc1=new char[2];
    kc1[0]=ke.GetKeyCode()%256;
    kc1[1]=0;
    if(kc1[0]=='\r')
    {
        //cbMessageBox(_T("converting keystroke"));
        kc1[0]='\n';
    }
//    cbMessageBox(_T("key press: ")+wxString::FromAscii(kc1)+wxString::Format(_T(" keycode: %i"),ke.GetKeyCode()));
    wxChar kc2=ke.GetUnicodeKey();
    wxString buf(kc2);
    //kc1[0]=buf[0];
    m_ostream->Write(kc1,1);
//    m_proc->GetOutputStream()->Write(kc1,1);
//    cbMessageBox(_T("bytes written: ")+wxString::Format(_T("code: %u"),m_ostream->LastWrite()));
    AppendText(kc2);
    SetInsertionPointEnd();
}



/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// HyperLinking ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



void ShellTermCtrl::OnDClick(wxMouseEvent &e)
{
    wxTextCoord x,y;
    HitTest(e.GetPosition(),&x,&y);
    wxRegEx re(linkregex,wxRE_ADVANCED|wxRE_NEWLINE);
    wxString text=GetLineText(y);
    wxString file;
    long line;
    while(1)
    {
        if(!re.Matches(text))
            return;
        size_t start,len;
        re.GetMatch(&start,&len,0);
        if(static_cast<size_t>(x)<start)
            return;
        if(static_cast<size_t>(x)>start+len)
        {
            text=text.Mid(start+len);
            x-=start+len;
            continue;
        }
        if(re.GetMatch(&start,&len,1))
            file=text.Mid(start,len);
        else
            file=wxEmptyString;
        if(re.GetMatch(&start,&len,3))
            text.Mid(start,len).ToLong(&line);
        else
            line=0;
        break;
//        re.GetMatch(&start,&len,0);
//        cbMessageBox(wxString::Format(_T("match '%s'\nfile '%s'\nline '%i'"), text.Mid(start,len).c_str(), file.c_str(), line));
    }
    wxFileName f(file);
    if(f.FileExists())
    {
        cbEditor* ed = Manager::Get()->GetEditorManager()->Open(f.GetFullPath());
        if (ed)
        {
            ed->Show(true);
//            if (!ed->GetProjectFile())
//                ed->SetProjectFile(f.GetFullPath());
            ed->GotoLine(line - 1, false);
            if(line>0)
                if(!ed->HasBookmark(line - 1))
                    ed->ToggleBookmark(line -1);
        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////// ShellManager /////////////////////////////////////////////

BEGIN_EVENT_TABLE(ShellManager, wxPanel)
    EVT_CHAR(ShellManager::OnUserInput)
    EVT_TIMER(ID_SHELLPOLLTIMER, ShellManager::OnPollandSyncOutput)
    EVT_FLATNOTEBOOK_PAGE_CLOSING(ID_SHELLMGR, ShellManager::OnPageClosing)
END_EVENT_TABLE()


void ShellManager::OnPageClosing(wxFlatNotebookEvent& event)
{
    ShellTermCtrl* sh = GetPage(event.GetSelection());
    //    LOGSTREAM << wxString::Format(_T("OnPageClosing(): ed=%p, title=%s\n"), eb, eb ? eb->GetTitle().c_str() : _T(""));
    if (!QueryClose(sh))
        event.Veto();
//    event.Skip(); // allow others to process it too
}


bool ShellManager::QueryClose(ShellTermCtrl* sh)
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


long ShellManager::LaunchProcess(wxString processcmd, wxString name, int stderrmode)
{
    int id=wxNewId();
    ShellTermCtrl *shell=new ShellTermCtrl(m_nb,id,this,name);
    long procid=shell->LaunchProcess(processcmd,0);
    if(procid>0)
    {
        if(!m_synctimer.IsRunning())
            m_synctimer.Start(100);
    }
    else
    {
        cbMessageBox(_T("process launch failed."));
        delete shell;
        return -1;
    }
    m_nb->AddPage(shell,name);
    m_nb->SetSelection(m_nb->GetPageCount()-1);
    return procid;
}

ShellTermCtrl *ShellManager::GetPage(size_t i)
{
    return (ShellTermCtrl*)m_nb->GetPage(i);
}

ShellTermCtrl *ShellManager::GetPage(const wxString &name)
{
    for(int i=0;i<m_nb->GetPageCount();i++)
    {
        ShellTermCtrl *sh=GetPage(i);
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

size_t ShellManager::GetTermNum(ShellTermCtrl *term)
{
    for(int i=0;i<m_nb->GetPageCount();i++)
    {
        ShellTermCtrl *shell=GetPage(i);
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


void ShellManager::OnShellTerminate(ShellTermCtrl *term)
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
        GetPage(i)->ReadStream();
    }
}

void ShellManager::OnUserInput(wxKeyEvent& ke)
{
    ShellTermCtrl *sh=(ShellTermCtrl*)m_nb->GetCurrentPage();
    sh->OnUserInput(ke);
}

ShellManager::ShellManager(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxCLIP_CHILDREN)
{
    m_synctimer.SetOwner(this, ID_SHELLPOLLTIMER);
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    m_nb = new wxFlatNotebook(this, ID_SHELLMGR, wxDefaultPosition, wxDefaultSize, wxFNB_X_ON_TAB|wxNB_TOP);
    bs->Add(m_nb, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);
    SetSizer(bs);
}

ShellManager::~ShellManager()
{
    //dtor
    //All of the subwindows owned by this panel will be destroyed automatically
}

