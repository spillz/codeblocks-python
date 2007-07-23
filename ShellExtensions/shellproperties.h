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


struct ShellCommandMenuRef
{
    ShellCommandMenuRef() {i=0;a=0;}
//    ShellCommandMenuRef(const ShellCommandMenuRef &copy) {i=copy.i;a=copy.a;}
    ShellCommandMenuRef(size_t interp, size_t act) {i=interp; a=act;}
    size_t i;
    size_t a;
};

WX_DECLARE_OBJARRAY(ShellCommandMenuRef, ShellCommandMenuVec);

struct ShellCommandAction
{
//    ShellCommandAction() {}
//    ShellCommandAction(const ShellCommandAction &copy) {name=copy.name;command=copy.command;mode=copy.mode;wdir=copy.wdir;envvarset=copy.envvarset;}
    wxString name; // the action's name, e.g. "Run"
    wxString command; // the command, e.g. "$interpreter --run $file"
    wxString mode; //"W" for spawning in the shells dockable panel, "C" for the codeblocks console runner, anything else for external process
    wxString wdir; //working directory for the command to start in.
    wxString envvarset;
};

WX_DECLARE_OBJARRAY(ShellCommandAction, ShellCommandActionVec);


struct ShellCommand
{
//    ShellCommand() {}
//    ShellCommand(const ShellCommand &copy) {name=copy.name;exec=copy.exec;extensions=copy.extensions;actions=copy.actions;}
   wxString name; // a friendly name for the interpreter
   wxString exec; // the full path to the interpreter's executable
   wxString extensions; // a semicolon-separated list of extension wildcards (e.g. "*.py;*.pyc")
   ShellCommandActionVec actions; // an array of possible actions with this interpreter
};

WX_DECLARE_OBJARRAY(ShellCommand, ShellCommandVec);

class CommandCollection
{
public:
//    CommandCollection() {}
//    CommandCollection(const CommandCollection &copy) {interps=copy.interps;}
//    CommandCollection();
//    virtual ~CommandCollection();
//    virtual ~CommandCollection();
    bool WriteConfig(); //TODO: pass handle to config manager
    bool ReadConfig(); // ditto
//    bool ConvertExtsWildCard(const wxString &seplist);
//    wxString ConvertExtsWildCard();
    ShellCommandVec interps;
};


#endif // INTERPRETERPROPERTIES_H
