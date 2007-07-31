#include "shellproperties.h"

#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
//WX_DEFINE_OBJARRAY(ShellCommandMenuVec);
WX_DEFINE_OBJARRAY(ShellCommandVec);

wxString istr0(int i)
{
    return wxString::Format(_T("%i"),i);
}

bool CommandCollection::WriteConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("ShellExtensions"));
    //cfg->Clear();
    int len=interps.GetCount();
    cfg->Write(_T("ShellCmds/numcmds"), len);
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/name"), interps[i].name);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/command"), interps[i].command);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/wdir"), interps[i].wdir);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/wildcards"), interps[i].wildcards);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/menu"), interps[i].menu);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/menupriority"), interps[i].menupriority);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/cmenu"), interps[i].cmenu);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/cmenupriority"), interps[i].cmenupriority);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/envvarset"), interps[i].envvarset);
        cfg->Write(_T("ShellCmds/I")+istr+_T("/mode"), interps[i].mode);
    }
    return true;
}


bool CommandCollection::ReadConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("ShellExtensions"));
    int len;
    if(!cfg->Read(_T("ShellCmds/numcmds"), &len))
    {
//        cbMessageBox(_T("Warning: couldn't read interpreter config data"));
        return false;
    }
    for(int i=0;i<len;i++)
    {
        ShellCommand interp;
        wxString istr=istr0(i);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/name"), &interp.name);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/command"), &interp.command);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/wdir"), &interp.wdir);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/wildcards"), &interp.wildcards);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/menu"), &interp.menu);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/menupriority"), &interp.menupriority);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/cmenu"), &interp.cmenu);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/cmenupriority"), &interp.cmenupriority);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/envvarset"), &interp.envvarset);
        cfg->Read(_T("ShellCmds/I")+istr+_T("/mode"), &interp.mode);
        interps.Add(interp);
    }
    return true;
}

