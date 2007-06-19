#include "InterpretedLangs.h"
#include "ConfigDialog.h"
#include <vector>

int ID_NEW=wxNewId();
int ID_COPY=wxNewId();
//int ID_EDIT=wxNewId();
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



//  EVT_UPDATE_UI( -1, ConfigDialog::UpdateUI)
//  EVT_LISTBOX(XRCID("lstHelp"), ConfigDialog::ListChange)
//  EVT_BUTTON(ID_EDIT, ConfigDialog::Edit)

BEGIN_EVENT_TABLE(ConfigDialog, wxPanel)
  EVT_BUTTON(ID_NEW, ConfigDialog::New)
  EVT_BUTTON(ID_COPY, ConfigDialog::Copy)
  EVT_BUTTON(ID_DELETE, ConfigDialog::Delete)
  EVT_BUTTON(ID_UP, ConfigDialog::OnUp)
  EVT_BUTTON(ID_DOWN, ConfigDialog::OnDown)
  EVT_BUTTON(ID_OK, ConfigDialog::OnEditOK)
  EVT_BUTTON(ID_CANCEL, ConfigDialog::OnEditCancel)
  EVT_BUTTON(ID_BROWSE_EXEC, ConfigDialog::OnEditBrowseExec)
  EVT_LISTBOX(ID_INTERP_LIST, ConfigDialog::ChangeSelection)
  EVT_TEXT(ID_NAME, ConfigDialog::NameChange)
END_EVENT_TABLE()

