#include "ConfigDialog.h"
#include "PyPlugin.h"

#include <vector>

int ID_NEW=wxNewId();
int ID_COPY=wxNewId();
int ID_EDIT=wxNewId();
int ID_DELETE=wxNewId();
int ID_UP=wxNewId();
int ID_DOWN=wxNewId();
int ID_INTERP_LIST=wxNewId();


int ID_NAME=wxNewId();
int ID_EXEC=wxNewId();
int ID_BROWSE_EXEC=wxNewId();
int ID_EXT=wxNewId();
int ID_ACTIONS=wxNewId();
int ID_OK=wxNewId();
int ID_CANCEL=wxNewId();
int ID_DEBUGCMDLINE=wxNewId();


BEGIN_EVENT_TABLE(ConfigDialog, wxPanel)
//  EVT_BUTTON(ID_OK, ConfigDialog::OnApply)
//  EVT_BUTTON(ID_CANCEL, ConfigDialog::OnEditCancel)
END_EVENT_TABLE()

ConfigDialog::ConfigDialog(wxWindow* parent, PyPlugin* plugin) //: cbConfigurationPanel()
{
    m_plugin=plugin;

    cbConfigurationPanel::Create(parent, -1, wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);


///////////////////////////////////////////// Form Builder generated code
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );

	m_staticText1 = new wxStaticText( this, wxID_DEFAULT, wxT("Python Interpreter Settings"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_staticText1, 0, wxALL, 5 );

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText3 = new wxStaticText( this, wxID_DEFAULT, wxT("Interpreter Executable"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( m_staticText3, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_editexec = new wxTextCtrl( this, ID_EXEC, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( m_editexec, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_butbrowse = new wxButton( this, ID_BROWSE_EXEC, wxT("..."), wxDefaultPosition, wxSize( 30,-1 ), 0 );
	bSizer20->Add( m_butbrowse, 0, wxALIGN_CENTER_VERTICAL, 5 );

	bSizer18->Add( bSizer20, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText5 = new wxStaticText( this, wxID_DEFAULT, wxT("File Extensions (example: *.py;*.pyw)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_staticText5, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_editext = new wxTextCtrl( this, ID_EXT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_editext, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	bSizer18->Add( bSizer21, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText6 = new wxStaticText( this, wxID_DEFAULT, wxT("Debugger: Command Line"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_staticText6, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_debugcmdline = new wxTextCtrl( this, ID_DEBUGCMDLINE, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_debugcmdline, 1, wxALL, 5 );

	bSizer18->Add( bSizer14, 0, wxEXPAND, 5 );

	m_staticText4 = new wxStaticText( this, wxID_DEFAULT, wxT("Debugger: Input Commands and Output Regular Expressions (NOT IMPLEMENTED)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_staticText4, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

	m_interplist = new wxListBox( this, ID_INTERP_LIST, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE );
	bSizer10->Add( m_interplist, 1, wxEXPAND|wxALL, 5 );

	m_buttedit = new wxButton( this, ID_EDIT, wxT("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_buttedit, 0, wxALL, 5 );

	bSizer18->Add( bSizer10, 1, wxEXPAND, 5 );

	bSizer12->Add( bSizer18, 1, wxEXPAND, 5 );

	this->SetSizer( bSizer12 );
/////////////////////////////////////////////////////////////////////////////////
	bSizer12->SetSizeHints(this);

    ReadDialogItems();
}



ConfigDialog::~ConfigDialog()
{
  //dtor
}

void ConfigDialog::Edit(wxCommandEvent& event)
{
}

void ConfigDialog::OnApply()
{
    WriteDialogItems();
    m_plugin->UpdateConfig();
}


void ConfigDialog::NameChange(wxCommandEvent& event)
{
}

void ConfigDialog::ChangeSelection(wxCommandEvent& event)
{
}

// Updates the Dialog controls to the stored values for the current interpreter
void ConfigDialog::ReadDialogItems()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("PyPlugin"));
    m_debugcmdline->SetValue(cfg->Read(_T("debug_cmd_line"),_T(" -u -m pdb ")));
    m_editexec->SetValue(cfg->Read(_T("python_executable"),_T("python"))); //TODO: make default command platform specific
    m_editext->SetValue(cfg->Read(_T("python_file_extensions"),_T("*.py;*.pyc")));
}

// Retrieve configuration values from the dialog widgets and store them appropriately
void ConfigDialog::WriteDialogItems()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("PyPlugin"));
    cfg->Write(_T("debug_cmd_line"),m_debugcmdline->GetValue());
    cfg->Write(_T("python_executable"),m_editexec->GetValue());
    cfg->Write(_T("python_file_extensions"),m_editext->GetValue());
}


void ConfigDialog::New(wxCommandEvent &event)
{
}

void ConfigDialog::Copy(wxCommandEvent &event)
{
}


void ConfigDialog::Delete(wxCommandEvent &event)
{
}


void ConfigDialog::OnEditBrowseExec(wxCommandEvent &event)
{
    wxString interpextension=_T("*.*");
    wxFileDialog *fd=new wxFileDialog(NULL,_T("Choose the interpreter executable"),_T(""),_T(""),interpextension,wxOPEN|wxFILE_MUST_EXIST);
    if(fd->ShowModal()==wxID_OK)
    {
        m_editexec->SetValue(fd->GetPath());
    }
    delete fd;
}

