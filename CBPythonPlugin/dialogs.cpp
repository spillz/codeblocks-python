#include "dialogs.h"

int ID_CMD = wxNewId();
int ID_NEWLINE = wxNewId();

SendCommandDlg::SendCommandDlg( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) : wxDialog( parent, id, title, pos, size, style )
{
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText8 = new wxStaticText( this, wxID_DEFAULT, wxT("Command"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_staticText8, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_cmd = new wxTextCtrl( this, ID_CMD, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_cmd, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	bSizer9->Add( bSizer10, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );

	m_newline = new wxCheckBox( this, ID_NEWLINE, wxT("Insert Newline After Command"), wxDefaultPosition, wxDefaultSize, 0 );
	m_newline->SetValue(true);

	bSizer11->Add( m_newline, 0, wxALL, 5 );

	bSizer9->Add( bSizer11, 1, wxEXPAND, 5 );

	bSizer7->Add( bSizer9, 1, wxEXPAND|wxTOP|wxBOTTOM, 5 );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_OK = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_OK, 0, wxALIGN_BOTTOM, 5 );

	m_cancel = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_cancel, 0, wxALIGN_BOTTOM, 5 );

	bSizer12->Add( bSizer8, 1, wxBOTTOM, 5 );

	bSizer7->Add( bSizer12, 0, 0, 5 );

	this->SetSizer( bSizer7 );
	this->Layout();
}

//SendCommandDlg::~SendCommandDlg()
//{
//    //dtor
//}
//


BEGIN_EVENT_TABLE(DebuggerWatch, wxPanel)
END_EVENT_TABLE()

DebuggerWatch::DebuggerWatch(wxWindow* parent, PyPlugin* debugger)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxCLIP_CHILDREN),
    m_debugger(debugger)
{
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);
    m_WatchText = new wxTextCtrl(this, wxID_DEFAULT, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxHSCROLL);
        bs->Add(m_WatchText, 1, wxEXPAND | wxALL);
    SetAutoLayout(TRUE);
    SetSizer(bs);
}

DebuggerWatch::~DebuggerWatch()
{
    //dtor
}


