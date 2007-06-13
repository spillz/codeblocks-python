#include "FileExplorer.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <vector>

int ID_FILETREE=wxNewId();
int ID_FILELOC=wxNewId();
int ID_FILEWILD=wxNewId();
int ID_SETLOC=wxNewId();

BEGIN_EVENT_TABLE(FileExplorer, wxPanel)
    EVT_MENU(ID_SETLOC, FileExplorer::OnSetLoc)
    EVT_TREE_ITEM_EXPANDING(ID_FILETREE, FileExplorer::OnExpand)
    //EVT_TREE_ITEM_COLLAPSED(id, func) //delete the children
    EVT_TREE_ITEM_ACTIVATED(ID_FILETREE, FileExplorer::OnActivate)  //double click - open file / expand folder (the latter is a default just need event.skip)
    EVT_TREE_ITEM_RIGHT_CLICK(ID_FILETREE, FileExplorer::OnRightClick) //right click open context menu -- interpreter actions, rename, delete, copy, properties, set as root etc
    //EVT_COMBOBOX(ID_FILELOC, FileExplorer::OnChooseLoc) //location selected from history of combo box - set as root
    //EVT_TEXT(ID_FILELOC, FileExplorer::OnLocChanging) //provide autotext hint for dir name in combo box
    EVT_TEXT_ENTER(ID_FILELOC, FileExplorer::OnEnterLoc) //location entered in combo box - set as root
    EVT_TEXT_ENTER(ID_FILEWILD, FileExplorer::OnEnterWild) //location entered in combo box - set as root  ** BUG RIDDEN
END_EVENT_TABLE()


FileExplorer::FileExplorer(wxWindow *parent,wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name):
    wxPanel(parent,id,pos,size,style, name)
{
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bsh = new wxBoxSizer(wxHORIZONTAL);
    m_Tree = new wxTreeCtrl(this, ID_FILETREE);
    m_Tree->SetIndent(2);
    m_Loc = new wxComboBox(this,ID_FILELOC,_T(""),wxDefaultPosition,wxDefaultSize,0,NULL,wxTE_PROCESS_ENTER);
    m_WildCards = new wxComboBox(this,ID_FILEWILD,_T(""),wxDefaultPosition,wxDefaultSize,0,NULL,wxTE_PROCESS_ENTER);
    bs->Add(m_Loc, 0, wxEXPAND);
    bsh->Add(new wxStaticText(this,wxID_ANY,_T("Wildcard: ")),0,wxALIGN_CENTRE);
    bsh->Add(m_WildCards,1);
    bs->Add(bsh, 0, wxEXPAND);
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
    m_Tree->AddRoot(m_root,fvsFolder);
    m_Tree->SetItemHasChildren(m_Tree->GetRootItem());
    m_Tree->Expand(m_Tree->GetRootItem());

    return true;
//    return AddTreeItems(m_Tree->GetRootItem());

}

void FileExplorer::GetExpandedNodes(wxTreeItemId ti, Expansion *exp)
{
    exp->name=m_Tree->GetItemText(ti);
    wxTreeItemIdValue cookie;
    wxTreeItemId ch=m_Tree->GetFirstChild(ti,cookie);
    while(ch.IsOk())
    {
        if(m_Tree->IsExpanded(ch))
        {
            Expansion *e=new Expansion();
            GetExpandedNodes(ch,e);
            exp->children.push_back(e);
        }
        ch=m_Tree->GetNextChild(ti,cookie);
    }
}


void FileExplorer::RecursiveRebuild(wxTreeItemId ti,Expansion *exp)
{
    AddTreeItems(ti);
    m_Tree->Expand(ti);
    if(exp->children.size()==0)
        return;
    wxTreeItemIdValue cookie;
    wxTreeItemId ch=m_Tree->GetFirstChild(ti,cookie);
    while(ch.IsOk())
    {
        for(size_t i=0;i<exp->children.size();i++)
            if(exp->children[i]->name==m_Tree->GetItemText(ch))
                RecursiveRebuild(ch,exp->children[i]);
        ch=m_Tree->GetNextChild(ti,cookie);
    }
}

void FileExplorer::Refresh(wxTreeItemId ti)
{
    Expansion e;
    GetExpandedNodes(ti,&e);
    RecursiveRebuild(ti,&e);
}

