#include "FileExplorerSettings.h"

///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

int ID_BROWSE=wxNewId();


BEGIN_EVENT_TABLE(FileBrowserSettings, wxPanel)
//    EVT_MENU(ID_NEW, FileBrowserSettings::OnBrowse)
//    EVT_MENU(ID_DELETE, FileBrowserSettings::OnDelete)
//    EVT_MENU(ID_EDIT, FileBrowserSettings::OnEdit)
//    EVT_MENU(ID_UP, FileBrowserSettings::OnUp)
//    EVT_MENU(ID_DOWN, FileBrowserSettings::OnDown)
END_EVENT_TABLE()


FileBrowserSettings::FileBrowserSettings( wxWindow* parent, int id, wxPoint pos, wxSize size, int style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );

	BookmarkedFolders = new wxStaticText( this, wxID_ANY, wxT("Bookmarked Folders"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( BookmarkedFolders, 0, wxALL, 5 );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );

	m_listBox2 = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	bSizer11->Add( m_listBox2, 1, wxALL|wxEXPAND, 0 );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );

	m_button1 = new wxButton( this, ID_NEW, wxT("New..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_button1, 0, wxALL, 0 );

	m_button2 = new wxButton( this, ID_EDIT, wxT("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_button2, 0, wxALL, 0 );

	m_button3 = new wxButton( this, ID_DELETE, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_button3, 0, wxALL, 0 );

	bSizer12->Add( 0, 0, 1, wxEXPAND, 0 );

	m_button4 = new wxButton( this, ID_UP, wxT("Up"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_button4, 0, wxALL, 0 );

	m_button5 = new wxButton( this, ID_DOWN, wxT("Down"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_button5, 0, wxALL, 0 );

	bSizer11->Add( bSizer12, 0, wxEXPAND, 5 );

	bSizer10->Add( bSizer11, 1, wxEXPAND, 5 );

	m_staticText10 = new wxStaticText( this, wxID_ANY, wxT("Other Settings"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_staticText10, 0, wxALL, 5 );

	m_ShowHiddenChk = new wxCheckBox( this, ID_SHOWHIDDEN, wxT("Show Hidden Files"), wxDefaultPosition, wxDefaultSize, 0 );

	bSizer10->Add( m_ShowHiddenChk, 0, wxALL, 5 );

	this->SetSizer( bSizer10 );
	this->Layout();
}

BEGIN_EVENT_TABLE(FileBrowserShortcuts, wxDialog)
//    EVT_MENU(ID_BROWSE, FileBrowserShortcuts::OnBrowse)
END_EVENT_TABLE()


FileBrowserShortcuts::FileBrowserShortcuts( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_staticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_Name = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_Name, 1, wxALL, 5 );

	bSizer13->Add( bSizer14, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );

	m_statictext12 = new wxStaticText( this, wxID_ANY, wxT("Path"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_statictext12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_Path = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_Path, 1, wxALL, 5 );

	m_button15 = new wxButton( this, ID_BROWSE, wxT("..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer15->Add( m_button15, 0, wxALIGN_CENTER_VERTICAL, 0 );

	bSizer13->Add( bSizer15, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );

	m_button16 = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer16->Add( m_button16, 0, wxALL, 5 );

	m_button17 = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer16->Add( m_button17, 0, wxALL, 5 );

	bSizer13->Add( bSizer16, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	this->SetSizer( bSizer13 );
	this->Layout();
}


