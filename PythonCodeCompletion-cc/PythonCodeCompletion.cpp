#include <sdk.h> // Code::Blocks SDK
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
int ID_STDLIB_LOAD = wxNewId();
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
    EVT_XMLRPC_RESPONSE(ID_STDLIB_LOAD,PythonCodeCompletion::OnStdLibLoaded)
    EVT_XMLRPC_RESPONSE(ID_COMPLETE_PHRASE,PythonCodeCompletion::OnCompletePhrase)
    EVT_XMLRPC_RESPONSE(ID_CALLTIP,PythonCodeCompletion::OnCalltip)
    EVT_XMLRPC_RESPONSE(ID_GOTO_DECLARATION,PythonCodeCompletion::OnGotoDefinition)
    EVT_MENU(ID_GOTO_DECLARATION, PythonCodeCompletion::OnClickedGotoDefinition)
END_EVENT_TABLE()

// constructor
PythonCodeCompletion::PythonCodeCompletion()
{
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
    m_request_id = 0;
    py_server=NULL;
    m_pImageList=NULL;
//    EditorHooks::HookFunctorBase* myhook = new EditorHooks::HookFunctor<PythonCodeCompletion>(this, &PythonCodeCompletion::EditorEventHook);
//    m_EditorHookId = EditorHooks::RegisterHook(myhook);

    // register event sinks
//    Manager* pm = Manager::Get();
//    pm->RegisterEventSink(cbEVT_COMPLETE_CODE, new cbEventFunctor<PythonCodeCompletion, CodeBlocksEvent>(this, &PythonCodeCompletion::CompleteCodeEvt));
//    pm->RegisterEventSink(cbEVT_SHOW_CALL_TIP, new cbEventFunctor<PythonCodeCompletion, CodeBlocksEvent>(this, &PythonCodeCompletion::ShowCallTipEvt));

    ConfigManager *mgr=Manager::Get()->GetConfigManager(_T("PythonCC"));

//    static wxString GetHomeFolder() { return home_folder; }
//    static wxString GetConfigFolder(){ return config_folder; }
//    static wxString GetPluginsFolder(bool global = true){ return GetFolder(global ? sdPluginsGlobal : sdPluginsUser); }
//    static wxString GetScriptsFolder(bool global = true){ return GetFolder(global ? sdScriptsGlobal : sdScriptsUser); }
//    static wxString GetDataFolder(bool global = true){ return global ? data_path_global : data_path_user; }
//    static wxString GetExecutableFolder(){ return app_path; }
//    static wxString GetTempFolder(){ return GetFolder(sdTemp); }

    wxString script = GetExtraFile(_T("/python/python_completion_server.py"));
    if(script==wxEmptyString)
    {
        Manager::Get()->GetLogManager()->Log(_T("PyCC: Missing python scripts. Try reinstalling the plugin."));
        return;
    }
    #ifndef EMBEDDER_DEBUG
    int port = -1; // Port == -1 uses pipe to do RPC over redirected stdin/stdout of the process, otherwise uses a socket
    wxString command = wxString::Format(_T("python -u %s %i"),script.c_str(),port);
    #else
    int port = 34567;
    wxString command = wxString::Format(_T("gnome-terminal -e \"python -u %s %i\""),script.c_str(),port);
    #endif
    Manager::Get()->GetLogManager()->Log(_T("PYCC: Launching python on ")+script);
    Manager::Get()->GetLogManager()->Log(_T("PYCC: with command ")+command);
    py_server = XmlRpcMgr::Get().LaunchProcess(command,port);
    if(py_server->IsDead())
    {
        Manager::Get()->GetLogManager()->Log(_("Error Starting Python Code Completion Server"));
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

//    bmp = cbLoadBitmap(prefix + _T("ctor_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_CTOR_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("ctor_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_CTOR_PROTECTED
//    bmp = cbLoadBitmap(prefix + _T("ctor_public.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_CTOR_PUBLIC
//    bmp = cbLoadBitmap(prefix + _T("dtor_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_DTOR_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("dtor_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_DTOR_PROTECTED
//    bmp = cbLoadBitmap(prefix + _T("dtor_public.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_DTOR_PUBLIC
//    bmp = cbLoadBitmap(prefix + _T("method_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_FUNC_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("var_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_VAR_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("var_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_VAR_PROTECTED
//    bmp = cbLoadBitmap(prefix + _T("preproc.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_PREPROCESSOR
//    bmp = cbLoadBitmap(prefix + _T("enum.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_ENUM
//    bmp = cbLoadBitmap(prefix + _T("enum_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_ENUM_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("enum_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_ENUM_PROTECTED
//    bmp = cbLoadBitmap(prefix + _T("enum_public.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_ENUM_PUBLIC
//    bmp = cbLoadBitmap(prefix + _T("enumerator.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_ENUMERATOR
//    bmp = cbLoadBitmap(prefix + _T("namespace.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_NAMESPACE
//    bmp = cbLoadBitmap(prefix + _T("typedef_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_TYPEDEF_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("typedef_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_TYPEDEF_PROTECTED
//    bmp = cbLoadBitmap(prefix + _T("typedef_public.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_TYPEDEF_PUBLIC
//    bmp = cbLoadBitmap(prefix + _T("symbols_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_SYMBOLS_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("vars_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_VARS_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("funcs_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_FUNCS_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("enums_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_ENUMS_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("preproc_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_PREPROC_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("others_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_OTHERS_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("typedefs_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_TYPEDEF_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("macro.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_MACRO
//    bmp = cbLoadBitmap(prefix + _T("macro_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_MACRO_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("macro_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_MACRO_PROTECTED
//    bmp = cbLoadBitmap(prefix + _T("macro_public.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_MACRO_PUBLIC
//    bmp = cbLoadBitmap(prefix + _T("macro_folder.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_MACRO_FOLDER
//    bmp = cbLoadBitmap(prefix + _T("class_private.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_CLASS_PRIVATE
//    bmp = cbLoadBitmap(prefix + _T("class_protected.png"), wxBITMAP_TYPE_PNG);
//    m_pImageList->Add(bmp); // PARSER_IMG_CLASS_PROTECTED
//    bmp = wxImage(cpp_keyword_xpm);
//    m_pImageList->Add(bmp);

//    if(port!=-1)
//        ::wxSleep(2); //need a delay to allow the xmlrpc server time to start
//    Manager::Get()->GetLogManager()->Log(_("Loading stdlib"));

    m_libs_loaded=true;
//Load the symbol lib synchronously
//    XmlRpc::XmlRpcValue(result);
//    if(py_server->Exec(_("load_stdlib"),XmlRpc::XmlRpcValue(stdlib.utf8_str()),result))
//        m_libs_loaded=true;
//    else
//        m_libs_loaded=false;

//Load the symbol lib asynchronously
//    py_server->ExecAsync(_("load_stdlib"),XmlRpc::XmlRpcValue(stdlib.utf8_str()),this,ID_STDLIB_LOAD);
//    m_libs_loaded=false;
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

void PythonCodeCompletion::OnStdLibLoaded(XmlRpcResponseEvent &event)
{
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        m_libs_loaded=true;
        Manager::Get()->GetLogManager()->Log(_("PyCC: Completion Library is Initialized"));
    }
    if(event.GetState()==XMLRPC_STATE_REQUEST_FAILED)
    {
        Manager::Get()->GetLogManager()->Log(_("PyCC: Completion Library Failed to initialize"));
    }
}

void PythonCodeCompletion::OnCalltip(XmlRpcResponseEvent &event)
{
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        XmlRpc::XmlRpcValue val=event.GetResponse();
        if(val.getType()==val.TypeArray && val.size()>0)
            val = val[0];
        if(val.getType()==val.TypeString)
        {
            m_ActiveCalltipDef=wxString(std::string(val).c_str(),wxConvUTF8);
            Manager::Get()->GetLogManager()->Log(_("PYCC: Call tip result ")+m_ActiveCalltipDef);
            ShowCallTip();
        }
        else
        {
            Manager::Get()->GetLogManager()->Log(_("PYCC: Call tip result is not a string"));
            XmlRpc::XmlRpcValue val=event.GetResponse();
            Manager::Get()->GetLogManager()->Log(wxString(val.toXml().c_str(),wxConvUTF8));
        }
    }
    else
    {
        Manager::Get()->GetLogManager()->Log(_("PYCC: Call tip failed"));
        XmlRpc::XmlRpcValue val=event.GetResponse();
        Manager::Get()->GetLogManager()->Log(wxString(val.toXml().c_str(),wxConvUTF8));
    }
}

