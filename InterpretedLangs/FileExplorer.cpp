#include "FileExplorer.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <vector>
#include "il_globals.h"

int ID_FILETREE=wxNewId();
int ID_FILELOC=wxNewId();
int ID_FILEWILD=wxNewId();
int ID_SETLOC=wxNewId();

int ID_FILENEWFILE=wxNewId();
int ID_FILENEWFOLDER=wxNewId();
int ID_FILECOPY=wxNewId();
int ID_FILEDUP=wxNewId();
int ID_FILEMOVE=wxNewId();
int ID_FILEDELETE=wxNewId();
int ID_FILERENAME=wxNewId();
int ID_FILEEXPANDALL=wxNewId();
int ID_FILESHOWHIDDEN=wxNewId();
int ID_FILE_UPBUTTON=wxNewId();

BEGIN_EVENT_TABLE(FileExplorer, wxPanel)
    EVT_BUTTON(ID_FILE_UPBUTTON, FileExplorer::OnUpButton)
    EVT_MENU(ID_SETLOC, FileExplorer::OnSetLoc)
    EVT_MENU(ID_FILENEWFILE, FileExplorer::OnNewFile)
    EVT_MENU(ID_FILENEWFOLDER,FileExplorer::OnNewFolder)
    EVT_MENU(ID_FILECOPY,FileExplorer::OnCopy)
    EVT_MENU(ID_FILEDUP,FileExplorer::OnDuplicate)
    EVT_MENU(ID_FILEMOVE,FileExplorer::OnMove)
    EVT_MENU(ID_FILEDELETE,FileExplorer::OnDelete)
    EVT_MENU(ID_FILERENAME,FileExplorer::OnRename)
    EVT_MENU(ID_FILEEXPANDALL,FileExplorer::OnExpandAll)
    EVT_MENU(ID_FILESHOWHIDDEN,FileExplorer::OnShowHidden)
    EVT_TREE_ITEM_EXPANDING(ID_FILETREE, FileExplorer::OnExpand)
    //EVT_TREE_ITEM_COLLAPSED(id, func) //delete the children
    EVT_TREE_ITEM_ACTIVATED(ID_FILETREE, FileExplorer::OnActivate)  //double click - open file / expand folder (the latter is a default just need event.skip)
    EVT_TREE_ITEM_RIGHT_CLICK(ID_FILETREE, FileExplorer::OnRightClick) //right click open context menu -- interpreter actions, rename, delete, copy, properties, set as root etc
    EVT_COMBOBOX(ID_FILELOC, FileExplorer::OnChooseLoc) //location selected from history of combo box - set as root
    EVT_COMBOBOX(ID_FILEWILD, FileExplorer::OnChooseWild) //location selected from history of combo box - set as root
    //EVT_TEXT(ID_FILELOC, FileExplorer::OnLocChanging) //provide autotext hint for dir name in combo box
    EVT_TEXT_ENTER(ID_FILELOC, FileExplorer::OnEnterLoc) //location entered in combo box - set as root
    EVT_TEXT_ENTER(ID_FILEWILD, FileExplorer::OnEnterWild) //location entered in combo box - set as root  ** BUG RIDDEN
END_EVENT_TABLE()



