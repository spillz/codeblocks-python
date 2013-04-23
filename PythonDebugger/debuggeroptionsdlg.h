/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef DEBUGGEROPTIONSDLG_H
#define DEBUGGEROPTIONSDLG_H

#include <debuggermanager.h>

class ConfigManagerWrapper;

class PyDebuggerConfiguration : public cbDebuggerConfiguration
{
    public:
        explicit PyDebuggerConfiguration(const ConfigManagerWrapper &config);

        virtual cbDebuggerConfiguration* Clone() const;
        virtual wxPanel* MakePanel(wxWindow *parent);
        virtual bool SaveChanges(wxPanel *panel);
    public:
        int GetState();
        wxString GetPrefix(int state);
        wxString GetInitCommands(int state);
        wxString GetCommandLine(int state);

};

#endif // DEBUGGEROPTIONSDLG_H
