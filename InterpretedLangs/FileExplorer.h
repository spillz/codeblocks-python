#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
    #include <wx/treectrl.h>
    #include <wx/combobox.h>
#endif


class FileExplorer: public wxPanel
{
public:
    FileExplorer(wxWindow *parent,wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL, const wxString& name = _T("Files"));
    ~FileExplorer() {}
    bool SetRootFolder(const wxString &folder);
    wxString GetRootFolder() {return m_root;}

private:
    bool AddTreeItems(wxTreeItemId ti);
    wxString m_root;
    wxTreeCtrl *m_Tree; //the widget display the file tree from root defined by m_Loc
    wxComboBox *m_Loc; // the combo box maintaining a list of useful locations and the current location
    DECLARE_EVENT_TABLE()
};

//wxPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "panel")

//wxTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS, const wxValidator& validator = wxDefaultValidator, const wxString& name = "treeCtrl")

#endif // FILEEXPLORER_H

