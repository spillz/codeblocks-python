#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
    #include <wx/treectrl.h>
    #include <wx/combobox.h>
#endif

#include "sdk.h"


class Expansion;

typedef std::vector<Expansion*> ExpList;

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
        const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL|wxTR_HAS_BUTTONS|wxTR_MULTIPLE|wxTR_NO_LINES,         const wxValidator& validator = wxDefaultValidator, const wxString& name = _T("treeCtrl"))
        : wxTreeCtrl(parent,id,pos,size,style,validator,name) {}
private:
    int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
    {
        if(GetItemImage(item1)>GetItemImage(item2))
            return -1;
        if(GetItemImage(item1)<GetItemImage(item2))
            return 1;
        return (GetItemText(item1).Cmp(GetItemText(item2)));
    }
};


class FileExplorer: public wxPanel
{
public:
    FileExplorer(wxWindow *parent,wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL|wxTE_PROCESS_ENTER, const wxString& name = _T("Files"));
    ~FileExplorer() {}
    bool SetRootFolder(const wxString &folder);
    wxString GetRootFolder() {return m_root;}
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
    void OnNewFolder(wxCommandEvent &event);
    void OnCopy(wxCommandEvent &event);
    void OnDuplicate(wxCommandEvent &event);
    void OnMove(wxCommandEvent &event);
    void OnDelete(wxCommandEvent &event);
    void OnRename(wxCommandEvent &event);
    void OnExpandAll(wxCommandEvent &event);
    void OnShowHidden(wxCommandEvent &event);


    bool AddTreeItems(wxTreeItemId ti);
    wxString GetFullPath(wxTreeItemId ti);
    void GetExpandedNodes(wxTreeItemId ti, Expansion *exp);
    void RecursiveRebuild(wxTreeItemId ti, Expansion *exp);
    void Refresh(wxTreeItemId ti);
    void SetImages();
    wxMenu *m_Popup; // the popup menu that displays on right clicks in the tree (and maybe loc in future??)
    wxString m_root;
    FileTreeCtrl *m_Tree; //the widget display the file tree from root defined by m_Loc
    wxComboBox *m_Loc; // the combo box maintaining a list of useful locations and the current location
    wxComboBox *m_WildCards; // the combo box maintaining a list of wildcard filters for files
    bool m_show_hidden;
    DECLARE_EVENT_TABLE()
};

//wxPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel")

//wxTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS, const wxValidator& validator = wxDefaultValidator, const wxString& name = "treeCtrl")

#endif // FILEEXPLORER_H