bool FileExplorer::AddTreeItems(wxTreeItemId ti)
{
    m_Tree->DeleteChildren(ti);
    wxString path=GetFullPath(ti);

    wxDir dir(path);

    if (!dir.IsOpened())
    {
        // deal with the error here - wxDir would already log an error message
        // explaining the exact reason for the failure
        return false;
    }
    wxString filename;
    bool cont = dir.GetFirst(&filename);
    while ( cont )
    {
        int itemstate=0;
        bool match=true;
        if(wxFileName(path,filename).DirExists())
            itemstate=fvsFolder;
        if(wxFileName(path,filename).FileExists())
        {
            itemstate=fvsNormal;
            wxString wildcard=m_WildCards->GetValue();
            if(wildcard!=_T("") && !::wxMatchWild(wildcard,filename,false))
                match=false;
        }
        if(match)
        {
            wxTreeItemId newitem=m_Tree->AppendItem(ti,filename,itemstate);
            m_Tree->SetItemHasChildren(newitem,itemstate==fvsFolder);
        }
        cont = dir.GetNext(&filename);
    }
//    m_Tree->SortChildren(ti);
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

wxString FileExplorer::GetFullPath(wxTreeItemId ti)
{
    wxFileName path(m_root);
    if(ti!=m_Tree->GetRootItem())
    {
        std::vector<wxTreeItemId> vti;
        vti.push_back(ti);
        wxTreeItemId pti=m_Tree->GetItemParent(vti[0]);
        while(pti!=m_Tree->GetRootItem())
        {
            vti.insert(vti.begin(),pti);
            pti=m_Tree->GetItemParent(pti);
        }
        for(size_t i=0;i<vti.size();i++)
            path.Assign(path.GetFullPath(),m_Tree->GetItemText(vti[i]));
    }
    return path.GetFullPath();
}

void FileExplorer::OnExpand(wxTreeEvent &event)
{
    AddTreeItems(event.GetItem());
}

//TODO: Save previous paths
void FileExplorer::OnEnterLoc(wxCommandEvent &event)
{
    if(!SetRootFolder(m_Loc->GetValue()))
        return;
    m_Loc->Insert(m_Loc->GetValue(),0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
}

void FileExplorer::OnEnterWild(wxCommandEvent &event)
{
    Refresh(m_Tree->GetRootItem());
    m_Loc->Insert(m_WildCards->GetValue(),0);
    if(m_WildCards->GetCount()>10)
        m_WildCards->Delete(10);
}


void FileExplorer::OnChangeLoc(wxCommandEvent &event)
{
    m_Loc->Delete(event.GetInt());
    if(SetRootFolder(m_Loc->GetValue()))
        m_Loc->Insert(m_Loc->GetValue(),0);
}

void FileExplorer::OnActivate(wxTreeEvent &event)
{
    wxString filename=GetFullPath(event.GetItem());
    EditorManager* em = Manager::Get()->GetEditorManager();
    EditorBase* eb = em->IsOpen(filename);
    if (eb)
    {
        // open files just get activated
        eb->Activate();
        return;
    }

    // Use Mime handler to open file
    cbMimePlugin* plugin = Manager::Get()->GetPluginManager()->GetMIMEHandlerForFile(filename);
    if (!plugin)
    {
        wxString msg;
        msg.Printf(_("Could not open file '%s'.\nNo handler registered for this type of file."), filename.c_str());
        LOG_ERROR(msg);
//        em->Open(filename); //should never need to open the file from here
    }
    else if (plugin->OpenFile(filename) != 0)
    {
        const PluginInfo* info = Manager::Get()->GetPluginManager()->GetPluginInfo(plugin);
        wxString msg;
        msg.Printf(_("Could not open file '%s'.\nThe registered handler (%s) could not open it."), filename.c_str(), info ? info->title.c_str() : wxString(_("<Unknown plugin>")).c_str());
        LOG_ERROR(msg);
    }

//    if(!em->IsOpen(file))
//        em->Open(file);

}

void FileExplorer::OnRightClick(wxTreeEvent &event)
{
    wxMenu *m_Popup=new wxMenu();
//    m_Popup->Append(wxID_ANY,_T("name"));
//    FileTreeData ftd;
//        void SetKind(FileTreeDataKind kind){ m_kind = kind; }
//        void SetProject(cbProject* project){ m_Project = project; }
//        // only valid for file selections
//        void SetFileIndex(int index){ m_Index = index; }
//        void SetProjectFile(ProjectFile* file){ m_file = file; }
//        // only valid for folder selections
//        void SetFolder(const wxString& folder){ m_folder = folder; }
    wxString filename=m_Tree->GetItemText(event.GetItem());
    wxString filepath=GetFullPath(event.GetItem());
    int img = m_Tree->GetItemImage(event.GetItem());
    FileTreeData* ftd = new FileTreeData(0, FileTreeData::ftdkUndefined);
    ftd->SetKind(FileTreeData::ftdkFile);
    if(img==fvsFolder)
    {
        m_Popup->Append(ID_SETLOC,_T("Make root"));
        ftd->SetKind(FileTreeData::ftdkFolder);
    }
    ftd->SetFolder(filepath);

    Manager::Get()->GetPluginManager()->AskPluginsForModuleMenu(mtUnknown, m_Popup, ftd);
    delete ftd;
//    m_plugin->BuildModuleMenu(mtProjectManager, m_Popup, const FileTreeData* data);
    wxWindow::PopupMenu(m_Popup);

}

void FileExplorer::OnSetLoc(wxCommandEvent &event)
{
    m_Loc->SetValue(GetFullPath(m_Tree->GetSelection()));
    if(!SetRootFolder(m_Loc->GetValue()))
        return;
    m_Loc->Insert(m_Loc->GetValue(),0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
}
