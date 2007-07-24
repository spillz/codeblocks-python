#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
    #include <wx/treectrl.h>
    #include <wx/combobox.h>
#endif

#include <wx/dynarray.h>

#include <sdk.h>


class Expansion;

typedef std::vector<Expansion*> ExpList;

class FavoriteDir
{
public:
    wxString alias;
    wxString path;
};

WX_DECLARE_OBJARRAY(FavoriteDir, FavoriteDirs);

class VCSstate
{
public:
    int state;
    wxString path;
};

WX_DECLARE_OBJARRAY(VCSstate, VCSstatearray);


class Expansion
{
public:
    Expansion() { name = _T("");}
    ~Expansion() {for(size_t i=0;i<children.size();i++) delete children[i];}
    wxString name;
    ExpList children;
};


class FileTreeCtrl: public wxTreeCtrl
{
public: //wxTR_HIDE_ROOT|
    FileTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS|wxTR_MULTIPLE|wxTR_NO_LINES,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = _T("treeCtrl"));
    FileTreeCtrl();
    FileTreeCtrl(wxWindow *parent);
//    void OnActivate(wxTreeEvent &event);
    virtual ~FileTreeCtrl();
//    void SortChildren(const wxTreeItemId& ti);
protected:
    virtual int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2);
    DECLARE_DYNAMIC_CLASS(FileTreeCtrl)
    DECLARE_EVENT_TABLE()
};


class FileExplorer: public wxPanel
{
public:
    FileExplorer(wxWindow *parent,wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL|wxTE_PROCESS_ENTER, const wxString& name = _T("Files"));
    ~FileExplorer() { WriteConfig(); }
    bool SetRootFolder(wxString root);
    wxString GetRootFolder() {return m_root;}
    void FindFile(const wxString &file) {}
private:
    void OnRightClick(wxTreeEvent &event);
    void OnActivate(wxTreeEvent &event);
    void OnExpand(wxTreeEvent &event);
    void OnEnterLoc(wxCommandEvent &event);
    void OnEnterWild(wxCommandEvent &event);
    void OnChooseLoc(wxCommandEvent &event);
    void OnChooseWild(wxCommandEvent &event);
    void OnSetLoc(wxCommandEvent &event);
    void OnNewFile(wxCommandEvent &event);
    void OnOpenInEditor(wxCommandEvent &event);
    void OnNewFolder(wxCommandEvent &event);
    void OnAddFavorite(wxCommandEvent &event);
    void OnCopy(wxCommandEvent &event);
    void OnDuplicate(wxCommandEvent &event);
    void OnMove(wxCommandEvent &event);
    void OnDelete(wxCommandEvent &event);
    void OnRename(wxCommandEvent &event);
    void OnExpandAll(wxCommandEvent &event);
    void OnShowHidden(wxCommandEvent &event);
    void OnUpButton(wxCommandEvent &event);
    void OnRefresh(wxCommandEvent &event);
    void OnBeginDragTreeItem(wxTreeEvent &event);
    void OnEndDragTreeItem(wxTreeEvent &event);
    void OnAddToProject(wxCommandEvent &event);


    void WriteConfig();
    void ReadConfig();

    VCSstatearray ParseSVNstate(const wxString &path);
    bool ParseBZRstate(const wxString &path, VCSstatearray &sa);
    bool ParseHGstate(const wxString &path, VCSstatearray &sa);
    bool IsFilesOnly(wxArrayTreeItemIds tis);
    void FindFile(const wxString &findfilename, const wxTreeItemId &ti);
    void FocusFile(const wxTreeItemId &ti);
    bool IsInSelection(const wxTreeItemId &ti);
    bool AddTreeItems(const wxTreeItemId &ti);
    wxString GetFullPath(const wxTreeItemId &ti);
    void GetExpandedNodes(wxTreeItemId ti, Expansion *exp);
    void RecursiveRebuild(wxTreeItemId ti, Expansion *exp);
    void Refresh(wxTreeItemId ti);
    void SetImages();
    wxMenu *m_Popup; // the popup menu that displays on right clicks in the tree (and maybe loc in future??)
    wxString m_root;
    FileTreeCtrl *m_Tree; //the widget display the file tree from root defined by m_Loc
    wxComboBox *m_Loc; // the combo box maintaining a list of useful locations and the current location
    wxComboBox *m_WildCards; // the combo box maintaining a list of wildcard filters for files
    wxButton *m_UpButton;
    bool m_show_hidden;
    wxArrayTreeItemIds m_selectti; //contains selections after context menu is called up
    FavoriteDirs m_favdirs;
    int m_ticount; //number of selections
    wxString m_dragtest;
    size_t m_findmatchcount;
    wxArrayString m_findmatch;
    bool m_show_vcs_state;
    DECLARE_EVENT_TABLE()
};

//wxPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel")

//wxTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS, const wxValidator& validator = wxDefaultValidator, const wxString& name = "treeCtrl")

#endif // FILEEXPLORER_H

