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

bool CommandCollection::ExportConfig(const wxString &filename)
{
    wxFile file(filename, wxFile::write);
    if(!file.IsOpened())
        return false;
    file.Write(_T("##Shell Extensions Plugin (v0.2) Command Export##"));
    int len=interps.GetCount();
    for(int i=0;i<len;i++)
    {
        file.Write(_T("COMMAND#####################################\n"));
        file.Write(_T("name:")+interps[i].name+_T("\n"));
        file.Write(_T("command line:")+interps[i].command+_T("\n"));
        file.Write(_T("workdir:")+interps[i].wdir+_T("\n"));
        file.Write(_T("wildcards:")+interps[i].wildcards+_T("\n"));
        file.Write(_T("menu string:")+interps[i].menu+_T("\n"));
        file.Write(wxString::Format(_T("menu priority:%i\n"), interps[i].menupriority));
        file.Write(_T("context menu string:")+interps[i].cmenu+_T("\n"));
        file.Write(wxString::Format(_T("context menu priority:%i\n"), interps[i].cmenupriority));
        file.Write(_T("envvarset:")+interps[i].envvarset+_T("\n"));
        file.Write(_T("mode (W,C,):")+interps[i].mode+_T("\n"));
    }
    return true;
}

wxString readconfigdata(wxString &configstr)
{
    configstr=configstr.AfterFirst(':');
    wxString data=configstr.BeforeFirst('\n');
    configstr=configstr.AfterFirst('\n');
    return data;
}

bool CommandCollection::ImportConfig(const wxString &filename)
{
    wxFile file(filename, wxFile::write);
    if(!file.IsOpened())
        return false;
    wxString import=cbReadFileContents(file);
    import.Replace(_T("\r\n"),_T("\n"));
    import.Replace(_T("\r"),_T("\n"));
    import=import.AfterFirst('\n');
    while(!import.IsEmpty())
    {
        ShellCommand s;
        import=import.AfterFirst('\n');
        s.name=readconfigdata(import);
        s.command=readconfigdata(import);
        s.wdir=readconfigdata(import);
        s.wildcards=readconfigdata(import);
        s.menu=readconfigdata(import);
        long i;
        readconfigdata(import).ToLong(&i);
        s.menupriority=i;
        s.cmenu=readconfigdata(import);
        readconfigdata(import).ToLong(&i);
        s.cmenupriority=i;
        s.envvarset=readconfigdata(import);
        s.mode=readconfigdata(import);
        interps.Add(s);
    }
    return true;
}


bool CommandCollection::ImportLegacyConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    int len=0;
    if(!cfg->Read(_T("InterpProps/numinterps"), &len))
    {
        return false;
    }
//    if(cbMessageBox(_T("Do you want to convert your old style shell extensions/interpreted langs commands to new style Shell Extension commands?"),_T("Shell Extension Plugin Legacy Import"),wxYES_NO)==wxNO)
//        return false;
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        wxString name,exec,extensions;
        cfg->Read(_T("InterpProps/I")+istr+_T("/name"), &name);
        cfg->Read(_T("InterpProps/I")+istr+_T("/exec"), &exec);
        cfg->Read(_T("InterpProps/I")+istr+_T("/ext"), &extensions);
        int lenact=0;
        cfg->Read(_T("InterpProps/I")+istr+_T("/numactions"), &lenact);
        for(int j=0;j<lenact;j++)
        {
            ShellCommand interp;
            wxString jstr=istr0(j);
            wxString aname,command,mode,wdir,envvarset;
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/name"), &aname);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/command"), &command);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/mode"), &mode);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/workingdir"), &wdir);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/envvarset"), &envvarset);
            interp.name=name+_T(" ")+aname;
            interp.wildcards=extensions;
            interp.command=command;
            interp.command.Replace(_T("$interpreter"),exec);
            interp.wdir=wdir;
            interp.menu=name+_T("/")+aname;
            interp.cmenu=name+_T("/")+aname;
            interp.cmenupriority=0;
            interp.menupriority=0;
            interp.envvarset=envvarset;
            interp.mode=mode;
            interps.Add(interp);
        }
    }
    cfg->Clear();
    WriteConfig();
    return true;
}