FileExplorer::FileExplorer(wxWindow *parent,wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name):
    wxPanel(parent,id,pos,size,style, name)
{
    m_show_hidden=false;
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* bsh = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bshloc = new wxBoxSizer(wxHORIZONTAL);
    m_Tree = new FileTreeCtrl(this, ID_FILETREE);
    m_Tree->SetIndent(m_Tree->GetIndent()/2);
    m_Loc = new wxComboBox(this,ID_FILELOC,_T(""),wxDefaultPosition,wxDefaultSize,0,NULL,wxTE_PROCESS_ENTER);
    m_WildCards = new wxComboBox(this,ID_FILEWILD,_T(""),wxDefaultPosition,wxDefaultSize,0,NULL,wxTE_PROCESS_ENTER);
    m_UpButton = new wxButton(this,ID_FILE_UPBUTTON,_("^"),wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    bshloc->Add(m_Loc, 1, wxEXPAND);
    bshloc->Add(m_UpButton, 0, wxEXPAND);
    bs->Add(bshloc, 0, wxEXPAND);
    bsh->Add(new wxStaticText(this,wxID_ANY,_T("Wildcard: ")),0,wxALIGN_CENTRE);
    bsh->Add(m_WildCards,1);
    bs->Add(bsh, 0, wxEXPAND);
    bs->Add(m_Tree, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);

    SetImages();
    ReadConfig();
    if(m_Loc->GetCount()>0)
    {
        m_Loc->Select(0);
        m_root=m_Loc->GetString(0);
    } else
    {
        m_root=wxFileName::GetPathSeparator();
        m_Loc->Append(m_root);
        m_Loc->Select(0);
    }
    if(m_WildCards->GetCount()>0)
        m_WildCards->Select(0);
    SetRootFolder(m_root);

    SetSizer(bs);
}

bool FileExplorer::SetRootFolder(wxString root)
{
    if(root[root.Len()-1]!=wxFileName::GetPathSeparator())
        root=root+wxFileName::GetPathSeparator();
    wxDir dir(root);
    if (!dir.IsOpened())
    {
        // deal with the error here - wxDir would already log an error message
        // explaining the exact reason of the failure
        m_Loc->SetValue(m_root);
        return false;
    }
    m_root=root;
    m_Loc->SetValue(m_root);
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
    int flags=wxDIR_FILES|wxDIR_DIRS;
    if(m_show_hidden)
        flags|=wxDIR_HIDDEN;

    bool cont = dir.GetFirst(&filename,wxEmptyString,wxDIR_FILES|wxDIR_DIRS);
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
            if(!WildCardListMatch(wildcard,filename))
                match=false;
        }
        if(match)
        {
            wxTreeItemId newitem=m_Tree->AppendItem(ti,filename,itemstate);
            m_Tree->SetItemHasChildren(newitem,itemstate==fvsFolder);
        }
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

void FileExplorer::ReadConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    int len;
    cfg->Read(_T("FileExplorer/RootList/Len"), &len);
    for(int i=0;i<len;i++)
    {
        wxString ref=wxString::Format(_T("FileExplorer/RootList/I%i"),i);
        wxString loc;
        cfg->Read(ref, &loc);
        m_Loc->Append(loc);
    }
    cfg->Read(_T("FileExplorer/WildMask/Len"), &len);
    for(int i=0;i<len;i++)
    {
        wxString ref=wxString::Format(_T("FileExplorer/WildMask/I%i"),i);
        wxString wild;
        cfg->Read(ref, &wild);
        m_WildCards->Append(wild);
    }
}


void FileExplorer::WriteConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    //cfg->Clear();
    cfg->Write(_T("FileExplorer/RootList/Len"), m_Loc->GetCount());
    for(int i=0;i<m_Loc->GetCount();i++)
    {
        wxString ref=wxString::Format(_T("FileExplorer/RootList/I%i"),i);
        cfg->Write(ref, m_Loc->GetString(i));
    }
    cfg->Write(_T("FileExplorer/WildMask/Len"), m_Loc->GetCount());
    for(int i=0;i<m_WildCards->GetCount();i++)
    {
        wxString ref=wxString::Format(_T("FileExplorer/WildMask/I%i"),i);
        cfg->Write(ref, m_WildCards->GetString(i));
    }
}

