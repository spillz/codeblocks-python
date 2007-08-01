///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb  2 2007)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "ShellExtensions.h"
#include "CmdConfigDialog.h"

///////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(CmdConfigDialog, wxPanel)
  EVT_BUTTON(ID_NEW, CmdConfigDialog::New)
  EVT_BUTTON(ID_COPY, CmdConfigDialog::Copy)
  EVT_BUTTON(ID_DELETE, CmdConfigDialog::Delete)
  EVT_BUTTON(ID_UP, CmdConfigDialog::OnUp)
  EVT_BUTTON(ID_DOWN, CmdConfigDialog::OnDown)
  EVT_LISTBOX(ID_COMMANDLIST, CmdConfigDialog::ChangeSelection)
  EVT_TEXT(ID_COMMANDNAME, CmdConfigDialog::NameChange)
END_EVENT_TABLE()


CmdConfigDialog::CmdConfigDialog( wxWindow* parent, ShellExtensions* plugin)
{
    m_plugin=plugin;
    m_icperm=&(plugin->m_ic);
    m_ic.interps=plugin->m_ic.interps; //temporary interpreter properties for edit mode (existing properties are not overwritten until user presses APPLY/OK)

    cbConfigurationPanel::Create(parent, -1, wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxVERTICAL );

	m_staticText27 = new wxStaticText( this, wxID_ANY, wxT("Known Commands"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer43->Add( m_staticText27, 0, wxALL, 5 );

	m_commandlist = new wxListBox( this, ID_COMMANDLIST, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	bSizer43->Add( m_commandlist, 1, wxALL|wxEXPAND, 1 );

	bSizer40->Add( bSizer43, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );

	m_butnew = new wxButton( this, ID_NEW, wxT("New"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butnew, 0, wxLEFT|wxRIGHT, 5 );

	m_butcopy = new wxButton( this, ID_COPY, wxT("Copy"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butcopy, 0, wxLEFT|wxRIGHT, 5 );

	m_butdelete = new wxButton( this, ID_DELETE, wxT("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butdelete, 0, wxRIGHT|wxLEFT, 5 );

	bSizer11->Add( 0, 10, 1, wxALL, 5 );

	m_butup = new wxButton( this, ID_UP, wxT("Up"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butup, 0, wxRIGHT|wxLEFT, 5 );

	m_butdown = new wxButton( this, ID_DOWN, wxT("Down"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_butdown, 0, wxRIGHT|wxLEFT, 5 );

	bSizer40->Add( bSizer11, 0, wxEXPAND, 5 );

	bSizer20->Add( bSizer40, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText11 = new wxStaticText( this, wxID_ANY, wxT("Command Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_staticText11, 0, wxALIGN_CENTER|wxALL, 5 );

	m_commandname = new wxTextCtrl( this, ID_COMMANDNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_commandname, 2, wxALL, 1 );
	m_commandname->SetToolTip(_T("Set a name to easily identify the command in this list. The name is also displayed in the Shell Console notebook when the command is executed (if you select that option)"));
	bSizer20->Add( bSizer21, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText12 = new wxStaticText( this, wxID_ANY, wxT("Command Line:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer211->Add( m_staticText12, 0, wxALIGN_CENTER|wxALL, 5 );

	bSizer20->Add( bSizer211, 0, wxEXPAND, 5 );

	m_command = new wxTextCtrl( this, ID_COMMAND, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_command->SetToolTip(_T("This is the command line that will be executed by the Shell. The following variable substitions are accepted:\n\n$relfile,$file: respectively the relative and absolute name of a selected file\n$reldir, $dir: respectively the relative and absolute name of a selected directory\n$relpath,$path: the relative and absolute name of the selected file or directory\n$mpaths: a list of selected files or directories (absolute paths only)\n$fname,$fext: the name without extension and the extension without name of a selected file\n$inputstr{prompt}: prompts the user to enter a string of text which is subsituted into the command line\n\nRight clicking on a file, directory or multiple paths in the Project Tree, File Explorer or Editor Pane will only populate if this command handles that type of object.\nTo use relative path names make sure you set the working directory appropriately (typically use $parentdir)\nYou can also use global, project and codeblocks special variables"));
	bSizer20->Add( m_command, 0, wxALL|wxEXPAND, 1 );

	wxBoxSizer* bSizer214;
	bSizer214 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText28 = new wxStaticText( this, wxID_ANY, wxT("Wildcards:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer214->Add( m_staticText28, 0, wxALIGN_CENTER|wxALL, 5 );

	m_wildcards = new wxTextCtrl( this, ID_WILDCARDS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_wildcards->SetToolTip(_T("The context menu will only be populated with this command if the file or directory selected matches the semi-colon separated list of wildcards. For example you could specify \"*.cpp;*.h;makefile.*;Makefile.*\" to handle C++ sources, headers and makefiles. Leave this blank to handle all file/directory types"));

	bSizer214->Add( m_wildcards, 1, wxALL, 1 );

	m_staticText112 = new wxStaticText( this, wxID_ANY, wxT("Working Directory:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer214->Add( m_staticText112, 0, wxALIGN_CENTER|wxALL, 5 );

	m_workdir = new wxTextCtrl( this, ID_WORKDIR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_workdir->SetToolTip(_T("This is the working directory for executing the command. Two context specific variables may be available:\n1. If you have specified $dir in the command line then you can use $dir here also.\n2. $parentdir is available for $relfile, $file, $reldir, $dir, $relpath, $path, $fname, $fext and is the absolute path of the directory containing the item.\nYou can also use codeblocks variables, project variables and global variables"));
	bSizer214->Add( m_workdir, 1, wxALL, 1 );

	bSizer20->Add( bSizer214, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer212;
	bSizer212 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText13 = new wxStaticText( this, wxID_ANY, wxT("Menu Location"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer212->Add( m_staticText13, 0, wxALIGN_CENTER|wxALL, 5 );

	m_menuloc = new wxTextCtrl( this, ID_MENULOC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_menuloc->SetToolTip(_T("This field controls the appearance of the command in the main \"Extensions\" menu.\nSpecify the nested structure as a path: for example submenu1/submenu2/itemname\nIf you leave itemname blank the command name will be used. If you specify a period as the first character of the field, the command will not be shown in the Extensions menu."));
	bSizer212->Add( m_menuloc, 1, wxALL, 1 );

	m_staticText16 = new wxStaticText( this, wxID_ANY, wxT("Priority"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer212->Add( m_staticText16, 0, wxALIGN_CENTER|wxALL, 5 );

	m_menulocpriority = new wxSpinCtrl( this, ID_MENULOCPRIORITY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
	bSizer212->Add( m_menulocpriority, 0, wxALL, 1 );

	bSizer20->Add( bSizer212, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer2121;
	bSizer2121 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText131 = new wxStaticText( this, wxID_ANY, wxT("Context Menu Location"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2121->Add( m_staticText131, 0, wxALIGN_CENTER|wxALL, 5 );

	m_cmenuloc = new wxTextCtrl( this, ID_CMENULOC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_cmenuloc->SetToolTip(_T("This field controls the appearance of the command in context menus offered when you right click files or directories in the File Explorer, files in the Project Manager and file in the Editor pane.\nSpecify the nested structure as a path: for example submenu1/submenu2/itemname\nIf you leave itemname blank the command name will be used. If you specify a period as the first character of the field, the command will not be shown in any context menu."));
	bSizer2121->Add( m_cmenuloc, 1, wxALL, 1 );

	m_staticText161 = new wxStaticText( this, wxID_ANY, wxT("Priority"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2121->Add( m_staticText161, 0, wxALIGN_CENTER|wxALL, 5 );

	m_cmenulocpriority = new wxSpinCtrl( this, ID_CMENULOCPRIORITY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
	bSizer2121->Add( m_cmenulocpriority, 0, wxALL, 1 );

	bSizer20->Add( bSizer2121, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer213;
	bSizer213 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText111 = new wxStaticText( this, wxID_ANY, wxT("Mode:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer213->Add( m_staticText111, 0, wxALIGN_CENTER|wxALL, 5 );

	wxString m_modeChoices[] = { wxT("Windowed Console Notepage"), wxT("Code::Blocks Console"), wxT("Standard Shell") };
	int m_modeNChoices = sizeof( m_modeChoices ) / sizeof( wxString );
	m_mode = new wxChoice( this, ID_MODE, wxDefaultPosition, wxDefaultSize, m_modeNChoices, m_modeChoices, 0 );
	m_mode->SetToolTip(_T("Select how the command is spawned:\n1. Windowed Console Notepage: redirects input and output to the Shell Extensions Dockable Notebook\n2. Code::Blocks Console: Runs as an external app in a terminal window, reports elapsed time and pause after execution.\n3. Standard Shell execution: will either spawn the command in a standard terminal window (win32), or spawn the command hidden (linux)."));
	bSizer213->Add( m_mode, 0, wxALL|wxEXPAND, 1 );

	m_staticText1111 = new wxStaticText( this, wxID_ANY, wxT("Env Vars:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer213->Add( m_staticText1111, 0, wxALIGN_CENTER|wxALL, 5 );

	wxString m_envvarsChoices[] = {  };
	int m_envvarsNChoices = sizeof( m_envvarsChoices ) / sizeof( wxString );
	m_envvars = new wxChoice( this, ID_ENVVARS, wxDefaultPosition, wxDefaultSize, m_envvarsNChoices, m_envvarsChoices, 0 );
	bSizer213->Add( m_envvars, 0, wxALL|wxEXPAND, 1 );

	bSizer20->Add( bSizer213, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer2131;
	bSizer2131 = new wxBoxSizer( wxHORIZONTAL );

	bSizer20->Add( bSizer2131, 0, wxEXPAND, 5 );

	this->SetSizer( bSizer20 );
//	this->Layout();
    m_activeinterp=0;
    for(unsigned int i=0;i<m_ic.interps.GetCount();i++)
        m_commandlist->Append(m_ic.interps[i].name);
    SetDialogItems();

}


void CmdConfigDialog::OnApply()
{
    GetDialogItems();
    m_icperm->interps=m_ic.interps;
    m_icperm->WriteConfig();
    m_plugin->UpdateMenu();
}

void CmdConfigDialog::NameChange(wxCommandEvent& event)
{
    if(m_ic.interps.GetCount()>0)
        m_commandlist->SetString(m_activeinterp, m_commandname->GetValue());
}

void CmdConfigDialog::ChangeSelection(wxCommandEvent& event)
{
    if(m_commandlist->GetSelection()>=0)
    {
        GetDialogItems();
        m_activeinterp=m_commandlist->GetSelection();
        SetDialogItems();
    }
}

// Updates the Dialog controls to the stored values for the current interpreter
void CmdConfigDialog::SetDialogItems()
{
    if(m_ic.interps.GetCount()>0&&m_activeinterp>=0&&m_activeinterp<static_cast<int>(m_ic.interps.GetCount()))
    {
        ShellCommand &interp=m_ic.interps[m_activeinterp];
        m_commandname->SetValue(interp.name);
        m_command->SetValue(interp.command);
        m_wildcards->SetValue(interp.wildcards);
        m_workdir->SetValue(interp.wdir);
        m_menuloc->SetValue(interp.menu);
        m_menulocpriority->SetValue(interp.menupriority);
        m_cmenuloc->SetValue(interp.cmenu);
        m_cmenulocpriority->SetValue(interp.cmenupriority);
        if(interp.mode==_T("W"))
            m_mode->SetSelection(0);
        else if(interp.mode==_T("C"))
            m_mode->SetSelection(1);
        else
            m_mode->SetSelection(2);
        m_envvars->SetSelection(m_envvars->FindString(interp.envvarset));
    } else
    {
        m_commandname->SetValue(_T(""));
        m_command->SetValue(_T(""));
        m_wildcards->SetValue(_T(""));
        m_workdir->SetValue(_T(""));
        m_menuloc->SetValue(_T(""));
        m_menulocpriority->SetValue(0);
        m_cmenuloc->SetValue(_T(""));
        m_cmenulocpriority->SetValue(0);
        m_mode->SetSelection(0);
        m_envvars->SetSelection(0);
    }
}

// Retrieve configuration values from the dialog widgets and store them appropriately
void CmdConfigDialog::GetDialogItems()
{
    if(!m_ic.interps.GetCount()||m_activeinterp<0||m_activeinterp>=static_cast<int>(m_ic.interps.GetCount()))
        return;
    ShellCommand &interp=m_ic.interps[m_activeinterp];
    interp.name=m_commandname->GetValue();
    interp.command=m_command->GetValue();
    interp.wildcards=m_wildcards->GetValue();
    interp.wdir=m_workdir->GetValue();
    interp.menu=m_menuloc->GetValue();
    interp.menupriority=m_menulocpriority->GetValue();
    interp.cmenu=m_cmenuloc->GetValue();
    interp.cmenupriority=m_cmenulocpriority->GetValue();
    switch(m_mode->GetSelection())
    {
        case 0:
            interp.mode=_T("W");
            break;
        case 1:
            interp.mode=_T("C");
            break;
        case 2:
            interp.mode=_T("");
            break;
    }
    interp.envvarset=m_envvars->GetStringSelection();
}

void CmdConfigDialog::New(wxCommandEvent &event)
{
    GetDialogItems();
    ShellCommand interp;
    interp.name=_T("New ShellCommand");
    m_ic.interps.Add(interp);

    m_activeinterp=m_ic.interps.GetCount()-1;

    m_commandlist->Insert(m_ic.interps[m_activeinterp].name,m_activeinterp);

    m_commandlist->SetSelection(m_activeinterp);
    SetDialogItems();
}

void CmdConfigDialog::Copy(wxCommandEvent &event)
{
    GetDialogItems();
    if(m_ic.interps.GetCount()>0)
    {
        ShellCommand interp=m_ic.interps[m_activeinterp];
        interp.name+=_T(" (Copy)");
        m_ic.interps.Add(interp);

        m_activeinterp=m_ic.interps.GetCount()-1;

        m_commandlist->Insert(m_ic.interps[m_activeinterp].name,m_activeinterp);

        m_commandlist->SetSelection(m_activeinterp);
        SetDialogItems();
    }
}

void CmdConfigDialog::Delete(wxCommandEvent &event)
{
    if(m_activeinterp>=0 && m_ic.interps.GetCount()>0)
//        if (cbMessageBox(_("Are you sure you want to remove this command?"), _("Remove"), wxICON_QUESTION | wxYES_NO) == wxID_YES)
        {
            m_ic.interps.RemoveAt(m_activeinterp);
            m_commandlist->Delete(m_activeinterp);
            if(m_activeinterp>=static_cast<int>(m_ic.interps.GetCount()))
                m_activeinterp=m_ic.interps.GetCount()-1;
            SetDialogItems();
            if(m_activeinterp>=0)
                m_commandlist->SetSelection(m_activeinterp);
        }
}

void CmdConfigDialog::OnUp(wxCommandEvent &event)
{
    if(m_activeinterp>0 && m_ic.interps.GetCount()>1)
    {
        GetDialogItems();
        ShellCommand interp=m_ic.interps[m_activeinterp];
        m_ic.interps.RemoveAt(m_activeinterp);
        m_commandlist->Delete(m_activeinterp);
        m_activeinterp--;
        m_ic.interps.Insert(interp,m_activeinterp);
        m_commandlist->Insert(interp.name,m_activeinterp);
        m_commandlist->Select(m_activeinterp);
    }
}

void CmdConfigDialog::OnDown(wxCommandEvent &event)
{
    if(m_activeinterp+1<static_cast<int>(m_ic.interps.GetCount()) && m_ic.interps.GetCount()>1)
    {
        GetDialogItems();
        ShellCommand interp=m_ic.interps[m_activeinterp];
        m_ic.interps.RemoveAt(m_activeinterp);
        m_commandlist->Delete(m_activeinterp);
        m_activeinterp++;
        m_ic.interps.Insert(interp,m_activeinterp);
        m_commandlist->Insert(interp.name,m_activeinterp);
        m_commandlist->Select(m_activeinterp);
    }
}

