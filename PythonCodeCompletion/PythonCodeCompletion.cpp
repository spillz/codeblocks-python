#include <sdk.h> // Code::Blocks SDK
#include <wx/menu.h>

#include <configurationpanel.h>
#include "PythonCodeCompletion.h"

#include <ccmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include <editor_hooks.h>
#include <configmanager.h>
#include <logmanager.h>
//
//#include <settings.h> // SDK
//#include <cbplugin.h>
//#include <cbproject.h>
//#include <sdk_events.h>

#include "XmlRpc.h"

#include <wx/arrstr.h>
#include <wx/string.h>
#include <wx/imaglist.h>


int ID_EDITOR_HOOKS = wxNewId();
int ID_COMPLETE_PHRASE = wxNewId();
int ID_CALLTIP = wxNewId();
int ID_GOTO_DECLARATION = wxNewId();
// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<PythonCodeCompletion> reg(_T("PythonCodeCompletion"));
}

// events handling
BEGIN_EVENT_TABLE(PythonCodeCompletion, cbCodeCompletionPlugin)
    // add any events you want to handle here
    EVT_XMLRPC_RESPONSE(ID_COMPLETE_PHRASE,PythonCodeCompletion::OnCompletePhrase)
    EVT_XMLRPC_RESPONSE(ID_CALLTIP,PythonCodeCompletion::OnCalltip)
    EVT_XMLRPC_RESPONSE(ID_GOTO_DECLARATION,PythonCodeCompletion::OnGotoDefinition)
    EVT_COMMAND(wxID_ANY,wxEVT_XMLRPC_PROC_END,PythonCodeCompletion::OnXmlRpcTerminated)
    EVT_MENU(ID_GOTO_DECLARATION, PythonCodeCompletion::OnClickedGotoDefinition)
END_EVENT_TABLE()

// constructor
PythonCodeCompletion::PythonCodeCompletion() : m_ActiveCalltipDef(_(""))
{
    m_comp_position.line = 0;
    m_comp_position.column = 0;
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("PythonCodeCompletion.zip")))
    {
        NotifyMissingFile(_T("PythonCodeCompletion.zip"));
    }
}

// destructor
PythonCodeCompletion::~PythonCodeCompletion()
{
}

wxString PythonCodeCompletion::GetExtraFile(const wxString &short_name)
{
    wxString fullname=ConfigManager::GetFolder(sdDataUser)+short_name;
    if(wxFileName::FileExists(fullname))
        return fullname;
    fullname=ConfigManager::GetFolder(sdDataGlobal)+short_name;
    if(wxFileName::FileExists(fullname))
        return fullname;
    return wxEmptyString;
}

