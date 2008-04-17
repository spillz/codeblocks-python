#include "FileExplorerSettings.h"

#include <sdk.h>
#ifndef CB_PRECOMP
  #include <wx/xrc/xmlres.h>
  #include <wx/checkbox.h>
  #include <wx/choice.h>

  #include "cbproject.h"
#endif


#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(FavoriteDirs);

///////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(FileBrowserSettings, wxDialog)
  EVT_BUTTON(XRCID("idnew"), FileBrowserSettings::New)
  EVT_BUTTON(XRCID("iddelete"), FileBrowserSettings::Delete)
  EVT_BUTTON(XRCID("idup"), FileBrowserSettings::OnUp)
  EVT_BUTTON(XRCID("iddown"), FileBrowserSettings::OnDown)
  EVT_BUTTON(XRCID("idbrowsepath"), FileBrowserSettings::OnBrowse)
  EVT_BUTTON(wxID_OK, FileBrowserSettings::OnOk)
//  EVT_BUTTON(XRCID("idcancel"), FileBrowserSettings::OnCancel)
  EVT_LISTBOX(XRCID("idfavlist"), FileBrowserSettings::ChangeSelection)
  EVT_TEXT(XRCID("idalias"), FileBrowserSettings::NameChange)
END_EVENT_TABLE()


FileBrowserSettings::FileBrowserSettings( const FavoriteDirs &favdirs, wxWindow* parent, int id, wxPoint pos, wxSize size, int style ) //: wxDialog( parent, id, _T("File Explorer Settings"), pos, size, style )
{
    wxXmlResource::Get()->LoadDialog(this,parent,_T("FileBrowserSettings"));
    idfavlist=XRCCTRL(*this,"idfavlist",wxListBox);
//    idnew=XRCCTRL(*this,"idnew",wxButton);
//    iddelete=XRCCTRL(*this,"iddelete",wxButton);
//    idup=XRCCTRL(*this,"idup",wxButton);
//    iddown=XRCCTRL(*this,"iddown",wxButton);
    idalias=XRCCTRL(*this,"idalias",wxTextCtrl);
    idpath=XRCCTRL(*this,"idpath",wxTextCtrl);
//    idbrowsepath=XRCCTRL(*this,"idbrowsepath",wxButton);
//    idok=XRCCTRL(*this,"idok",wxButton);
//    idcancel=XRCCTRL(*this,"idcancel",wxButton);

    m_favdirs=favdirs;
    for(size_t i=0;i<favdirs.GetCount();i++)
        idfavlist->Append(favdirs[i].alias);
    m_selected=-1;
    if(m_favdirs.GetCount()>0)
    {
        m_selected=0;
        idalias->SetValue(m_favdirs[0].alias);
        idpath->SetValue(m_favdirs[0].path);
    }
    idfavlist->SetSelection(m_selected);

//    GetSizer()->SetSizeHints(this);
    SetSize(wxSize(500,500));
//    Layout();
//    GetSizer()->Fit(this);

//    CentreOnParent();

}

void FileBrowserSettings::New(wxCommandEvent &event)
{
    FavoriteDir f;
    f.alias=_T("New Path");
    f.path=_T("");
    m_favdirs.Add(FavoriteDir());
    idfavlist->Append(f.alias);
    m_selected=idfavlist->GetCount()-1;
    idfavlist->SetSelection(m_selected);
    idalias->SetValue(f.alias);
    idpath->SetValue(f.path);
}

void FileBrowserSettings::Delete(wxCommandEvent &event)
{
    int i=idfavlist->GetSelection();
    m_favdirs.RemoveAt(i);
    idfavlist->Delete(i);
    if(i>=idfavlist->GetCount())
        i--;
    if(i>=0)
    {
        idfavlist->SetSelection(i);
        m_selected=i;
    }
    else
    {
        idfavlist->SetSelection(-1);
        m_selected=-1;
    }
    idalias->SetValue(m_favdirs[i].alias);
    idpath->SetValue(m_favdirs[i].path);
}

void FileBrowserSettings::OnUp(wxCommandEvent &event)
{
    int i=idfavlist->GetSelection();
    if(i<=0)
        return;
    m_favdirs[i].alias=idalias->GetValue();
    m_favdirs[i].path=idpath->GetValue();
    FavoriteDir fswap;
    fswap=m_favdirs[i];
    m_favdirs[i]=m_favdirs[i-1];
    m_favdirs[i-1]=fswap;
    idfavlist->SetString(i-1,m_favdirs[i-1].alias);
    idfavlist->SetString(i,m_favdirs[i].alias);
    idfavlist->SetSelection(i-1);
    m_selected=i-1;
}

void FileBrowserSettings::OnDown(wxCommandEvent &event)
{
    int i=idfavlist->GetSelection();
    if(i>=idfavlist->GetCount()-1||i<0)
        return;
    m_favdirs[i].alias=idalias->GetValue();
    m_favdirs[i].path=idpath->GetValue();
    FavoriteDir fswap;
    fswap=m_favdirs[i];
    m_favdirs[i]=m_favdirs[i+1];
    m_favdirs[i+1]=fswap;
    idfavlist->SetString(i+1,m_favdirs[i+1].alias);
    idfavlist->SetString(i,m_favdirs[i].alias);
    idfavlist->SetSelection(i+1);
    m_selected=i+1;
}

void FileBrowserSettings::ChangeSelection(wxCommandEvent &event)
{
    int i=idfavlist->GetSelection();
    if(i<0 || i>=idfavlist->GetCount())
        return;
    m_favdirs[m_selected].alias=idalias->GetValue();
    m_favdirs[m_selected].path=idpath->GetValue();
    idfavlist->SetString(i-1,m_favdirs[i-1].alias);
    idfavlist->SetString(i,m_favdirs[i].alias);
    m_selected=i;
    idalias->SetValue(m_favdirs[i].alias);
    idpath->SetValue(m_favdirs[i].path);
}

void FileBrowserSettings::NameChange(wxCommandEvent &event)
{
    if(m_selected<0 || m_selected>=idfavlist->GetCount())
        return;
    idfavlist->SetString(m_selected,idalias->GetValue());
}


void FileBrowserSettings::OnOk(wxCommandEvent &event)
{
    m_favdirs[m_selected].alias=idalias->GetValue();
    m_favdirs[m_selected].path=idpath->GetValue();
    EndModal(wxID_OK);
}

void FileBrowserSettings::OnBrowse(wxCommandEvent &event)
{
    // todo: change to a dir picker
    wxDirDialog *dd=new wxDirDialog(NULL,_T("Choose a Directory"));
    dd->SetPath(idpath->GetValue());
    if(dd->ShowModal()==wxID_OK)
    {
        idpath->SetValue(dd->GetPath());
    }
    delete dd;
}
