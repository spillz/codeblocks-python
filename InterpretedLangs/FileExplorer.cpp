#include "FileExplorer.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <vector>
#include "il_globals.h"

int ID_FILETREE=wxNewId();
int ID_FILELOC=wxNewId();
int ID_FILEWILD=wxNewId();
int ID_SETLOC=wxNewId();

int ID_OPENINED=wxNewId();
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
int ID_FILEREFRESH=wxNewId();
int ID_FILEADDTOPROJECT=wxNewId();

class DirTraverseFind : public wxDirTraverser     {
public:
    DirTraverseFind(const wxString& wildcard) : m_files(), m_wildcard(wildcard) { }
    virtual wxDirTraverseResult OnFile(const wxString& filename)
    {
        if(WildCardListMatch(m_wildcard,filename))
            m_files.Add(filename);
        return wxDIR_CONTINUE;
    }
    virtual wxDirTraverseResult OnDir(const wxString& dirname)
    {
        if(WildCardListMatch(m_wildcard,dirname))
            m_files.Add(dirname);
        return wxDIR_CONTINUE;
    }
    wxArrayString& GetMatches() {return m_files;}
private:
    wxArrayString m_files;
    wxString m_wildcard;
};

BEGIN_EVENT_TABLE(FileTreeCtrl, wxTreeCtrl)
//    EVT_TREE_ITEM_ACTIVATED(ID_FILETREE, FileTreeCtrl::OnActivate)  //double click -
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(FileTreeCtrl, wxTreeCtrl)

FileTreeCtrl::FileTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style,
    const wxValidator& validator,
    const wxString& name)
    : wxTreeCtrl(parent,id,pos,size,style,validator,name) {}

FileTreeCtrl::FileTreeCtrl() { }

FileTreeCtrl::FileTreeCtrl(wxWindow *parent): wxTreeCtrl(parent) {}

FileTreeCtrl::~FileTreeCtrl()
{
}

int FileTreeCtrl::OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
{
    if(GetItemImage(item1)>GetItemImage(item2))
        return -1;
    if(GetItemImage(item1)<GetItemImage(item2))
        return 1;
    return (GetItemText(item1).CmpNoCase(GetItemText(item2)));
}

BEGIN_EVENT_TABLE(FileExplorer, wxPanel)
    EVT_TREE_BEGIN_DRAG(ID_FILETREE, FileExplorer::OnBeginDragTreeItem)
    EVT_TREE_END_DRAG(ID_FILETREE, FileExplorer::OnEndDragTreeItem)
    EVT_BUTTON(ID_FILE_UPBUTTON, FileExplorer::OnUpButton)
    EVT_MENU(ID_SETLOC, FileExplorer::OnSetLoc)
    EVT_MENU(ID_OPENINED, FileExplorer::OnOpenInEditor)
    EVT_MENU(ID_FILENEWFILE, FileExplorer::OnNewFile)
    EVT_MENU(ID_FILENEWFOLDER,FileExplorer::OnNewFolder)
    EVT_MENU(ID_FILECOPY,FileExplorer::OnCopy)
    EVT_MENU(ID_FILEDUP,FileExplorer::OnDuplicate)
    EVT_MENU(ID_FILEMOVE,FileExplorer::OnMove)
    EVT_MENU(ID_FILEDELETE,FileExplorer::OnDelete)
    EVT_MENU(ID_FILERENAME,FileExplorer::OnRename)
    EVT_MENU(ID_FILEEXPANDALL,FileExplorer::OnExpandAll)
    EVT_MENU(ID_FILESHOWHIDDEN,FileExplorer::OnShowHidden)
    EVT_MENU(ID_FILEREFRESH,FileExplorer::OnRefresh)
    EVT_MENU(ID_FILEADDTOPROJECT,FileExplorer::OnAddToProject)
    EVT_TREE_ITEM_EXPANDING(ID_FILETREE, FileExplorer::OnExpand)
    //EVT_TREE_ITEM_COLLAPSED(id, func) //delete the children
    EVT_TREE_ITEM_ACTIVATED(ID_FILETREE, FileExplorer::OnActivate)  //double click - open file / expand folder (the latter is a default just need event.skip)
    EVT_TREE_ITEM_MENU(ID_FILETREE, FileExplorer::OnRightClick) //right click open context menu -- interpreter actions, rename, delete, copy, properties, set as root etc
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
    m_Loc = new wxComboBox(this,ID_FILELOC,_T(""),wxDefaultPosition,wxDefaultSize,0,NULL,wxTE_PROCESS_ENTER|wxCB_DROPDOWN);
    m_WildCards = new wxComboBox(this,ID_FILEWILD,_T(""),wxDefaultPosition,wxDefaultSize,0,NULL,wxTE_PROCESS_ENTER|wxCB_DROPDOWN);
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