void PythonCodeCompletion::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...
    // hook to editors
    m_state = STATE_NONE;
    m_request_submitted_count = 0;
    m_request_completed_count = 0;
    py_server=NULL;
    m_pImageList=NULL;

    ConfigManager *mgr=Manager::Get()->GetConfigManager(_T("PythonCC"));

    wxString server_config_module = ConfigManager::GetFolder(sdDataUser)+_T("/python/python_completion_config.py");

    wxString script = GetExtraFile(_T("/python/python_completion_server.py"));
    if(script==wxEmptyString)
    {
        Manager::Get()->GetLogManager()->Log(_T("PyCC: Missing python scripts. Try reinstalling the plugin."));
        return;
    }
    #ifndef EMBEDDER_DEBUG
    int port = -1; // Port == -1 uses pipe to do RPC over redirected stdin/stdout of the process, otherwise uses a socket
    wxString command = wxString::Format(_T("python -u %s %i %s"),script.c_str(),port, server_config_module.c_str());
    #else
    int port = 3456;
    wxString command = wxString::Format(_T("xterm -e python -u %s %i"),script.c_str(),port);
    #endif
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Starting python code completion server"));
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Launching python on ")+script);
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: with command ")+command);
    py_server = new XmlRpcInstance(command,port,_T("localhost"),0);
    if(py_server->IsDead())
    {
        Manager::Get()->GetLogManager()->Log(_("Error starting python code completion server"));
        return;
    }


    wxString prefix = ConfigManager::GetDataFolder() + _T("/images/codecompletion/");
    // bitmaps must be added by order of PARSER_IMG_* consts
    m_pImageList = new wxImageList(16, 16);
    wxBitmap bmp;
    bmp = cbLoadBitmap(prefix + _T("class_folder.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Module
    bmp = cbLoadBitmap(prefix + _T("class.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Class
    bmp = cbLoadBitmap(prefix + _T("class_public.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Class Object
    bmp = cbLoadBitmap(prefix + _T("typedef.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Type
    bmp = cbLoadBitmap(prefix + _T("var_public.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Type Instance
    bmp = cbLoadBitmap(prefix + _T("method_public.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // BuiltinFunctionType
    bmp = cbLoadBitmap(prefix + _T("method_protected.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // BuiltinMethodType
    bmp = cbLoadBitmap(prefix + _T("method_protected.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Method
    bmp = cbLoadBitmap(prefix + _T("method_private.png"), wxBITMAP_TYPE_PNG);
    m_pImageList->Add(bmp); // Function


    const wxString ctChars = wxT(",\n()"); // default set
    const wxString alChars = wxT("."); // default set

    Manager::Get()->GetCCManager()->RegisterCallTipChars(ctChars, this);
    Manager::Get()->GetCCManager()->RegisterAutoLaunchChars(alChars, this);


    if(port!=-1)
        ::wxSleep(2); //need a delay to allow the xmlrpc server time to start
}


void PythonCodeCompletion::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
//    EditorHooks::UnregisterHook(m_EditorHookId, true);
    if(py_server && !py_server->IsDead()) //TODO: Really should wait until server is no longer busy and request it to terminate via XMLRPC
        py_server->KillProcess(true);
    if(m_pImageList)
        delete m_pImageList;
}


//TODO: This does appear to work correctly (should get a notification if the server dies, but don't
void PythonCodeCompletion::OnXmlRpcTerminated(wxCommandEvent &event)
{
    Manager::Get()->GetLogManager()->LogError(_T("PYCC: Server died"));
}


void PythonCodeCompletion::HandleError(XmlRpcResponseEvent &event, wxString message)
{
    //TODO: Should probably kill/restart the XmlRpc server here
    if(event.GetState()==XMLRPC_STATE_REQUEST_FAILED)
    {
        wxMessageBox(_("ERROR PROCESSING PYCC REQUEST: Check the log for details"));
        Manager::Get()->GetLogManager()->LogError(message);
        Manager::Get()->GetLogManager()->LogError(wxString(event.GetResponse().toXml().c_str(),wxConvUTF8));
    }
}

void PythonCodeCompletion::OnCalltip(XmlRpcResponseEvent &event)
{
    m_state = STATE_NONE;
    m_request_completed_count++;
    Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_("PYCC: Got calltip response #%i from server"),m_request_completed_count));
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        if(m_request_completed_count != m_request_submitted_count)
            return;
        XmlRpc::XmlRpcValue val=event.GetResponse();
        if(val.getType()==val.TypeArray && val.size()>0)
            val = XmlRpc::XmlRpcValue(std::string(val[0]));
        if(val.getType()==val.TypeString)
        {
            m_ActiveCalltipDef=wxString(std::string(val).c_str(),wxConvUTF8);
            m_state = STATE_CALLTIP_RETURNED;
            Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Call tip result ")+m_ActiveCalltipDef.tip);
            CodeBlocksEvent evt(cbEVT_SHOW_CALL_TIP);
            Manager::Get()->ProcessEvent(evt);//->GetPluginManager()->NotifyPlugins(evt);
        }
        else
        {
            Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Call tip result is not a string"));
            XmlRpc::XmlRpcValue val=event.GetResponse();
            Manager::Get()->GetLogManager()->DebugLog(wxString(val.toXml().c_str(),wxConvUTF8));
        }
    }
    else
        HandleError(event,_T("Error requesting calltip"));
}

void PythonCodeCompletion::OnCompletePhrase(XmlRpcResponseEvent &event)
{
    Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Got complete code response from server"));
    m_state = STATE_NONE;
    m_request_completed_count++;
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        if(m_request_completed_count != m_request_submitted_count)
            return;
        Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Completion response"));
        XmlRpc::XmlRpcValue val=event.GetResponse();
        m_comp_results.Empty();
        if(val.getType()==val.TypeArray && val.size()>0)
        {
            for(int i=0;i<val.size();++i)
                m_comp_results.Add(wxString(std::string(val[i]).c_str(),wxConvUTF8));
            m_state = STATE_COMPLETION_RETURNED;
            CodeBlocksEvent evt(cbEVT_COMPLETE_CODE);
            Manager::Get()->ProcessEvent(evt);
            return;
        }
        Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Unexpected phrase completion result"));
        Manager::Get()->GetLogManager()->DebugLog(wxString(val.toXml().c_str(),wxConvUTF8));
        return;
    }
    else
        HandleError(event,_T("PYCC: Error requesting completion result"));
}


int PythonCodeCompletion::Configure()
{
    //create and display the configuration dialog for your plugin
    cbConfigurationDialog dlg(Manager::Get()->GetAppWindow(), wxID_ANY, _("Your dialog title"));
    cbConfigurationPanel* panel = GetConfigurationPanel(&dlg);
    if (panel)
    {
        dlg.AttachConfigurationPanel(panel);
        PlaceWindow(&dlg);
        return dlg.ShowModal() == wxID_OK ? 0 : -1;
    }
    return -1;
}

void PythonCodeCompletion::BuildMenu(wxMenuBar* menuBar)
{
    //The application is offering its menubar for your plugin,
    //to add any menu items you want...
    //Append any items you need in the menu...
    //NOTE: Be careful in here... The application's menubar is at your disposal.
    NotImplemented(_T("PythonCodeCompletion::BuildMenu()"));

}

void PythonCodeCompletion::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data)
{
    //Some library module is ready to display a pop-up menu.
    //Check the parameter \"type\" and see which module it is
    //and append any items you need in the menu...
    //TIP: for consistency, add a separator as the first item...
    // if not attached, exit
    if (!menu || !IsAttached() )
        return;

    if (type == mtEditorManager)
    {
        cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
        if (!ed)
            return;

        cbStyledTextCtrl *control = ed->GetControl();

        if(control->GetLexer() != wxSCI_LEX_PYTHON)
            return;

        int pos   = control->GetCurrentPos();
        int wordStartPos = control->WordStartPosition(pos, false);
        int wordEndPos = control->WordEndPosition(pos, false);
        wxString word = control->GetTextRange(wordStartPos, wordEndPos);
        if (word.Find(' ')>=0)
            return;

        if(wordStartPos<wordEndPos)
        {
            wxString msg;
            size_t pos = 0;
            msg.Printf(_("Python: goto definition of '%s'"), word.wx_str());
            menu->Insert(pos, ID_GOTO_DECLARATION, msg);
            ++pos;

        }
    }
}

PythonCodeCompletion::CCProviderStatus PythonCodeCompletion::GetProviderStatusFor(cbEditor* ed)
{
    if (ed->GetControl()->GetLexer()==wxSCI_LEX_PYTHON)
        return ccpsActive;
    return ccpsInactive;
}

std::vector<PythonCodeCompletion::CCToken> PythonCodeCompletion::GetAutocompList(bool isAuto, cbEditor* ed, int& tknStart, int& tknEnd)
{
    Manager::Get()->GetLogManager()->DebugLog(_("PYCC: GetAutocompList called"));

    cbStyledTextCtrl* control = ed->GetControl();
    const int line = control->LineFromPosition(tknStart);
    const int lnStart = control->PositionFromLine(line);
    int column = tknStart - lnStart;
    for (; column > 0; --column)
    {
        if (   !wxIsspace(control->GetCharAt(lnStart + column - 1))
            || (column != 1 && !wxIsspace(control->GetCharAt(lnStart + column - 2))) )
        {
            break;
        }
    }

    std::vector<CCToken> tokens;
    if (m_state == STATE_COMPLETION_RETURNED)
    {
        if ((m_comp_position.line != line)||(m_comp_position.column != column))
        {
            Manager::Get()->GetLogManager()->DebugLog( _("PYCC: Position has changed since last CC request") );
            m_state=STATE_NONE;
            return tokens;
        }
        for (int i = 0; i<m_comp_results.Count(); ++i)
        {
            long int category=-1;
            wxString cat = m_comp_results[i].AfterFirst('?');
            cat.ToLong(&category);
            PythonCodeCompletion::CCToken t(i,m_comp_results[i].BeforeFirst('?'),category);
            tokens.push_back(t);
        }
        control->ClearRegisteredImages();
        for (int i = 0; i < m_pImageList->GetImageCount(); i++)
            control->RegisterImage(i+1,m_pImageList->GetBitmap(i));
        m_state=STATE_NONE;
        return tokens;
    }
    else
    {
//        if (   (!ed->AutoCompActive()) // not already active autocompletion
//                 || (ch == _T('.')))
        Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Checking lexical state..."));
        int pos = tknEnd;
        int style = control->GetStyleAt(pos);
        wxChar ch = control->GetCharAt(pos);
        if (  ch != _T('.') && style != wxSCI_P_DEFAULT
            && style != wxSCI_P_CLASSNAME
            && style != wxSCI_P_DEFNAME
            && style != wxSCI_P_IDENTIFIER
            && style != wxSCI_P_OPERATOR)
        {
            Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Not a completable lexical state"));
            return tokens;
        }
        wxString phrase=control->GetTextRange(tknStart,tknEnd);
        Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Looking for ")+phrase+_T(" in ")+ed->GetFilename()+wxString::Format(_T(" %i"),pos));
        m_state = STATE_COMPLETION_REQUEST;
        m_comp_position.line = line;
        m_comp_position.column = column;
        RequestCompletion(control,pos,ed->GetFilename());
        return tokens;
    }
    return tokens;
}
/// returns html
wxString PythonCodeCompletion::GetDocumentation(const CCToken& token)
{
    wxString doc;
    doc = RequestDocString(token.id);
    return doc;
}

std::vector<PythonCodeCompletion::CCCallTip> PythonCodeCompletion::GetCallTips(int pos, int style, cbEditor* ed, int& argsPos)
//wxStringVec PythonCodeCompletion::GetCallTips(int pos, int style, cbEditor* ed, int& hlStart, int& hlEnd, int& argsPos)
{
    Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_("PYCC: GetCallTips called at pos %i"),pos));
    std::vector<PythonCodeCompletion::CCCallTip> cts;
    if (m_state == STATE_CALLTIP_RETURNED)
    {
        m_state=STATE_NONE;
        Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Supplying the calltip ")+m_ActiveCalltipDef.tip);
        PythonCodeCompletion::CCCallTip ct=m_ActiveCalltipDef;
        m_ActiveCalltipDef=PythonCodeCompletion::CCCallTip(_T(""));
        wxString s = ct.tip.BeforeFirst(_T('\n'));
        if(s==_(""))
        {
            return cts;
        }
        int pos = s.Find('(')+1;
        argsPos = pos;
        s=s.AfterFirst('(');
        for (int i=0;i<m_argNumber;++i)
        {
             int inc = s.Find(',');
             if (inc>=0)
             {
                pos+=inc+1;
                s = s.AfterFirst(',');
             }
        }
        ct.hlStart = pos;
        int linc = s.Find(',');
        if (linc<0)
            linc = s.Find(')');
        ct.hlEnd = ct.hlStart;
        if (linc>0)
            ct.hlEnd += linc;
        Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_("PYCC: Showing calltip at pos %i, hl: %i %i"),pos,ct.hlStart,ct.hlEnd));
//        for (int i = 0; i<m_calltip_results.Count(); ++i)
//            sv.push_back(m_calltip_results[i]);
        cts.push_back(ct);
    }
    else
    {
        //TODO: Probably don't need to hit the python server every time if we can figure out we are still in the scope of the same function as the last call
        int argNumber;
        int argsStartPos;
        GetCalltipPositions(ed, pos, argsStartPos, argNumber);
        if (argsStartPos<0)
            return cts;
        m_argsPos = argsStartPos; //TODO: Remove
        m_argNumber = argNumber;
        Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_("PYCC: pos %i, Requesting the calltip at pos %i, argnum %i"),pos,argsStartPos,argNumber));
        cbStyledTextCtrl *control=ed->GetControl();
        m_state = STATE_CALLTIP_REQUEST;
        RequestCallTip(control,argsStartPos,ed->GetFilename());
    }
    return cts;
}

std::vector<PythonCodeCompletion::CCToken> PythonCodeCompletion::GetTokenAt(int pos, cbEditor* ed, bool& allowCallTip)
//std::vector<PythonCodeCompletion::CCToken> PythonCodeCompletion::GetTokenAt(int pos, cbEditor* ed)
{
    std::vector<CCToken> tokens;
    return tokens;
}

/// dismissPopup is false by default
wxString PythonCodeCompletion::OnDocumentationLink(wxHtmlLinkEvent& event, bool& dismissPopup)
{
    wxString doc;
    return doc;
}

void PythonCodeCompletion::RequestCompletion(cbStyledTextCtrl *control, int pos, const wxString &filename)
{
    int line = control->LineFromPosition(pos);
    int column = pos - control->PositionFromLine(line);
    XmlRpc::XmlRpcValue value;
    value.setSize(4);
    value[0] = filename.utf8_str(); //mb_str(wxConvUTF8);
    value[1] = control->GetText().utf8_str(); //mb_str(wxConvUTF8);
    value[2] = line;
    value[3] = column;
//    py_server->ClearJobs();
    py_server->ExecAsync(_T("complete_phrase"),value,this,ID_COMPLETE_PHRASE);
    m_request_submitted_count++;
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Started server request"));
}

void PythonCodeCompletion::RequestCallTip(cbStyledTextCtrl *control, int pos, const wxString &filename)
{
    int line = control->LineFromPosition(pos);
    int column = pos - control->PositionFromLine(line);
    XmlRpc::XmlRpcValue value;
    value.setSize(4);
    value[0] = filename.utf8_str(); //mb_str(wxConvUTF8);
    value[1] = control->GetText().utf8_str(); //mb_str(wxConvUTF8);
    value[2] = line;
    value[3] = column;
//    py_server->ClearJobs();
    py_server->ExecAsync(_T("complete_tip"),value,this,ID_CALLTIP);
    m_request_submitted_count++;
    Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_T("PYCC: Started server request #%i line %i col %i"),m_request_submitted_count,line,column));
}

wxString PythonCodeCompletion::RequestDocString(int id)
{
    //Unlike the other request*** functions this one is synchronous and will block the UI if it takes to long
    XmlRpc::XmlRpcValue value;
    value.setSize(1);
    value[0] = id; //mb_str(wxConvUTF8);
    py_server->ClearJobs();
    if(py_server->IsJobRunning())
    {
        Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Can't get doc, server is busy"));
        return wxString();
    }
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Requesting doc"));
    XmlRpc::XmlRpcValue result;
    if (py_server->Exec(_T("get_doc"),value,result))
    {
//        if(result.getType()==result.TypeArray && result.size()>0)
//            result = result[0];
        m_request_submitted_count++;
        m_request_completed_count++;
        if(result.getType()==result.TypeString)
            return wxString(std::string(result).c_str(),wxConvUTF8);
        else
        {
            Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Unexpected get_doc result"));
            Manager::Get()->GetLogManager()->DebugLog(wxString(result.toXml().c_str(),wxConvUTF8));
            return wxString();
        }
    } else
    {
        Manager::Get()->GetLogManager()->LogError(_T("PYCC: Bad response for get_doc"));
        Manager::Get()->GetLogManager()->LogError(wxString(result.toXml().c_str(),wxConvUTF8));

    }
    return wxString();
}

/*
This function computes
argsStartPos: is the stc position of the brace
argNumber: the number of the argument that the cursor is currently position at (0 for the first, 1 for the second etc.)
*/
void PythonCodeCompletion::GetCalltipPositions(cbEditor* editor, int pos, int &argsStartPos, int &argNumber)
{
    cbStyledTextCtrl* control = editor->GetControl();

    //FROM CURRENT SCOPE REVERSE FIND FROM CURRENT POS FOR '(', IGNORING COMMENTS, STRINGS, CHAR STYLE
    //LOOK FOR MATCHING BRACE, IF FOUND, IS THE POSITION AHEAD OF THE CURRENT BRACE? NO, SKIP.
    //RETRIEVE THE SYMBOL TO THE LEFT OF THE '(' E.G. 'os.path.exists' <- ACTIVE SYMBOL
    //COUNT THE NUMBER OF COMMAS BETWEEN CURRENT POS AND '('    <- ACTIVE ARG
    //RETRIEVE THE CALLTIP AND DOC STRING FROM THE PYTHON SERVER
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Attempting to find call position"));
    wxChar ch;
    int cpos=pos-1;
    int startpos=pos;
    int minpos=pos-1000;
    if (minpos<0)
        minpos=0;
    while(cpos>=minpos)
    {
        ch = control->GetCharAt(cpos);
        int style = control->GetStyleAt(cpos);
        if (control->IsString(style) || control->IsCharacter(style) || control->IsComment(style))
        {
            cpos--;
            continue;
        }
        if(ch==_T(')'))
            cpos=control->BraceMatch(cpos);
        else if(ch==_T('('))
            break;
        cpos--;
    }
    if(cpos<minpos || control->BraceMatch(cpos)<pos && control->BraceMatch(cpos)!=-1)
    {
        Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Not in function scope"));
        argsStartPos = -1;
        argNumber = 0;
        return;
    }

    argsStartPos = cpos + 1;

    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Finding the calltip symbol"));
    //now find the symbol, if any, associated with the parens
    int end_pos=cpos;
    cpos--;
    ch = control->GetCharAt(cpos);
    while(cpos>=0)
    {
        while (ch == _T(' ') || ch == _T('\t'))
        {
            cpos--;
            ch = control->GetCharAt(cpos);
            wxString s=control->GetTextRange(cpos,end_pos);
            Manager::Get()->GetLogManager()->DebugLog(_T("#")+s);
        }
        int npos=control->WordStartPosition(cpos+1,true);
        if(npos==cpos+1)
        {
            argsStartPos = -1;
            argNumber = 0;
            return;
        }
        cpos=npos;
        npos--;
        wxString s1=control->GetTextRange(cpos,end_pos);
        Manager::Get()->GetLogManager()->DebugLog(_T("#")+s1);
        ch = control->GetCharAt(npos);
        while (ch == _T(' ') || ch == _T('\t'))
        {
            npos--;
            ch = control->GetCharAt(npos);
            wxString s=control->GetTextRange(npos,end_pos);
            Manager::Get()->GetLogManager()->DebugLog(_T("#")+s);
        }
        if (ch!=_T('.'))
            break;
        cpos=npos-1;
        if(cpos<0)
        {
            argsStartPos = -1;
            argNumber = 0;
            return;
        }
    }
    int token_pos=cpos;
    wxString symbol=control->GetTextRange(token_pos,end_pos);
    symbol.Replace(_T(" "),wxEmptyString);
    symbol.Replace(_T("\t"),wxEmptyString);
    if(symbol==wxEmptyString) //No symbol means not a function call, so return
    {
        argsStartPos = -1;
        argNumber = 0;
        return;
    }

    //Now figure out which argument the cursor is at by counting "," characters
    argNumber = 0;
    cpos = argsStartPos;
    while(cpos<=startpos)
    {
        ch = control->GetCharAt(cpos);
        int style = control->GetStyleAt(cpos);
        if ((ch=='(' || ch =='[' || ch =='{') && (style == wxSCI_P_DEFAULT || style==wxSCI_P_OPERATOR))
        {
            cpos = control->BraceMatch(cpos);
            if (cpos == wxSCI_INVALID_POSITION)
                break;
        }
        if (ch==',' && (style==wxSCI_P_DEFAULT || style==wxSCI_P_OPERATOR))
            argNumber++;
        cpos++;
    }
    Manager::Get()->GetLogManager()->DebugLog(_T("PYCC: Found calltip symbol ")+symbol);
}

void PythonCodeCompletion::OnClickedGotoDefinition(wxCommandEvent& event)
{
    cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed)
        return;

    cbStyledTextCtrl *control = ed->GetControl();

    if(control->GetLexer() != wxSCI_LEX_PYTHON)
        return;
    int pos   = control->GetCurrentPos();
    int line = control->LineFromPosition(pos);
    int column = pos - control->PositionFromLine(line);
    XmlRpc::XmlRpcValue value;
    value.setSize(4);
    value[0] = ed->GetFilename().utf8_str();
    value[1] = control->GetText().utf8_str();
    value[2] = line;
    value[3] = column;
    py_server->ClearJobs();
    py_server->ExecAsync(_T("get_definition_location"),value,this,ID_GOTO_DECLARATION);
}