//ConfigDialog::ConfigDialog( wxWindow* parent, int id, wxPoint pos, wxSize size, int style ) : wxPanel( parent, id, pos, size, style )
ConfigDialog::ConfigDialog(wxWindow* parent, InterpretedLangs* plugin) //: cbConfigurationPanel()
{
    m_plugin=plugin;
    m_icperm=&(plugin->m_ic);
    m_ic.interps=plugin->m_ic.interps; //temporary interpreter properties for edit mode (existing properties are not overwritten until user presses APPLY/OK)

    cbConfigurationPanel::Create(parent, -1, wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

///////////////////////////////////////////// Form Builder generated code
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );

	m_staticText1 = new wxStaticText( this, wxID_DEFAULT, wxT("Known Interpreters"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_staticText1, 0, wxALL, 5 );

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );

	m_butnew = new wxButton( this, ID_NEW, wxT("&New"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butnew, 0, wxLEFT|wxRIGHT, 5 );

	m_butcopy = new wxButton( this, ID_COPY, wxT("&Copy"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butcopy, 0, wxLEFT|wxRIGHT, 5 );

	m_butdelete = new wxButton( this, ID_DELETE, wxT("D&elete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butdelete, 0, wxRIGHT|wxLEFT, 5 );

	bSizer11->Add( 0, 10, 1, wxALL, 5 );

	m_butup = new wxButton( this, ID_UP, wxT("&Up"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butup, 0, wxRIGHT|wxLEFT, 5 );

	m_down = new wxButton( this, ID_DOWN, wxT("&Down"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_down, 0, wxRIGHT|wxLEFT, 5 );

	bSizer14->Add( bSizer11, 0, wxTOP, 5 );

	m_interplist = new wxListBox( this, ID_INTERP_LIST, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE|wxTAB_TRAVERSAL );
	bSizer14->Add( m_interplist, 1, wxBOTTOM|wxEXPAND|wxRIGHT|wxTOP, 5 );

	bSizer12->Add( bSizer14, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText2 = new wxStaticText( this, wxID_DEFAULT, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer19->Add( m_staticText2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_editname = new wxTextCtrl( this, ID_NAME, wxT(""), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	bSizer19->Add( m_editname, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	bSizer18->Add( bSizer19, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText3 = new wxStaticText( this, wxID_DEFAULT, wxT("Interpreter Executable"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( m_staticText3, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_editexec = new wxTextCtrl( this, ID_EXEC, wxT(""), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	bSizer20->Add( m_editexec, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_butbrowse = new wxButton( this, ID_BROWSE_EXEC, wxT("..."), wxDefaultPosition, wxSize( 30,-1 ), 0 );
	bSizer20->Add( m_butbrowse, 0, wxALIGN_CENTER_VERTICAL, 5 );

	bSizer18->Add( bSizer20, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText5 = new wxStaticText( this, wxID_DEFAULT, wxT("File Extensions (example: *.py;*.pyc)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_staticText5, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_editext = new wxTextCtrl( this, ID_EXT, wxT(""), wxDefaultPosition, wxDefaultSize, 0|wxTAB_TRAVERSAL );
	bSizer21->Add( m_editext, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	bSizer18->Add( bSizer21, 0, wxEXPAND, 5 );

	m_staticText4 = new wxStaticText( this, wxID_DEFAULT, wxT("Actions string format: Name;Command;[W|C];WorkDir;EnvVarSet \nCommand line variables: $interpreter, $file, $dir, $path, $mpaths\nWorking directory variables: $dir, $parentdir\nYou may also use global and project variables"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_staticText4, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_editactions = new wxTextCtrl( this, ID_ACTIONS, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTAB_TRAVERSAL );
	bSizer18->Add( m_editactions, 1, wxALL|wxEXPAND, 5 );

	bSizer16->Add( bSizer18, 1, wxEXPAND, 5 );

	bSizer15->Add( bSizer16, 1, wxEXPAND, 5 );

	bSizer12->Add( bSizer15, 1, wxEXPAND, 5 );

	this->SetSizer( bSizer12 );

	bSizer12->SetSizeHints(this);

    m_activeinterp=0;
    for(unsigned int i=0;i<m_ic.interps.size();i++)
        m_interplist->Append(m_ic.interps[i].name);
    SetDialogItems();

}

ConfigDialog::~ConfigDialog()
{
  //dtor
}

void ConfigDialog::OnApply()
{
    GetDialogItems();
    m_icperm->interps=m_ic.interps;
    m_icperm->WriteConfig();
    m_plugin->UpdateMenu();
}

void ConfigDialog::NameChange(wxCommandEvent& event)
{
    if(m_ic.interps.size()>0)
        m_interplist->SetString(m_activeinterp, m_editname->GetValue());
}

void ConfigDialog::ChangeSelection(wxCommandEvent& event)
{
    if(m_ic.interps.size()>0)
    {
        GetDialogItems();
        m_activeinterp=m_interplist->GetSelection();
        SetDialogItems();
    }
}

// Updates the Dialog controls to the stored values for the current interpreter
void ConfigDialog::SetDialogItems()
{
    if(m_ic.interps.size()>0)
    {
        m_interplist->Select(m_activeinterp);
        Interpreter &interp=m_ic.interps[m_activeinterp];
        m_editname->SetValue(interp.name);
        m_editexec->SetValue(interp.exec);
        m_editext->SetValue(interp.extensions);
        wxString actionstring;
        for(unsigned int i=0;i<interp.actions.size();i++)
            actionstring+=interp.actions[i].name+_T(";")+interp.actions[i].command+_T(";")
            +interp.actions[i].mode+_T(";")+interp.actions[i].wdir+_T(";")+
            interp.actions[i].envvarset+_T("\n");
        m_editactions->SetValue(actionstring);
    }
}

// Retrieve configuration values from the dialog widgets and store them appropriately
void ConfigDialog::GetDialogItems()
{
    if(!m_ic.interps.size())
        return;
    Interpreter &interp=m_ic.interps[m_activeinterp];
    interp.name=m_editname->GetValue();
    interp.exec=m_editexec->GetValue();
    interp.extensions=m_editext->GetValue();
    wxString actions=m_editactions->GetValue();
    InterpreterAction act;
    interp.actions.clear();
    for(int i=0;actions!=_T("");i++)
    {
        wxString actionstring=actions.BeforeFirst('\n');
        act.name=actionstring.BeforeFirst(';');
        actionstring=actionstring.AfterFirst(';');
        act.command=actionstring.BeforeFirst(';');
        actionstring=actionstring.AfterFirst(';');
        act.mode=actionstring.BeforeFirst(';');
        actionstring=actionstring.AfterFirst(';');
        act.wdir=actionstring.BeforeFirst(';');
        actionstring=actionstring.AfterFirst(';');
        act.envvarset=actionstring.BeforeFirst(';');
        interp.actions.push_back(act);
        actions=actions.AfterFirst('\n');
    }
}

void ConfigDialog::New(wxCommandEvent &event)
{
    GetDialogItems();
    Interpreter interp;
    interp.name=_T("New Interpreter");

    // Add a default action
    InterpreterAction act;
    act.name=_T("Run");
    act.command=_T("$interpreter $file");
    act.mode=_T("W");
    act.wdir=_T("$parentdir");
    interp.actions.push_back(act);
    m_ic.interps.push_back(interp);

    m_activeinterp=m_ic.interps.size()-1;

    m_interplist->Insert(m_ic.interps[m_activeinterp].name,m_activeinterp);

    SetDialogItems();
}

void ConfigDialog::Copy(wxCommandEvent &event)
{
    GetDialogItems();
    if(m_ic.interps.size()>0)
    {
        Interpreter interp=m_ic.interps[m_activeinterp];
        interp.name+=_T(" (Copy)");
        m_ic.interps.push_back(interp);

        m_activeinterp=m_ic.interps.size()-1;

        m_interplist->Insert(m_ic.interps[m_activeinterp].name,m_activeinterp);

        m_interplist->Select(m_activeinterp);
        SetDialogItems();
    }
}

void ConfigDialog::Delete(wxCommandEvent &event)
{
  if(m_ic.interps.size()>0)
      if (cbMessageBox(_("Are you sure you want to remove this interpreter?"), _("Remove"), wxICON_QUESTION | wxYES_NO, this) == wxID_YES)
      {
          GetDialogItems();
          m_ic.interps.erase(m_ic.interps.begin()+m_activeinterp);
          m_interplist->Delete(m_activeinterp);
          if(m_activeinterp>=m_ic.interps.size())
            m_activeinterp=m_ic.interps.size()-1;
          SetDialogItems();
      }
}

void ConfigDialog::OnUp(wxCommandEvent &event)
{
    if(m_activeinterp>0 && m_ic.interps.size()>1)
    {
        GetDialogItems();
        Interpreter interp=m_ic.interps[m_activeinterp];
        m_ic.interps.erase(m_ic.interps.begin()+m_activeinterp);
        m_interplist->Delete(m_activeinterp);
        m_activeinterp--;
        m_ic.interps.insert(m_ic.interps.begin()+m_activeinterp,interp);
        m_interplist->Insert(interp.name,m_activeinterp);
        m_interplist->Select(m_activeinterp);
    }
}

void ConfigDialog::OnDown(wxCommandEvent &event)
{
    if(m_activeinterp+1<m_ic.interps.size() && m_ic.interps.size()>1)
    {
        GetDialogItems();
        Interpreter interp=m_ic.interps[m_activeinterp];
        m_ic.interps.erase(m_ic.interps.begin()+m_activeinterp);
        m_interplist->Delete(m_activeinterp);
        m_activeinterp++;
        m_ic.interps.insert(m_ic.interps.begin()+m_activeinterp,interp);
        m_interplist->Insert(interp.name,m_activeinterp);
        m_interplist->Select(m_activeinterp);
    }
}

void ConfigDialog::OnEditOK(wxCommandEvent &event)
{
}

void ConfigDialog::OnEditCancel(wxCommandEvent &event)
{
}

void ConfigDialog::OnEditBrowseExec(wxCommandEvent &event)
{
    #ifdef __WXMSW__
    wxString interpextension=_T("*.*");
    #else
    wxString interpextension=_T("*");
    #endif
    wxFileDialog *fd=new wxFileDialog(NULL,_T("Choose the interpreter executable"),_T(""),_T(""),interpextension,wxOPEN|wxFILE_MUST_EXIST);
    if(fd->ShowModal()==wxID_OK)
    {
        m_editexec->SetValue(fd->GetPath());
    }
    delete fd;
}

