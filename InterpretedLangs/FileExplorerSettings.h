///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef FILEEXPLORERSETTINGS_H
#define FILEEXPLORERSETTINGS_H

// Define WX_GCH in order to support precompiled headers with GCC compiler.
// You have to create the header "wx_pch.h" and include all files needed
// for compile your gui inside it.
// Then, compile it and place the file "wx_pch.h.gch" into the same
// directory that "wx_pch.h".
#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#include <wx/button.h>

///////////////////////////////////////////////////////////////////////////

#define ID_DEFAULT wxID_ANY // Default
#define ID_COPY 1000
#define ID_DELETE 1001
#define ID_UP 1002
#define ID_DOWN 1003
#define ID_INTERP_LIST 1004
#define ID_NAME 1005
#define ID_EXEC 1006
#define ID_BROWSE_EXEC 1007
#define ID_EXT 1008
#define ID_ACTIONS 1009
#define ID_NEW 1010
#define ID_EDIT 1011
#define ID_SHOWHIDDEN 1012

///////////////////////////////////////////////////////////////////////////////
/// Class FileBrowserSettings
///////////////////////////////////////////////////////////////////////////////
class FileBrowserSettings : public wxPanel
{
	private:

	protected:
		wxStaticText* BookmarkedFolders;
		wxListBox* m_listBox2;
		wxButton* m_button1;
		wxButton* m_button2;
		wxButton* m_button3;
		wxButton* m_button4;
		wxButton* m_button5;
		wxStaticText* m_staticText10;
		wxCheckBox* m_ShowHiddenChk;

	public:
		FileBrowserSettings( wxWindow* parent, int id = wxID_ANY, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 500,400 ), int style = wxTAB_TRAVERSAL );
    DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////
/// Class FileBrowserShortcuts
///////////////////////////////////////////////////////////////////////////////
class FileBrowserShortcuts : public wxDialog
{
	private:

	protected:
		wxStaticText* m_staticText11;
		wxTextCtrl* m_Name;
		wxStaticText* m_statictext12;
		wxTextCtrl* m_Path;
		wxButton* m_button15;
		wxButton* m_button16;
		wxButton* m_button17;

	public:
		FileBrowserShortcuts( wxWindow* parent, int id = wxID_ANY, wxString title = wxEmptyString, wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 500,400 ), int style = wxDEFAULT_DIALOG_STYLE );
    DECLARE_EVENT_TABLE()
};

#endif //FILEEXPLORERSETTINGS_H