//TODO: Save previous paths
void FileExplorer::OnEnterLoc(wxCommandEvent &event)
{
    wxString loc=m_Loc->GetValue();
    if(!SetRootFolder(loc))
        return;
    m_Loc->Insert(loc,0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
}

void FileExplorer::OnEnterWild(wxCommandEvent &event)
{
    wxString wild=m_WildCards->GetValue();
    Refresh(m_Tree->GetRootItem());
    m_WildCards->Insert(wild,0);
    if(m_WildCards->GetCount()>10)
        m_WildCards->Delete(10);
}

void FileExplorer::OnChooseWild(wxCommandEvent &event)
{
    Refresh(m_Tree->GetRootItem());
    wxString wild=m_WildCards->GetValue();
    m_WildCards->Delete(event.GetInt());
    m_WildCards->Insert(wild,0);
    m_WildCards->SetValue(wild);
}

void FileExplorer::OnChooseLoc(wxCommandEvent &event)
{
    wxString loc=m_Loc->GetValue();
    if(!SetRootFolder(loc))
    {
        return;
    }
    m_Loc->Delete(event.GetInt());
    m_Loc->Insert(loc,0);
    m_Loc->SetValue(loc);
//    Refresh(m_Tree->GetRootItem());
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
    wxString filename=m_Tree->GetItemText(event.GetItem());
    wxString filepath=GetFullPath(event.GetItem());
    int img = m_Tree->GetItemImage(event.GetItem());
    FileTreeData* ftd = new FileTreeData(0, FileTreeData::ftdkUndefined);
    ftd->SetKind(FileTreeData::ftdkFile);
    if(img==fvsFolder)
    {
        ftd->SetKind(FileTreeData::ftdkFolder);
        m_Popup->Append(ID_SETLOC,_T("Make root"));
        #ifndef __WXMSW__
        m_Popup->Append(ID_FILEEXPANDALL,_T("Expand All Children")); //not available win32 -- TODO: will have to implement manually
        #endif
        m_Popup->Append(ID_FILENEWFILE,_T("New File..."));
        m_Popup->Append(ID_FILENEWFOLDER,_T("Make Directory..."));
    }
    m_Popup->Append(ID_FILEDUP,_T("Duplicate"));
    m_Popup->Append(ID_FILECOPY,_T("Copy To..."));
    m_Popup->Append(ID_FILEMOVE,_T("Move To..."));
    m_Popup->Append(ID_FILERENAME,_T("Rename..."));
    m_Popup->Append(ID_FILEDELETE,_T("Delete"));
    m_Popup->AppendCheckItem(ID_FILESHOWHIDDEN,_T("Show Hidden Files"))->Check(m_show_hidden);
    ftd->SetFolder(filepath);

    Manager::Get()->GetPluginManager()->AskPluginsForModuleMenu(mtUnknown, m_Popup, ftd);
    delete ftd;
//    m_plugin->BuildModuleMenu(mtProjectManager, m_Popup, const FileTreeData* data);
    wxWindow::PopupMenu(m_Popup);

}

void FileExplorer::OnSetLoc(wxCommandEvent &event)
{
    wxString loc=GetFullPath(m_Tree->GetSelection());
    m_Loc->SetValue(loc);
    if(!SetRootFolder(m_Loc->GetValue()))
        return;
    m_Loc->Insert(m_Loc->GetValue(),0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
}

void FileExplorer::OnNewFile(wxCommandEvent &event)
{
    cbMessageBox(_T("Not Implemented"));
}

void FileExplorer::OnNewFolder(wxCommandEvent &event)
{
    wxString workingdir=GetFullPath(m_Tree->GetSelection());
    wxTextEntryDialog te(this,_T("New Directory Name: "));
    if(te.ShowModal()!=wxID_OK)
        return;
    wxString name=te.GetValue();
    wxFileName dir(workingdir);
    dir.Assign(dir.GetFullPath(),name);
    if(!dir.FileExists() && !dir.DirExists())
    {
        dir.Mkdir(dir.GetFullPath());
        Refresh(m_Tree->GetSelection());
    }
    else
        cbMessageBox(_T("File/Directory Already Exists"));
}

void FileExplorer::OnCopy(wxCommandEvent &event)
{
    cbMessageBox(_T("Not Implemented"));
}

void FileExplorer::OnDuplicate(wxCommandEvent &event)
{
    cbMessageBox(_T("Not Implemented"));
}

void FileExplorer::OnMove(wxCommandEvent &event)
{
    cbMessageBox(_T("Not Implemented"));
}

void FileExplorer::OnDelete(wxCommandEvent &event)
{
    wxTreeItemId parent=m_Tree->GetItemParent(m_Tree->GetSelection());
    wxFileName path(GetFullPath(m_Tree->GetSelection()));
    if(path.FileExists())
    {
        EditorManager* em = Manager::Get()->GetEditorManager();
        if(em->IsOpen(path.GetFullPath()))
        {
            cbMessageBox(_T("Close file first"));
            return;
        }
        if(cbMessageBox(_T("Are you sure?"),_T("Delete"),wxYES_NO)!=wxID_YES)
            return;
        if(!::wxRemoveFile(path.GetFullPath()))
            cbMessageBox(_T("Delete file failed"));
    }
    if(path.DirExists())
        if(!path.Rmdir())
            cbMessageBox(_T("Remove directory failed"));
    Refresh(parent);
}

void FileExplorer::OnRename(wxCommandEvent &event)
{
    wxFileName path(GetFullPath(m_Tree->GetSelection()));
    if(path.FileExists())
    {
        EditorManager* em = Manager::Get()->GetEditorManager();
        if(em->IsOpen(path.GetFullPath()))
        {
            cbMessageBox(_T("Close file first"));
            return;
        }
        wxTextEntryDialog te(this,_T("New Directory Name: "));
        if(te.ShowModal()==wxID_CANCEL)
            return;
        wxFileName destpath(path);
        destpath.SetFullName(te.GetValue());
        cbMessageBox(_T("Renaming ")+path.GetFullPath()+_T(" ")+destpath.GetFullPath());
        if(!::wxRenameFile(path.GetFullPath(),destpath.GetFullPath()))
            cbMessageBox(_T("Rename file failed"));
    }
    Refresh(m_Tree->GetItemParent(m_Tree->GetSelection()));
}

void FileExplorer::OnExpandAll(wxCommandEvent &event)
{
    #ifndef __WXMSW__
    m_Tree->ExpandAll(m_Tree->GetSelection());
    #endif
}

void FileExplorer::OnShowHidden(wxCommandEvent &event)
{
    m_show_hidden=!m_show_hidden;
    Refresh(m_Tree->GetRootItem());
}

void FileExplorer::OnUpButton(wxCommandEvent &event)
{
    wxFileName loc(m_Loc->GetValue());
    loc.RemoveLastDir();
    SetRootFolder(loc.GetFullPath()); //TODO: Check if this is always the root folder
}