void PythonCodeCompletion::OnCompletePhrase(XmlRpcResponseEvent &event)
{
    Manager::Get()->GetLogManager()->Log(_("PYCC: Got complete code request"));
//    if(!((m_state == STATE_COMPLETION_REQUEST) && event.GetId() == m_request_id))
//        return;
    m_state = STATE_COMPLETION_RETURNED;
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        Manager::Get()->GetLogManager()->Log(_("PYCC: Completion response"));
        XmlRpc::XmlRpcValue val=event.GetResponse();
        m_comp_results.Empty();
        if(val.getType()==val.TypeArray && val.size()>0)
        {
            Manager::Get()->GetLogManager()->Log(_("PYCC: Getting comp results"));
            for(int i=0;i<val.size();++i)
                m_comp_results.Add(wxString(std::string(val[i]).c_str(),wxConvUTF8));
            Manager::Get()->GetLogManager()->Log(wxString::Format(_("PYCC: Beginning comp with %i items"),val.size()));
//            CodeComplete();
            CodeBlocksEvent evt(cbEVT_COMPLETE_CODE);
            Manager::Get()->GetPluginManager()->NotifyPlugins(evt);
            Manager::Get()->GetLogManager()->Log(_("PYCC: Sent complete code msg"));
            //::wxPostEvent(Manager::Get()->GetCCManager(),evt);
            return;
        }
        Manager::Get()->GetLogManager()->Log(_("PYCC: Unexpected phrase completion result"));
        Manager::Get()->GetLogManager()->Log(wxString(val.toXml().c_str(),wxConvUTF8));
        return;
    }
    XmlRpc::XmlRpcValue val=event.GetResponse();
    Manager::Get()->GetLogManager()->Log(_("PYCC: Error in completion result"));
    Manager::Get()->GetLogManager()->Log(wxString(val.toXml().c_str(),wxConvUTF8));
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
    if (!menu || !IsAttached() || !m_libs_loaded)
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

