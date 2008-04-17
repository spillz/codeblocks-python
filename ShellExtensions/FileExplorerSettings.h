#ifndef FILEEXPLORERSETTINGS_H
#define FILEEXPLORERSETTINGS_H

#include <wx/wx.h>

#include <wx/button.h>


class FavoriteDir
{
public:
    wxString alias;
    wxString path;
};

WX_DECLARE_OBJARRAY(FavoriteDir, FavoriteDirs);

///////////////////////////////////////////////////////////////////////////////
/// Class FileBrowserSettings
///////////////////////////////////////////////////////////////////////////////
class FileBrowserSettings : public wxDialog
{
    private:

    protected:
        wxListBox* idfavlist;
        wxButton* idnew;
        wxButton* iddelete;
        wxButton* idup;
        wxButton* iddown;
        wxTextCtrl* idalias;
        wxTextCtrl* idpath;
        wxButton* idbrowsepath;
        wxButton* idok;
        wxButton* idcancel;

        void New(wxCommandEvent &event);
        void Delete(wxCommandEvent &event);
        void OnUp(wxCommandEvent &event);
        void OnDown(wxCommandEvent &event);
        void ChangeSelection(wxCommandEvent &event);
        void NameChange(wxCommandEvent &event);
        void OnOk(wxCommandEvent &event);
        void OnBrowse(wxCommandEvent &event);

    public:
        int m_selected;
        FavoriteDirs m_favdirs;
        FileBrowserSettings( const FavoriteDirs &favdirs, wxWindow* parent, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 500,400 ), int style = wxTAB_TRAVERSAL );
    DECLARE_EVENT_TABLE()
};

#endif
