#include "interpreterproperties.h"
#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
//#include "InterpretedLangs.h"

/*
virtual InterpreterCollection::~InterpreterProperties()
{
}
*/

wxString istr0(int i)
{
    char buffer[20]="";
    char *buf=&buffer[0];
    buf=_itoa(i,buf,10);
    wxString s;
    while(*buf!=0)
    {
        s+=*buf;
        buf++;
    }
    return s;
}

bool InterpreterCollection::WriteConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    cfg->Clear();
    int len=interps.size();
    cfg->Write(_T("InterpProps/numinterps"), len);
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        cfg->Write(_T("InterpProps/I")+istr+_T("/name"), interps[i].name);
        cfg->Write(_T("InterpProps/I")+istr+_T("/exec"), interps[i].exec);
//        cfg->Write(_T("/InterpProps/")+istr+_T("/execpath"), interps[i].execpath);
        cfg->Write(_T("InterpProps/I")+istr+_T("/ext"), interps[i].extensions);
        int lenact = interps[i].actions.size();
        cfg->Write(_T("InterpProps/I")+istr+_T("/numactions"), lenact);
        for(int j=0;j<lenact;j++)
        {
            wxString jstr=istr0(j);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/name"), interps[i].actions[j].name);
            cfg->Write(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/command"), interps[i].actions[j].command);
        }
    }
//    cfg->EnumerateSubPaths("/InterpProps");
    return true;
}


bool InterpreterCollection::ReadConfig()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    int len;
    if(!cfg->Read(_T("InterpProps/numinterps"), &len))
    {
//        wxMessageBox(_T("Warning: couldn't read interpreter config data"));
        return false;
    }
    interps.resize(len);
    for(int i=0;i<len;i++)
    {
        wxString istr=istr0(i);
        cfg->Read(_T("InterpProps/I")+istr+_T("/name"), &interps[i].name);
        cfg->Read(_T("InterpProps/I")+istr+_T("/exec"), &interps[i].exec);
//        interps[i].execpath=cfg->Read(_T("/InterpProps/")+istr+_T("/execpath"), &empty);
        cfg->Read(_T("InterpProps/I")+istr+_T("/ext"), &interps[i].extensions);
        int lenact;
        cfg->Read(_T("InterpProps/I")+istr+_T("/numactions"), &lenact);
        interps[i].actions.resize(lenact);
        for(int j=0;j<lenact;j++)
        {
            wxString jstr=istr0(j);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/name"), &interps[i].actions[j].name);
            cfg->Read(_T("InterpProps/I")+istr+_T("/actions/A")+jstr+_T("/command"), &interps[i].actions[j].command);
        }
    }
//    cfg->EnumerateSubPaths("/InterpProps");
    return true;
}

/*
bool InterpreterCollection::ConvertExtsWildCard(const wxString &seplist)
{
}

wxString InterpreterCollection::ConvertExtsWildCard()
{
}

InterpreterCollection::InterpreterCollection()
{
    //ctor
}

InterpreterCollection::~InterpreterCollection()
{
    //dtor
}
*/

/*
bool InterpreterProperties::WriteConfig(const wxString &id)
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    cfg->Write(_T("/InterProps/"), m_name);
    cfg->Write(_T("/InterProps/"), m_cmd);
    cfg->Write(_T("/InterProps/"), m_cmdlinepre);
    cfg->Write(_T("/InterProps/"), m_cmdlinepost);
    cfg->Write(_T("/InterProps/"), m_path);
//    cfg->EnumerateSubPaths("/InterpProps");
    return true;
}

bool InterpreterProperties::ReadConfig(const wxString &id)
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("InterpretedLangs"));
    m_name=cfg->Read(_T("/InterProps/"), m_name);
    m_cmd=cfg->Read(_T("/InterProps/"), m_cmd);
    m_cmdlinepre=cfg->Read(_T("/InterProps/"), m_cmdlinepre);
    m_cmdlinepost=cfg->Read(_T("/InterProps/"), m_cmdlinepost);
    m_path=cfg->Read(_T("/InterProps/"), m_path);
    return true;
}


wxString InterpreterProperties::ConvertExtsWildCard()
{
    wxString str;
    wxString strsrc=m_fileexts;
//    while(strsrc[i].AfterFirst(_T(';')).insert(0,_T("*."));
//    for(int i=0;i<m_fileexts.size();i++)
//        str+=_T("*.")+m_fileexts[i]+_T(";");
//    return str;
}


bool InterpreterProperties::ConvertExtsWildCard(const wxString &str)
{
//    m_fileexts.clear();
//    wxString s=str;
//    m_fileexts.push_back(s.BeforeFirst(_T(';')));
//    s=s.AfterFirst(_T(';'));
//    while(s!=_T(""))
//    {
//        m_fileexts.push_back(s.BeforeFirst(_T(';')));
//        s=s.AfterFirst(_T(';'));
//   }
    return true; //could check for invalid characters and return false if present...
}
*/