bool PythonCodeCompletion::IsProviderFor(cbEditor *ed)
{
    if(ed->GetControl()->GetLexer()==wxSCI_LEX_PYTHON)
        return true;
    return false;
}

std::vector<PythonCodeCompletion::CCToken> PythonCodeCompletion::GetAutocompList(bool isAuto, cbEditor* ed, int& tknStart, int& tknEnd)
{
    Manager::Get()->GetLogManager()->Log(_("PYCC: GetAutocompList called"));
    std::vector<CCToken> tokens;
    if (m_state == STATE_COMPLETION_RETURNED)
    {
        for (int i = 0; i<m_comp_results.Count(); ++i)
        {
            PythonCodeCompletion::CCToken t(i,m_comp_results[i]);
            tokens.push_back(t);
        }
        cbStyledTextCtrl* control = ed->GetControl();
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
        Manager::Get()->GetLogManager()->Log(_("PYCC: Checking lexical state..."));
        cbStyledTextCtrl *control=ed->GetControl();
        int pos = control->GetCurrentPos();
        int style = control->GetStyleAt(pos);
        wxChar ch = control->GetCharAt(pos);
        if (  ch != _T('.') && style != wxSCI_P_DEFAULT
            && style != wxSCI_P_CLASSNAME
            && style != wxSCI_P_DEFNAME
            && style != wxSCI_P_IDENTIFIER )
        {
            Manager::Get()->GetLogManager()->Log(_("PYCC: Not a completable lexical state"));
            return tokens;
        }
        wxString phrase=control->GetTextRange(tknStart,tknEnd);
        Manager::Get()->GetLogManager()->Log(_T("PYCC: Looking for ")+phrase+_T(" in ")+ed->GetFilename()+wxString::Format(_T(" %i"),pos));
        m_state = STATE_COMPLETION_REQUEST;
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

wxStringVec PythonCodeCompletion::GetCallTips(int pos, int style, cbEditor* ed, int& hlStart, int& hlEnd, int& argsPos)
{
    Manager::Get()->GetLogManager()->Log(_("PYCC: GetCallTips called"));
    wxStringVec sv;
    if (m_state == STATE_CALLTIP_RETURNED)
    {
        Manager::Get()->GetLogManager()->Log(_("PYCC: Supplying the calltip ")+m_ActiveCalltip);
        sv.push_back(m_ActiveCalltipDef);
//        for (int i = 0; i<m_calltip_results.Count(); ++i)
//            sv.push_back(m_calltip_results[i]);
        m_state=STATE_NONE;
        return sv;
    }
    else
    {
//        if (   (!ed->AutoCompActive()) // not already active autocompletion
//                 || (ch == _T('.')))
        Manager::Get()->GetLogManager()->Log(wxString::Format(_("PYCC: Requesting the calltip at pos %i"),pos));
        cbStyledTextCtrl *control=ed->GetControl();
        m_state = STATE_CALLTIP_REQUEST;
        RequestCallTip(control,pos,ed->GetFilename());
        return sv;
    }
    return sv;
}

std::vector<PythonCodeCompletion::CCToken> PythonCodeCompletion::GetTokenAt(int pos, cbEditor* ed)
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

/// callbacks for actually autocompleting/writing the token to the editor
void PythonCodeCompletion::DoAutocomplete(const CCToken& token, cbEditor* ed)
{
    return;
}

void PythonCodeCompletion::DoAutocomplete(const wxString& token, cbEditor* ed)
{
    DoAutocomplete(CCToken(-1, token), ed);
}


int PythonCodeCompletion::CodeComplete()
{
    if (!IsAttached() || !m_libs_loaded)
        return -1;

    EditorManager* edMan = Manager::Get()->GetEditorManager();
    cbEditor* ed = edMan->GetBuiltinActiveEditor();
    if (!ed)
        return -3;

    if(true)
    {
        int maxmatches=100;
        int count=0;
        if(count<maxmatches)
        {
            cbStyledTextCtrl* control = ed->GetControl();
            control->ClearRegisteredImages();
            for (int i = 0; i < m_pImageList->GetImageCount(); i++)
                control->RegisterImage(i+1,m_pImageList->GetBitmap(i));

            int pos   = control->GetCurrentPos();
            //int start = ed->GetControl()->WordStartPosition(pos, true);
            int wordStartPos = control->WordStartPosition(pos, true);
//            while(control->GetCharAt(wordStartPos-1)==_T('.'))
//                wordStartPos = control->WordStartPosition(wordStartPos-2, true);
            int start=wordStartPos;
            bool caseSens=true;
            ed->GetControl()->AutoCompSetFillUps(wxEmptyString);
            ed->GetControl()->AutoCompSetIgnoreCase(!caseSens);
            ed->GetControl()->AutoCompSetCancelAtStart(true);
            ed->GetControl()->AutoCompSetSeparator(' ');
            ed->GetControl()->AutoCompSetTypeSeparator('?');
//            ed->GetControl()->AutoCompSetFillUps(m_CCFillupChars);
//            ed->GetControl()->AutoCompSetChooseSingle(m_IsAutoPopup ? false : m_CCAutoSelectOne);
            ed->GetControl()->AutoCompSetAutoHide(true);
//            ed->GetControl()->AutoCompSetDropRestOfWord(m_IsAutoPopup ? false : true);
//            wxString final = GetStringFromArray(m_comp_results, _T("\n"));
            wxString final = GetStringFromArray(m_comp_results, _T(" "));
            final.RemoveLast(); // remove last space

            ed->GetControl()->AutoCompShow(pos - start, final);
            return 0;
        }
        else if (!ed->GetControl()->CallTipActive())
        {
            wxString msg = _("Too many results.\n"
                             "Please edit results' limit in code-completion options,\n"
                             "or type at least one more character to narrow the scope down.");
            ed->GetControl()->CallTipShow(ed->GetControl()->GetCurrentPos(), msg);
            return -2;
        }
    }
    else if (!ed->GetControl()->CallTipActive())
    {
            wxString msg = _("The parser is still parsing files.");
            ed->GetControl()->CallTipShow(ed->GetControl()->GetCurrentPos(), msg);
    }

    return -5;
}

void PythonCodeCompletion::ShowCallTip()
{
    if (!IsAttached() || !m_libs_loaded)
        return;

    if (!Manager::Get()->GetEditorManager())
        return;

    cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!ed)
        return;
    Manager::Get()->GetLogManager()->Log(_T("PYCC: Showing calltip"));


    // calculate the size of the calltips window
    int pos = ed->GetControl()->GetCurrentPos();
    wxPoint p = ed->GetControl()->PointFromPosition(pos); // relative point
    int pixelWidthPerChar = ed->GetControl()->TextWidth(wxSCI_STYLE_LINENUMBER, _T("W"));
    int maxCalltipLineSizeInChars = (ed->GetSize().x - p.x) / pixelWidthPerChar;
    if (maxCalltipLineSizeInChars < 64)
    {
        // if less than a threshold in chars, recalculate the starting position (instead of shrinking it even more)
        p.x -= (64 - maxCalltipLineSizeInChars) * pixelWidthPerChar;
        // but if it goes out of range, continue shrinking
        if (p.x >= 0)
        {
            maxCalltipLineSizeInChars = 64;
            pos = ed->GetControl()->PositionFromPoint(p);
        }
        // else, out of range
    }

    int start = 0, end = 0, count = 0, typedCommas = 0;

//    int pos=m_ActiveCalltipPos;
    wxString definition=m_ActiveCalltipDef;

    ed->GetControl()->CallTipShow(pos, definition);
    if (start != 0 && end > start)
        ed->GetControl()->CallTipSetHighlight(start, end);
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
    py_server->ClearJobs();
    py_server->ExecAsync(_T("complete_phrase"),value,this,ID_COMPLETE_PHRASE);
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
    py_server->ClearJobs();
    py_server->ExecAsync(_T("complete_tip"),value,this,ID_CALLTIP);
    Manager::Get()->GetLogManager()->Log(_T("PYCC: Started server request"));
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
        Manager::Get()->GetLogManager()->Log(_("PYCC: Can't get doc, server is busy"));
        return wxString();
    }
    Manager::Get()->GetLogManager()->Log(_T("PYCC: Requesting doc"));
    XmlRpc::XmlRpcValue result;
    if (py_server->Exec(_T("get_doc"),value,result))
    {
//        if(result.getType()==result.TypeArray && result.size()>0)
//            result = result[0];
        if(result.getType()==result.TypeString)
            return wxString(std::string(result).c_str(),wxConvUTF8);
        else
        {
            Manager::Get()->GetLogManager()->Log(_("PYCC: Unexpected get_doc result"));
            Manager::Get()->GetLogManager()->Log(wxString(result.toXml().c_str(),wxConvUTF8));
            return wxString();
        }
    }
    Manager::Get()->GetLogManager()->Log(_("PYCC: Error in completion result"));
    Manager::Get()->GetLogManager()->Log(wxString(result.toXml().c_str(),wxConvUTF8));
    return wxString();

}

