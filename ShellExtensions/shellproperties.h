#ifndef INTERPRETERPROPERTIES_H
#define INTERPRETERPROPERTIES_H

#include <vector>
#include <stdlib.h>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <sdk.h> // Code::Blocks SDK


struct ShellCommandMenuRef
{
    ShellCommandMenuRef(size_t interp, size_t act) {i=interp; a=act;}
    size_t i;
    size_t a;
};

typedef std::vector<ShellCommandMenuRef> ShellCommandMenuVec;


struct ShellCommandAction
{
   wxString name; // the action's name, e.g. "Run"
   wxString command; // the command, e.g. "$interpreter --run $file"
   wxString mode; //"W" for spawning in the shells dockable panel, "C" for the codeblocks console runner, anything else for external process
   wxString wdir; //working directory for the command to start in.
   wxString envvarset;
};

struct ShellCommand
{
   wxString name; // a friendly name for the interpreter
   wxString exec; // the full path to the interpreter's executable
   wxString extensions; // a semicolon-separated list of extension wildcards (e.g. "*.py;*.pyc")
   std::vector<ShellCommandAction> actions; // an array of possible actions with this interpreter
};

class CommandCollection
{
public:
//    CommandCollection();
//    virtual ~CommandCollection();
//    virtual ~CommandCollection();
    bool WriteConfig(); //TODO: pass handle to config manager
    bool ReadConfig(); // ditto
//    bool ConvertExtsWildCard(const wxString &seplist);
//    wxString ConvertExtsWildCard();
    std::vector<ShellCommand> interps;
};


#endif // INTERPRETERPROPERTIES_H
