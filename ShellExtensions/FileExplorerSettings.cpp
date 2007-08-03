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
//  EVT_BUTTON(XRCID("idok"), FileBrowserSettings::OnOk)
//  EVT_BUTTON(XRCID("idcancel"), FileBrowserSettings::OnCancel)
  EVT_LISTBOX(XRCID("idfavlist"), FileBrowserSettings::ChangeSelection)
  EVT_TEXT(XRCID("idalias"), FileBrowserSettings::NameChange)
END_EVENT_TABLE()


FileBrowserSettings::FileBrowserSettings( const FavoriteDirs &favdirs, wxWindow* parent, int id, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, _T("File Explorer Settings"), pos, size, style )
{
    wxXmlResource::Get()->LoadDialog(this,parent,_T("FileBrowserSettings"));
    idfavlist=XRCCTRL(*this,"idfavlist",wxListBox);
    idnew=XRCCTRL(*this,"idnew",wxButton);
    iddelete=XRCCTRL(*this,"iddelete",wxButton);
    idup=XRCCTRL(*this,"idup",wxButton);
    iddown=XRCCTRL(*this,"iddown",wxButton);
    idalias=XRCCTRL(*this,"idalias",wxTextCtrl);
    idpath=XRCCTRL(*this,"idpath",wxTextCtrl);
    idbrowsepath=XRCCTRL(*this,"idbrowsepath",wxButton);
    idok=XRCCTRL(*this,"idok",wxButton);
    idcancel=XRCCTRL(*this,"idcancel",wxButton);

    m_favdirs=favdirs;
    for(size_t i=0;i<favdirs.GetCount();i++)
        idfavlist->Append(favdirs[i].alias);
}

void FileBrowserSettings::New(wxCommandEvent &event)
{
    FavoriteDir f;
    f.alias=_T("New Path");
    f.path=_T("");
    m_favdirs.Add(FavoriteDir());
    idfavlist->Append(f.alias);
    idfavlist->SetSelection(idfavlist->GetCount()-1);
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
        idfavlist->SetSelection(i);
    idalias->SetValue(m_favdirs[i].alias);
    idpath->SetValue(m_favdirs[i].path);
}

void FileBrowserSettings::OnUp(wxCommandEvent &event)
{
}

void FileBrowserSettings::OnDown(wxCommandEvent &event)
{
}

void FileBrowserSettings::ChangeSelection(wxCommandEvent &event)
{
}

void FileBrowserSettings::NameChange(wxCommandEvent &event)
{
}



