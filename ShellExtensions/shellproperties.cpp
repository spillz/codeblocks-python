#include "shellproperties.h"

#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(ShellCommandMenuVec);
WX_DEFINE_OBJARRAY(ShellCommandActionVec);
WX_DEFINE_OBJARRAY(ShellCommandVec);

wxString istr0(int i)
{
    return wxString::Format(_T("%i"),i);
}

bool CommandCollection::WriteConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    //cfg->Clear();
    int len=interps.GetCount();
    cfg->Write(_T("InterpProps/numinterps"), len);
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        cfg->Write(_T("InterpProps/I")+istr+_T("/name"), interps[i].name);
        cfg->Write(_T("InterpProps/I")+istr+_T("/exec"), interps[i].exec);
        cfg->Write(_T("InterpProps/I")+istr+_T("/ext"), interps[i].extensions);
        int lenact = interps[i].actions.GetCount();
        cfg->Write(_T("InterpProps/I")+istr+_T("/numactions"), lenact);
        for(int j=0;j<lenact;j++)
        {
            wxString jstr=istr0(j);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/name"), interps[i].actions[j].name);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/command"), interps[i].actions[j].command);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/mode"), interps[i].actions[j].mode);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/workingdir"), interps[i].actions[j].wdir);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/envvarset"), interps[i].actions[j].envvarset);
        }
    }
    return true;
}


bool CommandCollection::ReadConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    int len;
    if(!cfg->Read(_T("InterpProps/numinterps"), &len))
    {
//        cbMessageBox(_T("Warning: couldn't read interpreter config data"));
        return false;
    }
    for(int i=0;i<len;i++)
    {
        ShellCommand interp;
        wxString istr=istr0(i);
        cfg->Read(_T("InterpProps/I")+istr+_T("/name"), &interp.name);
        cfg->Read(_T("InterpProps/I")+istr+_T("/exec"), &interp.exec);
        cfg->Read(_T("InterpProps/I")+istr+_T("/ext"), &interp.extensions);
        int lenact;
        cfg->Read(_T("InterpProps/I")+istr+_T("/numactions"), &lenact);
        for(int j=0;j<lenact;j++)
        {
            ShellCommandAction a;
            wxString jstr=istr0(j);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/name"), &a.name);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/command"), &a.command);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/mode"), &a.mode);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/workingdir"), &a.wdir);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/envvarset"), &a.envvarset);
            interp.actions.Add(a);
        }
        interps.Add(interp);
    }
    return true;
}

/*
bool CommandCollection::ConvertExtsWildCard(const wxString &seplist)
{
}

wxString CommandCollection::ConvertExtsWildCard()
{
}

CommandCollection::CommandCollection()
{
    //ctor
}

CommandCollection::~CommandCollection()
{
    //dtor
}
*/

