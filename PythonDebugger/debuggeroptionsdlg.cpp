/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * $Revision: 8543 $
 * $Id: debuggeroptionsdlg.cpp 8543 2012-11-10 22:36:18Z thomasdenk $
 * $HeadURL: http://svn.berlios.de/svnroot/repos/codeblocks/trunk/src/plugins/debuggergdb/debuggeroptionsdlg.cpp $
 */

#include <sdk.h>
#include "debuggeroptionsdlg.h"
#ifndef CB_PRECOMP
    #include <wx/checkbox.h>
    #include <wx/choice.h>
//    #include <wx/filedlg.h>
    #include <wx/intl.h>
    #include <wx/radiobox.h>
//    #include <wx/spinctrl.h>
    #include <wx/textctrl.h>
    #include <wx/xrc/xmlres.h>

    #include <configmanager.h>
    #include <macrosmanager.h>
#endif


class DebuggerConfigurationPanel : public wxPanel
{
    public:
        DebuggerConfigurationPanel(PyDebuggerConfiguration *dbgcfg): m_dbgcfg(dbgcfg) {}
    protected:
        void OnChangeEngine(wxCommandEvent &event)
        {
            int state = XRCCTRL(*this, "DebugEngine",       wxRadioBox)->GetSelection();
            XRCCTRL(*this, "InitCommands",      wxTextCtrl)->ChangeValue(m_dbgcfg->GetInitCommands(state));
            XRCCTRL(*this, "CommandLine",       wxTextCtrl)->ChangeValue(m_dbgcfg->GetCommandLine(state));
        }
    private:
        PyDebuggerConfiguration *m_dbgcfg;
        DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(DebuggerConfigurationPanel, wxPanel)
    EVT_RADIOBOX(XRCID("DebugEngine"), DebuggerConfigurationPanel::OnChangeEngine)
END_EVENT_TABLE()

PyDebuggerConfiguration::PyDebuggerConfiguration(const ConfigManagerWrapper &config) : cbDebuggerConfiguration(config)
{
}

cbDebuggerConfiguration* PyDebuggerConfiguration::Clone() const
{
    return new PyDebuggerConfiguration(*this);
}

wxPanel* PyDebuggerConfiguration::MakePanel(wxWindow *parent)
{
    DebuggerConfigurationPanel *panel = new DebuggerConfigurationPanel(this);
    if (!wxXmlResource::Get()->LoadPanel(panel, parent, wxT("dlgPythonDebuggerOptions")))
        return panel;

    XRCCTRL(*panel, "DebugEngine",       wxRadioBox)->SetSelection(GetState());
    XRCCTRL(*panel, "InitCommands",      wxTextCtrl)->ChangeValue(GetInitCommands(GetState()));
    XRCCTRL(*panel, "CommandLine",       wxTextCtrl)->ChangeValue(GetCommandLine(GetState()));
    return panel;
}

bool PyDebuggerConfiguration::SaveChanges(wxPanel *panel)
{
    int sel = XRCCTRL(*panel, "DebugEngine",  wxRadioBox)->GetSelection();
    m_config.Write(wxT("active_engine"),      sel);
    wxString prefix = GetPrefix(sel);
    m_config.Write(prefix + wxT("command_line"),       XRCCTRL(*panel, "CommandLine",  wxTextCtrl)->GetValue());
    m_config.Write(prefix + wxT("init_commands"),      XRCCTRL(*panel, "InitCommands", wxTextCtrl)->GetValue());

    return true;
}

int PyDebuggerConfiguration::GetState()
{
    return m_config.ReadInt(wxT("active_engine"), 0);
}

wxString PyDebuggerConfiguration::GetPrefix(int state)
{
    if(state == 0)
        return wxT("pdb/");
    else
        return wxT("rpdb2/");
}

wxString PyDebuggerConfiguration::GetCommandLine(int state)
{
    wxString default_cmd = (state == 0) ?  wxT("python -u -m pdb $target") : wxT("python -u -m rpdb2 $target");
    wxString result = m_config.Read(GetPrefix(state)+wxT("command_line"), default_cmd);
    return result;
}


wxString PyDebuggerConfiguration::GetInitCommands(int state)
{
    return m_config.Read(GetPrefix(state) + wxT("init_commands"), wxEmptyString);
}
