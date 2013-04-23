#ifndef DIALOGS_H
#define DIALOGS_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h>

class PyDebugger;

class SendCommandDlg : public wxDialog
{
	private:

	public:
		wxStaticText* m_staticText8;
		wxTextCtrl* m_cmd;
		wxCheckBox* m_newline;
		wxButton* m_OK;
		wxButton* m_cancel;

		SendCommandDlg( wxWindow* parent, int id = -1, wxString title = wxT("Enter a Command"), wxPoint pos = wxDefaultPosition, wxSize size = wxSize( 600,100 ), int style = wxDEFAULT_DIALOG_STYLE );

};

#endif // DIALOGS_H
