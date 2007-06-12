#include "FileExplorer.h"
#include <wx/dir.h>
#include <wx/filename.h>
int ID_FILETREE=wxNewId();
int ID_FILELOC=wxNewId();


BEGIN_EVENT_TABLE(FileExplorer, wxPanel)
    EVT_TREE_ITEM_EXPANDING(ID_FILETREE, FileExplorer::OnExpand)
    //EVT_TREE_ITEM_COLLAPSED(id, func) //delete the children
    //EVT_TREE_ITEM_ACTIVATED(id, func)  //double click - open file / expand folder (the latter is a default just need event.skip)
    //EVT_TREE_ITEM_RIGHT_CLICK(id, func) //right click open context menu -- interpreter actions, rename, delete, copy, properties, set as root etc
    //EVT_COMBOBOX(ID_FILELOC, FileExplorer::OnChooseLoc) //location selected from history of combo box - set as root
    //EVT_TEXT(ID_FILELOC, FileExplorer::OnLocChanging) //provide autotext hint for dir name in combo box
    EVT_TEXT_ENTER(ID_FILELOC, FileExplorer::OnEnterLoc) //location entered in combo box - set as root
END_EVENT_TABLE()


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

    SetImages();
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
    m_Tree->SetItemHasChildren(m_Tree->GetRootItem());

    return true;
//    return AddTreeItems(m_Tree->GetRootItem());

}

bool FileExplorer::AddTreeItems(wxTreeItemId ti)
{
    m_Tree->DeleteChildren(ti);
    wxFileName path(m_Tree->GetItemText(ti));
    wxFileName rpath(m_root);
    if(ti!=m_Tree->GetRootItem())
    {
        wxTreeItemId parent=m_Tree->GetItemParent(ti);
        while(parent!=m_Tree->GetRootItem())
        {
            path=m_Tree->GetItemText(parent)+wxFileName::GetPathSeparator()+path.GetFullPath();
            parent=m_Tree->GetItemParent(parent);
        }
        path=rpath.GetPathWithSep()+path.GetFullPath(); //TODO: fix bug - this path doesn't always resolve...
    } else
        path=m_root;

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
        int itemstate=0;
        if(wxFileName(path.GetFullPath(),filename).DirExists())
            itemstate=fvsFolder;
        if(wxFileName(path.GetFullPath(),filename).FileExists())
            itemstate=fvsNormal;
        wxTreeItemId newitem=m_Tree->AppendItem(ti,filename,itemstate);
        m_Tree->SetItemHasChildren(newitem,itemstate==fvsFolder);
        cont = dir.GetNext(&filename);
    }
    m_Tree->SortChildren(ti);
    return true;
}

void FileExplorer::SetImages()
{
    static const wxString imgs[] = {

        // NOTE: Keep in sync with FileVisualState in globals.h!

        // The following are related to (editable, source-) file states
        _T("file.png"),                  // fvsNormal
        _T("file-missing.png"),          // fvsMissing,
        _T("file-modified.png"),         // fvsModified,
        _T("file-readonly.png"),         // fvsReadOnly,

        // The following are related to version control systems (vc)
        _T("rc-file-added.png"),         // fvsVcAdded,
        _T("rc-file-conflict.png"),      // fvsVcConflict,
        _T("rc-file-missing.png"),       // fvsVcMissing,
        _T("rc-file-modified.png"),      // fvsVcModified,
        _T("rc-file-outofdate.png"),     // fvsVcOutOfDate,
        _T("rc-file-uptodate.png"),      // fvsVcUpToDate,
        _T("rc-file-requireslock.png"),  // fvsVcRequiresLock,
        _T("rc-file-external.png"),      // fvsVcExternal,
        _T("rc-file-gotlock.png"),       // fvsVcGotLock,
        _T("rc-file-lockstolen.png"),    // fvsVcLockStolen,
        _T("rc-file-mismatch.png"),      // fvsVcMismatch,
        _T("rc-file-noncontrolled.png"), // fvsVcNonControlled,

        // The following are related to C::B workspace/project/folder/virtual
        _T("workspace.png"),             // fvsWorkspace,         WorkspaceIconIndex()
        _T("workspace-readonly.png"),    // fvsWorkspaceReadOnly, WorkspaceIconIndex(true)
        _T("project.png"),               // fvsProject,           ProjectIconIndex()
        _T("project-readonly.png"),      // fvsProjectReadOnly,   ProjectIconIndex(true)
        _T("folder_open.png"),           // fvsFolder,            FolderIconIndex()
        _T("vfolder_open.png"),          // fvsVirtualFolder,     VirtualFolderIconIndex()

        wxEmptyString
    };
    wxBitmap bmp;
    wxImageList *m_pImages = new wxImageList(16, 16);
    wxString prefix = ConfigManager::ReadDataPath() + _T("/images/");

    int i = 0;
    while (!imgs[i].IsEmpty())
    {
//        cbMessageBox(wxString::Format(_T("%d: %s"), i, wxString(prefix + imgs[i]).c_str()));
        bmp = cbLoadBitmap(prefix + imgs[i], wxBITMAP_TYPE_PNG); // workspace
        m_pImages->Add(bmp);
        ++i;
    }
    m_Tree->SetImageList(m_pImages);

//    // make sure tree is not "frozen"
//    UnfreezeTree(true);
}


void FileExplorer::OnExpand(wxTreeEvent &event)
{
    AddTreeItems(event.GetItem());
}

//TODO: Save previous paths
void FileExplorer::OnEnterLoc(wxCommandEvent &event)
{
    SetRootFolder(m_Loc->GetValue());
    m_Loc->Insert(m_Loc->GetValue(),0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
}

void FileExplorer::OnChangeLoc(wxCommandEvent &event)
{
    SetRootFolder(m_Loc->GetValue());
    m_Loc->Delete(event.GetInt());
    m_Loc->Insert(m_Loc->GetValue(),0);
}
