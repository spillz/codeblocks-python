#include "FileExplorer.h"
#include <wx/dir.h>
#include <wx/filename.h>

BEGIN_EVENT_TABLE(FileExplorer, wxPanel)
END_EVENT_TABLE()

int ID_FILETREE=wxNewId();
int ID_FILELOC=wxNewId();

FileExplorer::FileExplorer(wxWindow *parent,wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name):
    wxPanel(parent,id,pos,size,style, name)
{
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    m_Tree = new wxTreeCtrl(this, ID_FILETREE);
    m_Loc = new wxComboBox(this,ID_FILELOC);
    bs->Add(m_Loc, 0, wxEXPAND);
    bs->Add(m_Tree, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);

    m_root=_T("/");
    m_Loc->SetValue(m_root);
    SetRootFolder(m_root);

    SetSizer(bs);
}

bool FileExplorer::SetRootFolder(const wxString &root)
{
    wxDir dir(root);
    if (!dir.IsOpened())
    {
        // deal with the error here - wxDir would already log an error message
        // explaining the exact reason of the failure
        return false;
    }
    m_root=root;
    m_Tree->DeleteAllItems();
    m_Tree->AddRoot(m_root);

    return AddTreeItems(m_Tree->GetRootItem());

}

bool FileExplorer::AddTreeItems(wxTreeItemId ti)
{
    wxFileName path(m_Tree->GetItemText(ti));
    wxTreeItemId parent;
    if(ti!=m_Tree->GetRootItem())
    {
        do
        {
            parent=m_Tree->GetItemParent(ti);
            path.PrependDir(m_Tree->GetItemText(parent));
        } while(parent!=m_Tree->GetRootItem());
    }

    wxDir dir(path.GetFullPath());

    if (!dir.IsOpened())
    {
        // deal with the error here - wxDir would already log an error message
        // explaining the exact reason of the failure
        return false;
    }
    wxString filename;
    bool cont = dir.GetFirst(&filename);
    while ( cont )
    {
        m_Tree->AppendItem(ti,filename);
        cont = dir.GetNext(&filename);
    }
    return true;
}
