#ifndef INTERPRETERPROPERTIES_H
#define INTERPRETERPROPERTIES_H

//#include "InterpretedLangs.h"
#include <vector>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif


struct InterpreterAction
{
   wxString name; // the action's name, e.g. "Run"
   wxString command; // the command, e.g. "$interpreter --run $file"
   wxString windowed; //"W" for spawning in the shells dockable panel, "E" for an external window
};

struct Interpreter
{
   wxString name; // a friendly name for the interpreter
   wxString exec; // the full path to the interpreter's executable
   wxString extensions; // a semicolon-separated list of extension wildcards (e.g. "*.py;*.pyc")
   std::vector<InterpreterAction> actions; // an array of possible actions with this interpreter
};

class InterpreterCollection
{
public:
//    InterpreterCollection();
//    virtual ~InterpreterCollection();
//    virtual ~InterpreterCollection();
    bool WriteConfig(); //TODO: pass handle to config manager
    bool ReadConfig(); // ditto
//    bool ConvertExtsWildCard(const wxString &seplist);
//    wxString ConvertExtsWildCard();
    std::vector<Interpreter> interps;
};


/*
class InterpreterProperties
{
    public:
        InterpreterProperties();
        virtual ~InterpreterProperties();
        bool WriteConfig(); //TODO: pass handle to config manager
        bool ReadConfig(); // ditto
        bool ConvertExtsWildCard(const wxString &seplist);
        wxString ConvertExtsWildCard();
//        bool ConvertExtsWildCard(const wxString &wildcardlist);

        wxArrayString m_name; // The displayable name of the interpreter
        wxArrayString m_cmd; // The command used to run the interpreter
        wxArrayString m_cmdlinepre; // Optional command line (before the run target)
        wxArrayString m_cmdlinepost; // Optional command line (after the run target)
        wxArrayString m_path; // The path to the interpreter (optional)
        wxArrayString m_fileexts;

    protected:
    private:
};
*/
#endif // INTERPRETERPROPERTIES_H

/*
ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("your_plugin_name"));
cfg->Write(_T("/any/kind/of/path"), some_string);
cfg->Write(_T("/paths/are/not/filesystem/paths"), some_float);
cfg->Write(_T("/paths/are/just/a/way/for/you/to/organize/data"), some_bool);

 some_string = cfg->Read(_T("/another/path"), a_default_string_if_the_key_doesnt_exist);
 some_int = cfg->ReadInt(_T("/always/start/the/paths/with/a/slash"), a default_int);
*/