void PythonCodeCompletion::CompleteCodeEvt(CodeBlocksEvent& event)
{
    event.Skip();
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed)
        return;
    cbStyledTextCtrl *control = ed->GetControl();
    if(control->GetLexer()!=wxSCI_LEX_PYTHON)
        return;
    int pos = control->GetCurrentPos();
    wxString filename = ed->GetFilename();
    RequestCompletion(control, pos, filename);
}

void PythonCodeCompletion::ShowCallTipEvt(CodeBlocksEvent& event)
{
    event.Skip();
    cbEditor *ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if(!ed)
        return;
    cbStyledTextCtrl *control = ed->GetControl();
    if(control->GetLexer()!=wxSCI_LEX_PYTHON)
        return;
    int pos = control->GetCurrentPos();
    wxString filename = ed->GetFilename();
    RequestCallTip(control, pos, filename);
}

void PythonCodeCompletion::EditorEventHook(cbEditor* editor, wxScintillaEvent& event)
{
    if (!IsAttached())
    {
        Manager::Get()->GetLogManager()->Log(_("PYCC: Not attached"));
        event.Skip();
        return;
    }

    if(editor->GetControl()->GetLexer() != wxSCI_LEX_PYTHON)
    {
        event.Skip();
        return;
    }

    cbStyledTextCtrl* control = editor->GetControl();

////    if      (event.GetEventType() == wxEVT_SCI_CHARADDED)
////        TRACE(_T("wxEVT_SCI_CHARADDED"));
////    else if (event.GetEventType() == wxEVT_SCI_CHANGE)
////        TRACE(_T("wxEVT_SCI_CHANGE"));
////    else if (event.GetEventType() == wxEVT_SCI_KEY)
////        TRACE(_T("wxEVT_SCI_KEY"));
////    else if (event.GetEventType() == wxEVT_SCI_MODIFIED)
////        TRACE(_T("wxEVT_SCI_MODIFIED"));
////    else if (event.GetEventType() == wxEVT_SCI_AUTOCOMP_SELECTION)
////        TRACE(_T("wxEVT_SCI_AUTOCOMP_SELECTION"));

    if (event.GetEventType() == wxEVT_SCI_CHARADDED)
    {
        // a character was just added in the editor
        Manager::Get()->GetLogManager()->Log(_("PYCC: completion check begin"));
        wxChar ch = event.GetKey();
        int pos = control->GetCurrentPos();
        int wordStartPos = control->WordStartPosition(pos, true);
        while(control->GetCharAt(wordStartPos-1)==_T('.'))
            wordStartPos = control->WordStartPosition(wordStartPos-2, true);

        // -2 is used next because the char has already been added and Pos is ahead of it...
        const wxChar prevChar = control->GetCharAt(pos - 2);

        // if more than two chars have been typed, invoke CC
        bool m_CCAutoLaunch = true;
        int m_CCAutoLaunchChars = 2;
        const bool autoCC = m_CCAutoLaunch && (pos - wordStartPos >= m_CCAutoLaunchChars);

        // update calltip highlight while we type
        if (control->CallTipActive())
            ShowCallTip();

        //HOW TO DO CALLTIPS
        //FROM CURRENT SCOPE REVERSE FIND FROM CURRENT POS FOR '(', IGNORING COMMENTS, STRINGS, CHAR STYLE
        //LOOK FOR MATCHING BRACE, IF FOUND IS THE POSITION AHEAD OF THE CURRENT BRACE? NO, SKIP.
        //RETRIEVE THE SYMBOL TO THE LEFT OF THE '(' E.G. 'os.path.exists' <- ACTIVE SYMBOL
        //COUNT THE NUMBER OF COMMAS BETWEEN CURRENT POS AND '('    <- ACTIVE ARG
        //RETRIEVE THE CALLTIP AND DOC STRING FROM THE PYTHON SERVER

        // code completion
        if (   (autoCC && !control->AutoCompActive()) // not already active autocompletion
                 || (ch == _T('.')))
        {
            Manager::Get()->GetLogManager()->Log(_("PYCC: Checking lexical state..."));
            int style = control->GetStyleAt(pos-2);
            if (  ch != _T('.') && style != wxSCI_P_DEFAULT
                && style != wxSCI_P_CLASSNAME
                && style != wxSCI_P_DEFNAME
                && style != wxSCI_P_IDENTIFIER )
            {
                Manager::Get()->GetLogManager()->Log(_("PYCC: Not a completable lexical state"));
                event.Skip();
                return;
            }
            wxString phrase=control->GetTextRange(wordStartPos,pos);
            Manager::Get()->GetLogManager()->Log(_T("PYCC: Looking for ")+phrase+_T(" in ")+editor->GetFilename()+wxString::Format(_T(" %i"),pos));
            RequestCompletion(control,pos,editor->GetFilename());
            event.Skip();
            return;
        }

        // calltip
        Manager::Get()->GetLogManager()->Log(_T("PYCC: Checking for calltip"));
        pos=control->GetCurrentPos()-1;
        int startpos=pos;
        int cursorpos = pos+1;
        int minpos=control->GetCurrentPos()-1000;
        if (minpos<0)
            minpos=0;
        while(pos>=minpos)
        {
            wxChar ch = control->GetCharAt(pos);
            int style = control->GetStyleAt(pos);
            if (control->IsString(style) || control->IsCharacter(style) || control->IsComment(style))
            {
                pos--;
                continue;
            }
            if(ch==_T(')'))
                pos=control->BraceMatch(pos);
            else if(ch==_T('('))
                break;
            pos--;
        }
        if(pos<minpos || control->BraceMatch(pos)<control->GetCurrentPos() && control->BraceMatch(pos)!=-1)
        {
            Manager::Get()->GetLogManager()->Log(_T("PYCC: Not in function scope"));
            event.Skip();
            return;
        }

        m_ActiveCalltipPos=cursorpos;

        Manager::Get()->GetLogManager()->Log(_T("PYCC: Finding the calltip symbol"));
        //now find the symbol, if any, associated with the parens
        int end_pos=pos;
        pos--;
        ch = control->GetCharAt(pos);
        while(pos>=0)
        {
            while (ch == _T(' ') || ch == _T('\t'))
            {
                pos--;
                ch = control->GetCharAt(pos);
                wxString s=control->GetTextRange(pos,end_pos);
                Manager::Get()->GetLogManager()->Log(_T("#")+s);
            }
            int npos=control->WordStartPosition(pos+1,true);
            if(npos==pos+1)
            {
                event.Skip();
                return;
            }
            pos=npos;
            npos--;
            wxString s1=control->GetTextRange(pos,end_pos);
            Manager::Get()->GetLogManager()->Log(_T("#")+s1);
            ch = control->GetCharAt(npos);
            while (ch == _T(' ') || ch == _T('\t'))
            {
                npos--;
                ch = control->GetCharAt(npos);
                wxString s=control->GetTextRange(npos,end_pos);
                Manager::Get()->GetLogManager()->Log(_T("#")+s);
            }
            if (ch!=_T('.'))
                break;
            pos=npos-1;
            if(pos<0)
            {
                event.Skip();
                return;
            }
        }
        int start_pos=pos;
        wxString symbol=control->GetTextRange(start_pos,end_pos);
        symbol.Replace(_T(" "),wxEmptyString);
        symbol.Replace(_T("\t"),wxEmptyString);
        if(symbol==wxEmptyString)
        {
            event.Skip();
            return;
        }
        m_ActiveSymbol=symbol;
        Manager::Get()->GetLogManager()->Log(_T("PYCC: Found calltip symbol ")+symbol);
        RequestCallTip(control, cursorpos, editor->GetFilename());
    }
    // allow others to handle this event
    event.Skip();
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
        Manager::Get()->GetLogManager()->Log(_("PYCC: Goto Definition response"));
        XmlRpc::XmlRpcValue val=event.GetResponse(); //Should return an array of path, lineno
        m_comp_results.Empty();
        Manager::Get()->GetLogManager()->Log(wxString::Format(_("PYCC: XML response \n%s"),val.toXml().c_str()));
        if(val.getType()==val.TypeArray && val.size()>0)
        {
            wxString path = wxString(std::string(val[0]).c_str(),wxConvUTF8);
            int line = val[1];
            wxFileName f(path);
//            cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
//            if(!ed)
//                return;
//            f.MakeRelativeTo(ed->GetFilename());
            Manager::Get()->GetLogManager()->Log(wxString::Format(_("PYCC: Definition is at %s:%i"),f.GetFullPath().wx_str(),line));
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
            Manager::Get()->GetLogManager()->Log(_("PYCC: file could not be opened"));
            XmlRpc::XmlRpcValue val=event.GetResponse();
            Manager::Get()->GetLogManager()->Log(wxString(val.toXml().c_str(),wxConvUTF8));
            return;
        }
    }
    Manager::Get()->GetLogManager()->Log(_("PYCC: Bad response for goto definition"));
    XmlRpc::XmlRpcValue val=event.GetResponse();
    Manager::Get()->GetLogManager()->Log(wxString(val.toXml().c_str(),wxConvUTF8));
}

