#include "shellproperties.h"


wxString istr0(int i)
{
    return wxString::Format(_T("%i"),i);
}

bool CommandCollection::WriteConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    //cfg->Clear();
    int len=interps.size();
    cfg->Write(_T("InterpProps/numinterps"), len);
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        cfg->Write(_T("InterpProps/I")+istr+_T("/name"), interps[i].name);
        cfg->Write(_T("InterpProps/I")+istr+_T("/exec"), interps[i].exec);
        cfg->Write(_T("InterpProps/I")+istr+_T("/ext"), interps[i].extensions);
        int lenact = interps[i].actions.size();
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
    interps.resize(len);
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        cfg->Read(_T("InterpProps/I")+istr+_T("/name"), &interps[i].name);
        cfg->Read(_T("InterpProps/I")+istr+_T("/exec"), &interps[i].exec);
        cfg->Read(_T("InterpProps/I")+istr+_T("/ext"), &interps[i].extensions);
        int lenact;
        cfg->Read(_T("InterpProps/I")+istr+_T("/numactions"), &lenact);
        interps[i].actions.resize(lenact);
        for(int j=0;j<lenact;j++)
        {
            wxString jstr=istr0(j);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/name"), &interps[i].actions[j].name);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/command"), &interps[i].actions[j].command);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/mode"), &interps[i].actions[j].mode);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/workingdir"), &interps[i].actions[j].wdir);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/envvarset"), &interps[i].actions[j].envvarset);
        }
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

