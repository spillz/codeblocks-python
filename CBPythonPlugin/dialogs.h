#ifndef DIALOGS_H
#define DIALOGS_H

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <cbplugin.h> // for "class cbPlugin"
#include "configurationpanel.h"

class PyPlugin;

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



class DebuggerWatch : public wxPanel
{
    public:
        DebuggerWatch(wxWindow* parent, PyPlugin* debugger);
        virtual ~DebuggerWatch();
        wxTextCtrl *m_WatchText; // contains the watch variables and results
    protected:
        PyPlugin *m_debugger;
    private:

        DECLARE_EVENT_TABLE()
};


#endif // DIALOGS_H
