#ifndef INTERPRETERPROPERTIES_H
#define INTERPRETERPROPERTIES_H

//#include <vector>
//#include <stdlib.h>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h> // Code::Blocks SDK

#include <wx/dynarray.h>


//struct ShellCommandMenuRef
//{
//    ShellCommandMenuRef() {i=0; a=0;}
//    ShellCommandMenuRef(size_t interp, size_t act) {i=interp; a=act;}
//    size_t i;
////    size_t a;
//};

WX_DEFINE_ARRAY_INT(int, ShellCommandMenuVec);
//WX_DECLARE_OBJARRAY(ShellCommandMenuRef, ShellCommandMenuVec);

struct ShellCommand
{
    wxString name; // the action's name, e.g. "Run"
    wxString command; // the command, e.g. "$interpreter --run $file"
    wxString wdir; //working directory for the command to start in.
    wxString wildcards;
    wxString menu;
    int menupriority;
    wxString cmenu;
    int cmenupriority;
    wxString envvarset;
    wxString mode; //"W" for spawning in the shells dockable panel, "C" for the codeblocks console runner, anything else for external process
};

WX_DECLARE_OBJARRAY(ShellCommand, ShellCommandVec);

class CommandCollection
{
public:
    bool WriteConfig(); //TODO: pass handle to config manager
    bool ReadConfig(); // ditto
    bool ImportLegacyConfig();
    bool ExportConfig(const wxString &filename);
    bool ImportConfig(const wxString &filename);
    ShellCommandVec interps;
};


#endif // INTERPRETERPROPERTIES_H