void PythonCodeCompletion::OnGotoDefinition(XmlRpcResponseEvent &event)
{
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        Manager::Get()->GetLogManager()->DebugLog(_("PYCC: Goto Definition response"));
        XmlRpc::XmlRpcValue val=event.GetResponse(); //Should return an array of path, lineno
        m_comp_results.Empty();
        Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_("PYCC: XML response \n%s"),val.toXml().c_str()));
        if(val.getType()==val.TypeArray && val.size()>0)
        {
            wxString path = wxString(std::string(val[0]).c_str(),wxConvUTF8);
            int line = val[1];
            wxFileName f(path);
//            cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
//            if(!ed)
//                return;
//            f.MakeRelativeTo(ed->GetFilename());
            Manager::Get()->GetLogManager()->DebugLog(wxString::Format(_("PYCC: Definition is at %s:%i"),f.GetFullPath().wx_str(),line));
            if(f.FileExists())
            {
                cbEditor* ed = Manager::Get()->GetEditorManager()->Open(f.GetFullPath());
                if (ed)
                {
                    ed->Show(true);
                    ed->GotoLine(line, false);
                }
                return;
            }
            Manager::Get()->GetLogManager()->DebugLog(_("PYCC: file could not be opened"));
            XmlRpc::XmlRpcValue val=event.GetResponse();
            Manager::Get()->GetLogManager()->DebugLog(wxString(val.toXml().c_str(),wxConvUTF8));
            return;
        }
    }
    else
        HandleError(event,_T("PYCC: Bad response for goto definition"));
}