// find a file in the filesystem below a selected root
void FileExplorer::FindFile(const wxString &findfilename, const wxTreeItemId &ti)
{
    wxString path=GetFullPath(ti);

    wxDir dir(path);

    if (!dir.IsOpened())
    {
        // deal with the error here - wxDir would already log an error message
        // explaining the exact reason for the failure
        return;
    }
    wxString filename;
    int flags=wxDIR_FILES|wxDIR_DIRS;
    if(m_show_hidden)
        flags|=wxDIR_HIDDEN;

    DirTraverseFind dtf(findfilename);
    m_findmatchcount=dir.Traverse(dtf,wxEmptyString,flags);
    m_findmatch=dtf.GetMatches();
}

// focus the item in the tree.
void FileExplorer::FocusFile(const wxTreeItemId &ti)
{
    m_Tree->SetFocus();
    m_Tree->UnselectAll();
    m_Tree->SelectItem(ti);
    m_Tree->EnsureVisible(ti);
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

bool FileExplorer::AddTreeItems(const wxTreeItemId &ti)
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

    bool cont = dir.GetFirst(&filename,wxEmptyString,flags);
    while ( cont )
    {
        int itemstate=0;
        bool match=true;
        if(wxFileName(path,filename).DirExists()) //TODO: use static version instead, this only works by accident on win32 (always true)
            itemstate=fvsFolder;
        if(wxFileName(path,filename).FileExists())
        {
            wxFileName fn(path,filename);
#if wxCHECK_VERSION(2,8,0)
            if(fn.IsFileWritable())
                itemstate=fvsNormal;
            else
                itemstate=fvsReadOnly;
#else
            itemstate=fvsNormal; //file writeable only available from wx2.8 -- TODO: create win32/linux API calls for early versions?
#endif
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

wxString FileExplorer::GetFullPath(const wxTreeItemId &ti)
{
    if(!ti.IsOk())
        return wxEmptyString;
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
    int count=static_cast<int>(m_Loc->GetCount());
    cfg->Write(_T("FileExplorer/RootList/Len"), count);
    for(int i=0;i<count;i++)
    {
        wxString ref=wxString::Format(_T("FileExplorer/RootList/I%i"),i);
        cfg->Write(ref, m_Loc->GetString(i));
    }
    count=static_cast<int>(m_Loc->GetCount());
    cfg->Write(_T("FileExplorer/WildMask/Len"), count);
    for(int i=0;i<count;i++)
    {
        wxString ref=wxString::Format(_T("FileExplorer/WildMask/I%i"),i);
        cfg->Write(ref, m_WildCards->GetString(i));
    }
}

void FileExplorer::OnEnterWild(wxCommandEvent &event)
{
    wxString wild=m_WildCards->GetValue();
    Refresh(m_Tree->GetRootItem());
    m_WildCards->Insert(wild,0);
    if(m_WildCards->GetCount()>10)
        m_WildCards->Delete(10);
    m_WildCards->SetSelection(0);
}

void FileExplorer::OnChooseWild(wxCommandEvent &event)
{
    Refresh(m_Tree->GetRootItem());
    wxString wild=m_WildCards->GetValue();
    m_WildCards->Delete(m_WildCards->GetSelection());
    m_WildCards->Insert(wild,0);
//    event.Skip(true);
//    cbMessageBox(wild);
    m_WildCards->SetSelection(0);
}

void FileExplorer::OnEnterLoc(wxCommandEvent &event)
{
    wxString loc=m_Loc->GetValue();
    if(!SetRootFolder(loc))
        return;
    m_Loc->Insert(m_root,0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
    m_Loc->SetSelection(0);
}

void FileExplorer::OnChooseLoc(wxCommandEvent &event)
{
    wxString loc=m_Loc->GetValue();
    if(!SetRootFolder(loc))
    {
        return;
    }
    m_Loc->Delete(event.GetInt());
    m_Loc->Insert(m_root,0);
    m_Loc->SetSelection(0);
}

void FileExplorer::OnOpenInEditor(wxCommandEvent &event)
{
    wxFileName path(GetFullPath(m_selectti[0])); //SINGLE: m_Tree->GetSelection()
    wxString filename=path.GetFullPath();
    if(!path.FileExists())
        return;
    EditorManager* em = Manager::Get()->GetEditorManager();
    EditorBase* eb = em->IsOpen(filename);
    if (eb)
    {
        // open files just get activated
        eb->Activate();
        return;
    } else
    em->Open(filename);

}

void FileExplorer::OnActivate(wxTreeEvent &event)
{
    wxString filename=GetFullPath(event.GetItem());
    if(m_Tree->GetItemImage(event.GetItem())==fvsFolder)
    {
//        if(!SetRootFolder(filename)) // this causes crash...
//            return;
//        m_Loc->Insert(m_root,0);
//        if(m_Loc->GetCount()>10)
//            m_Loc->Delete(10);
//        return;
        return;
    }
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
    m_ticount=m_Tree->GetSelections(m_selectti);
    if(!IsInSelection(event.GetItem())) //replace the selection with right clicked item if right clicked item isn't in the selection
    {
        for(int i=0;i<m_ticount;i++)
            m_Tree->SelectItem(m_selectti[i],false);
        m_Tree->SelectItem(event.GetItem());
        m_ticount=m_Tree->GetSelections(m_selectti);
        m_Tree->Update();
    }
    wxString filename=m_Tree->GetItemText(m_selectti[0]);
    wxString filepath=GetFullPath(m_selectti[0]);
    int img = m_Tree->GetItemImage(m_selectti[0]);
    FileTreeData* ftd = new FileTreeData(0, FileTreeData::ftdkUndefined);
    ftd->SetKind(FileTreeData::ftdkFile);
    if(m_ticount>0)
    {
        if(m_ticount==1)
            if(img==fvsFolder)
            {
                ftd->SetKind(FileTreeData::ftdkFolder);
                m_Popup->Append(ID_SETLOC,_T("Make roo&t"));
                #ifndef __WXMSW__
        //        m_Popup->Append(ID_FILEEXPANDALL,_T("Expand All Children")); //TODO: check availability in wx2.8 for win32 (not avail wx2.6)
                #endif
                m_Popup->Append(ID_FILENEWFILE,_T("New File..."));
                m_Popup->Append(ID_FILENEWFOLDER,_T("Ma&ke Directory..."));
            } else
            {
                m_Popup->Append(ID_OPENINED,_T("&Open in CB Editor"));
                m_Popup->Append(ID_FILERENAME,_T("&Rename..."));
            }
        if(IsFilesOnly(m_selectti)&&Manager::Get()->GetProjectManager()->GetActiveProject())
            m_Popup->Append(ID_FILEADDTOPROJECT,_T("&Add to Active Project..."));
        m_Popup->Append(ID_FILEDUP,_T("&Duplicate"));
        m_Popup->Append(ID_FILECOPY,_T("&Copy To..."));
        m_Popup->Append(ID_FILEMOVE,_T("&Move To..."));
        m_Popup->Append(ID_FILEDELETE,_T("D&elete"));
    }
    m_Popup->AppendCheckItem(ID_FILESHOWHIDDEN,_T("Show &Hidden Files"))->Check(m_show_hidden);
    m_Popup->Append(ID_FILEREFRESH,_T("Re&fresh"));
    if(m_ticount>1)
    {
        ftd->SetKind(FileTreeData::ftdkVirtualGroup);
        wxString pathlist=GetFullPath(m_selectti[0]);
        for(int i=1;i<m_ticount;i++)
            pathlist+=_T("*")+GetFullPath(m_selectti[i]); //passing a '*' separated list of files/directories to any plugin takers
        ftd->SetFolder(pathlist);
    }
    else
        ftd->SetFolder(filepath);
    if(m_ticount>0)
        Manager::Get()->GetPluginManager()->AskPluginsForModuleMenu(mtUnknown, m_Popup, ftd);
    delete ftd;
    wxWindow::PopupMenu(m_Popup);

}

void FileExplorer::OnSetLoc(wxCommandEvent &event)
{
    wxString loc=GetFullPath(m_selectti[0]); //SINGLE: m_Tree->GetSelection()
    if(!SetRootFolder(loc))
        return;
    m_Loc->Insert(m_root,0);
    if(m_Loc->GetCount()>10)
        m_Loc->Delete(10);
}

void FileExplorer::OnNewFile(wxCommandEvent &event)
{
//    cbMessageBox(_T("Not Implemented"));
}

void FileExplorer::OnNewFolder(wxCommandEvent &event)
{
    wxString workingdir=GetFullPath(m_selectti[0]); //SINGLE: m_Tree->GetSelection()
    wxTextEntryDialog te(this,_T("New Directory Name: "));
    if(te.ShowModal()!=wxID_OK)
        return;
    wxString name=te.GetValue();
    wxFileName dir(workingdir);
    dir.Assign(dir.GetFullPath(),name);
    wxString mkd=dir.GetFullPath();
    if(!wxFileName::DirExists(mkd) &&!wxFileName::DirExists(mkd))
    {
        dir.Mkdir(mkd);
        Refresh(m_selectti[0]); //SINGLE: m_Tree->GetSelection()
    }
    else
        cbMessageBox(_T("File/Directory Already Exists with Name ")+name);
}

void FileExplorer::OnDuplicate(wxCommandEvent &event)
{
    for(int i=0;i<m_ticount;i++)
    {
        wxFileName path(GetFullPath(m_selectti[i]));  //SINGLE: m_Tree->GetSelection()
        if(wxFileName::FileExists(path.GetFullPath())||wxFileName::DirExists(path.GetFullPath()))
        {
            if(!PromptSaveOpenFile(_T("File is modified, press Yes to save before duplication, No to copy unsaved file or Cancel to skip file"),wxFileName(path)))
                continue;
            int j=1;
            wxString destpath(path.GetPathWithSep()+path.GetName()+wxString::Format(_T("(%i)"),j));
            if(path.GetExt()!=wxEmptyString)
                destpath+=_T(".")+path.GetExt();
            while(j<100 && (wxFileName::FileExists(destpath) || wxFileName::DirExists(destpath)))
            {
                j++;
                destpath=path.GetPathWithSep()+path.GetName()+wxString::Format(_T("(%i)"),j);
                if(path.GetExt()!=wxEmptyString)
                    destpath+=_T(".")+path.GetExt();
            }
            if(j==100)
            {
                cbMessageBox(_T("Too many copies of file or directory"));
                continue;
            }

#ifdef __WXMSW__
            wxArrayString output;
            wxString cmdline;
            if(wxFileName::FileExists(path.GetFullPath()))
                cmdline=_T("cmd /c copy /Y \"")+path.GetFullPath()+_T("\" \"")+destpath+_T("\"");
            else
                cmdline=_T("cmd /c xcopy /S/E/Y/H/I \"")+path.GetFullPath()+_T("\" \"")+destpath+_T("\"");
            int hresult=::wxExecute(cmdline,output,wxEXEC_SYNC);
#else
            wxString cmdline=_T("/bin/cp -r -b \"")+path.GetFullPath()+_T("\" \"")+destpath+_T("\"");
            int hresult=::wxExecute(cmdline,wxEXEC_SYNC);
#endif
            if(hresult)
                MessageBox(m_Tree,_T("Command '")+cmdline+_T("' failed with error ")+wxString::Format(_T("%i"),hresult));
        }
    }
    Refresh(m_Tree->GetRootItem()); //TODO: Can probably be more efficient than this
    //TODO: Reselect item in new location?? (what it outside root scope?)
}

void FileExplorer::OnCopy(wxCommandEvent &event)
{
    wxDirDialog dd(this,_T("Copy to"));
    dd.SetPath(GetFullPath(m_Tree->GetRootItem()));
    if(dd.ShowModal()==wxID_CANCEL)
        return;
    for(int i=0;i<m_ticount;i++)
    {
        wxString path(GetFullPath(m_selectti[i]));  //SINGLE: m_Tree->GetSelection()
        wxFileName destpath;
        destpath.Assign(dd.GetPath(),wxFileName(path).GetFullName());
        if(destpath.SameAs(path))
            continue;
        if(wxFileName::FileExists(path)||wxFileName::DirExists(path))
        {
            if(!PromptSaveOpenFile(_T("File is modified, press Yes to save before duplication, No to copy unsaved file or Cancel to skip file"),wxFileName(path)))
                continue;
#ifdef __WXMSW__
            wxArrayString output;
            wxString cmdline;
            if(wxFileName::FileExists(path))
                cmdline=_T("cmd /c copy /Y \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\"");
            else
                cmdline=_T("cmd /c xcopy /S/E/Y/H/I \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\"");
            int hresult=::wxExecute(cmdline,output,wxEXEC_SYNC);
#else
            int hresult=::wxExecute(_T("/bin/cp -r -b \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\""),wxEXEC_SYNC);
#endif
            if(hresult)
                MessageBox(m_Tree,_T("Copying '")+path+_T("' failed with error ")+wxString::Format(_T("%i"),hresult));
        }
    }
    Refresh(m_Tree->GetRootItem()); //TODO: Can probably be more efficient than this
    //TODO: Reselect item in new location?? (what it outside root scope?)
}

void FileExplorer::OnMove(wxCommandEvent &event)
{
    wxDirDialog dd(this,_T("Move to"));
    dd.SetPath(GetFullPath(m_Tree->GetRootItem()));
    if(dd.ShowModal()==wxID_CANCEL)
        return;
    for(int i=0;i<m_ticount;i++)
    {
        wxString path(GetFullPath(m_selectti[i]));  //SINGLE: m_Tree->GetSelection()
        wxFileName destpath;
        destpath.Assign(dd.GetPath(),wxFileName(path).GetFullName());
        if(destpath.SameAs(path))
            continue;
        if(wxFileName::FileExists(path)||wxFileName::DirExists(path))
        {
#ifdef __WXMSW__
            wxArrayString output;
            int hresult=::wxExecute(_T("cmd /c move /Y \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\""),output,wxEXEC_SYNC);
#else
            int hresult=::wxExecute(_T("/bin/mv -b \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\""),wxEXEC_SYNC);
#endif
            if(hresult)
                MessageBox(m_Tree,_T("Moving '")+path+_T("' failed with error ")+wxString::Format(_T("%i"),hresult));
        }
    }
    Refresh(m_Tree->GetRootItem()); //TODO: Can probably be more efficient than this
    //TODO: Reselect item in new location?? (what if outside root scope?)
}

void FileExplorer::OnDelete(wxCommandEvent &event)
{
    if(MessageBox(m_Tree,_T("Are you sure?"),_T("Delete"),wxYES_NO)!=wxID_YES)
        return;
    for(int i=0;i<m_ticount;i++)
    {
        wxString path(GetFullPath(m_selectti[i]));  //SINGLE: m_Tree->GetSelection()
        if(wxFileName::FileExists(path))
        {
            //        EditorManager* em = Manager::Get()->GetEditorManager();
            //        if(em->IsOpen(path))
            //        {
            //            cbMessageBox(_T("Close file ")+path.GetFullPath()+_T(" first"));
            //            return;
            //        }
            if(!::wxRemoveFile(path))
                MessageBox(m_Tree,_T("Delete file '")+path+_T("' failed"));
        } else
        if(wxFileName::DirExists(path))
        {
#ifdef __WXMSW__
            wxArrayString output;
            int hresult=::wxExecute(_T("cmd /c rmdir /S/Q \"")+path+_T("\""),output,wxEXEC_SYNC);
#else
            int hresult=::wxExecute(_T("/bin/rm -r -f \"")+path+_T("\""),wxEXEC_SYNC);
#endif
            if(hresult)
                MessageBox(m_Tree,_T("Delete directory '")+path+_T("' failed with error ")+wxString::Format(_T("%i"),hresult));
        }
    }
    Refresh(m_Tree->GetRootItem());
}

void FileExplorer::OnRename(wxCommandEvent &event)
{
    wxFileName path(GetFullPath(m_selectti[0]));  //SINGLE: m_Tree->GetSelection()
    if(path.FileExists())
    {
        EditorManager* em = Manager::Get()->GetEditorManager();
        if(em->IsOpen(path.GetFullPath()))
        {
            cbMessageBox(_T("Close file first"));
            return;
        }
        wxTextEntryDialog te(this,_T("New Name: "));
        if(te.ShowModal()==wxID_CANCEL)
            return;
        wxFileName destpath(path);
        destpath.SetFullName(te.GetValue());
        if(!::wxRenameFile(path.GetFullPath(),destpath.GetFullPath()))
            cbMessageBox(_T("Rename failed"));
    }
    Refresh(m_Tree->GetItemParent(m_selectti[0])); //SINGLE: m_Tree->GetSelection()
}

void FileExplorer::OnExpandAll(wxCommandEvent &event)
{
//    #ifndef __WXMSW__
////    m_Tree->ExpandAll(m_Tree->GetSelection());
//    #endif
}

void FileExplorer::OnShowHidden(wxCommandEvent &event)
{
    m_show_hidden=!m_show_hidden;
    Refresh(m_Tree->GetRootItem());
}

void FileExplorer::OnUpButton(wxCommandEvent &event)
{
    wxFileName loc(m_root);
    loc.RemoveLastDir();
    SetRootFolder(loc.GetFullPath()); //TODO: Check if this is always the root folder
}

void FileExplorer::OnRefresh(wxCommandEvent &event)
{
    if(m_Tree->GetItemImage(m_selectti[0])==fvsFolder) //SINGLE: m_Tree->GetSelection()
        Refresh(m_selectti[0]); //SINGLE: m_Tree->GetSelection()
    else
        Refresh(m_Tree->GetRootItem());
}

//TODO: Set copy cursor state if necessary
void FileExplorer::OnBeginDragTreeItem(wxTreeEvent &event)
{
//    SetCursor(wxCROSS_CURSOR);
//    if(IsInSelection(event.GetItem()))
//        return; // don't start a drag for an unselected item
    if(m_Tree->GetItemImage(event.GetItem())==fvsNormal||m_Tree->GetItemImage(event.GetItem())==fvsFolder)
        event.Allow();
//    m_dragtest=GetFullPath(event.GetItem());
    m_ticount=m_Tree->GetSelections(m_selectti);
}

bool FileExplorer::IsInSelection(const wxTreeItemId &ti)
{
    for(int i=0;i<m_ticount;i++)
        if(ti==m_selectti[i])
            return true;
    return false;
}

//TODO: End copy cursor state if necessary
void FileExplorer::OnEndDragTreeItem(wxTreeEvent &event)
{
//    SetCursor(wxCursor(wxCROSS_CURSOR));
//    cbMessageBox(_T("Dragged ")+m_dragtest+_T(" to ")+m_Tree->GetItemText(event.GetItem()));
    if(m_Tree->GetItemImage(event.GetItem())!=fvsFolder) //can only copy to folders
        return;
    for(int i=0;i<m_ticount;i++)
    {
        wxString path(GetFullPath(m_selectti[i]));
        wxFileName destpath;
        if(!event.GetItem().IsOk())
            return;
        destpath.Assign(GetFullPath(event.GetItem()),wxFileName(path).GetFullName());
        if(destpath.SameAs(path))
            continue;
        if(wxFileName::DirExists(path)||wxFileName::FileExists(path))
        {
            if(!::wxGetKeyState(WXK_CONTROL))
            {
                if(wxFileName::FileExists(path))
                    if(!PromptSaveOpenFile(_T("File is modified, press Yes to save before move, No to move unsaved file or Cancel to skip file"),wxFileName(path)))
                        continue;
#ifdef __WXMSW__
                wxArrayString output;
                int hresult=::wxExecute(_T("cmd /c move /Y \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\""),output,wxEXEC_SYNC);
#else
                int hresult=::wxExecute(_T("/bin/mv -b \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\""),wxEXEC_SYNC);
#endif
                if(hresult)
                    MessageBox(m_Tree,_T("Move directory '")+path+_T("' failed with error ")+wxString::Format(_T("%i"),hresult));
            } else
            {
                if(wxFileName::FileExists(path))
                    if(!PromptSaveOpenFile(_T("File is modified, press Yes to save before copy, No to copy unsaved file or Cancel to skip file"),wxFileName(path)))
                        continue;
#ifdef __WXMSW__
                wxArrayString output;
                wxString cmdline;
                if(wxFileName::FileExists(path))
                    cmdline=_T("cmd /c copy /Y \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\"");
                else
                    cmdline=_T("cmd /c xcopy /S/E/Y/H/I \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\"");
                int hresult=::wxExecute(cmdline,output,wxEXEC_SYNC);
#else
                int hresult=::wxExecute(_T("/bin/cp -r -b \"")+path+_T("\" \"")+destpath.GetFullPath()+_T("\""),wxEXEC_SYNC);
#endif
                if(hresult)
                    MessageBox(m_Tree,_T("Copy directory '")+path+_T("' failed with error ")+wxString::Format(_T("%i"),hresult));
            }
        }
//        if(!PromptSaveOpenFile(_T("File is modified, press \"Yes\" to save before move/copy, \"No\" to move/copy unsaved file or \"Cancel\" to abort the operation"),path)) //TODO: specify move or copy depending on whether CTRL held down
//            return;
    }
    Refresh(m_Tree->GetRootItem());
}


void FileExplorer::OnAddToProject(wxCommandEvent &event)
{
    wxArrayString files;
    wxString file;
    for(int i=0;i<m_ticount;i++)
    {
        file=GetFullPath(m_selectti[i]);
        if(wxFileName::FileExists(file))
            files.Add(file);
    }
    wxArrayInt prompt;
    Manager::Get()->GetProjectManager()->AddMultipleFilesToProject(files, NULL, prompt);
    Manager::Get()->GetProjectManager()->RebuildTree();
}

bool FileExplorer::IsFilesOnly(wxArrayTreeItemIds tis)
{
    for(size_t i=0;i<tis.GetCount();i++)
        if(m_Tree->GetItemImage(tis[i])==fvsFolder)
            return false;
    return true;
}
