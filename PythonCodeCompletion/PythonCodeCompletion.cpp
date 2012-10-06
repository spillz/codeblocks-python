#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include "PythonCodeCompletion.h"

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


int ID_EDITOR_HOOKS = wxNewId();
int ID_STDLIB_LOAD = wxNewId();
int ID_COMPLETE_PHRASE = wxNewId();
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

void PythonCodeCompletion::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...
    // hook to editors
    EditorHooks::HookFunctorBase* myhook = new EditorHooks::HookFunctor<PythonCodeCompletion>(this, &PythonCodeCompletion::EditorEventHook);
    m_EditorHookId = EditorHooks::RegisterHook(myhook);

    ConfigManager *mgr=Manager::Get()->GetConfigManager(_T("PythonCC"));

//    static wxString GetHomeFolder() { return home_folder; }
//    static wxString GetConfigFolder(){ return config_folder; }
//    static wxString GetPluginsFolder(bool global = true){ return GetFolder(global ? sdPluginsGlobal : sdPluginsUser); }
//    static wxString GetScriptsFolder(bool global = true){ return GetFolder(global ? sdScriptsGlobal : sdScriptsUser); }
//    static wxString GetDataFolder(bool global = true){ return global ? data_path_global : data_path_user; }
//    static wxString GetExecutableFolder(){ return app_path; }
//    static wxString GetTempFolder(){ return GetFolder(sdTemp); }

    #ifdef __WXMSW__
    wxString script = mgr->GetDataFolder(false)+_T("\\python\\python_completion_server.py");
    wxString stdlib = mgr->GetDataFolder(false)+_T("\\python\\STDLIB");
    #else
    wxString script = mgr->GetDataFolder(false)+_T("/python/python_completion_server.py");
    wxString stdlib = mgr->GetDataFolder(false)+_T("/python/STDLIB");
    #endif
    wxString command = _T("python ")+script+_T(" 8001");
    Manager::Get()->GetLogManager()->Log(_T("Launching python on ")+script);
    Manager::Get()->GetLogManager()->Log(_T("with command ")+command);
    py_server = XmlRpcMgr::Get().LaunchInterpreter(command,8001);
    if(py_server->IsDead())
    {
        wxMessageBox(_("Error Starting Python Code Completion Server"));
    }
    py_server->ExecAsync(_("load_stdlib"),XmlRpc::XmlRpcValue(stdlib.utf8_str()),this,ID_STDLIB_LOAD);
    m_libs_loaded=false;
}

void PythonCodeCompletion::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    EditorHooks::UnregisterHook(m_EditorHookId, true);
    if(!py_server->IsDead()) //TODO: Really should wait until serverr is no longer busy and request it to terminate via XMLRPC
        py_server->KillProcess(true);
}

void PythonCodeCompletion::OnStdLibLoaded(XmlRpcResponseEvent &event)
{
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        m_libs_loaded=true;
        Manager::Get()->GetLogManager()->Log(_("Python CC Initialized"));
    }
    if(event.GetState()==XMLRPC_STATE_REQUEST_FAILED)
    {
        Manager::Get()->GetLogManager()->Log(_("Python CC failed to initialize"));
    }
}

void PythonCodeCompletion::OnCompletePhrase(XmlRpcResponseEvent &event)
{
    if(event.GetState()==XMLRPC_STATE_RESPONSE)
    {
        Manager::Get()->GetLogManager()->Log(_("Completion return success"));
        XmlRpc::XmlRpcValue val=event.GetResponse();
        Manager::Get()->GetLogManager()->Log(_("Val = ")+wxString(std::string(val.toXml()).c_str(),wxConvUTF8));
        m_comp_results.Empty();
        if(val.getType()==val.TypeArray)
        {
            for(int i=0;i<val.size();++i)
                m_comp_results.Add(wxString(std::string(val[i]).c_str(),wxConvUTF8));
        }
        CodeComplete();
    }
    else
        Manager::Get()->GetLogManager()->Log(_("Comp failed"));
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
    NotImplemented(_T("PythonCodeCompletion::BuildModuleMenu()"));
}

int PythonCodeCompletion::CodeComplete()
{
    if (!IsAttached() || !m_libs_loaded)
        return -1;

    EditorManager* edMan = Manager::Get()->GetEditorManager();
    cbEditor* ed = edMan->GetBuiltinActiveEditor();
    if (!ed)
        return -3;

    FileType ft = FileTypeOf(ed->GetShortName());

    if(true)
    {
        int maxmatches=100;
        int count=0;
        if(count<maxmatches)
        {
            int pos   = ed->GetControl()->GetCurrentPos();
            cbStyledTextCtrl* control = ed->GetControl();
            //int start = ed->GetControl()->WordStartPosition(pos, true);
            int wordStartPos = control->WordStartPosition(pos, true);
//            while(control->GetCharAt(wordStartPos-1)==_T('.'))
//                wordStartPos = control->WordStartPosition(wordStartPos-2, true);
            int start=wordStartPos;
            bool caseSens=true;
            ed->GetControl()->AutoCompSetFillUps(wxEmptyString);
            ed->GetControl()->AutoCompSetIgnoreCase(!caseSens);
            ed->GetControl()->AutoCompSetCancelAtStart(true);
//            ed->GetControl()->AutoCompSetFillUps(m_CCFillupChars);
//            ed->GetControl()->AutoCompSetChooseSingle(m_IsAutoPopup ? false : m_CCAutoSelectOne);
            ed->GetControl()->AutoCompSetAutoHide(true);
//            ed->GetControl()->AutoCompSetDropRestOfWord(m_IsAutoPopup ? false : true);
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
            wxString msg = _("The Parser is still parsing files.");
            ed->GetControl()->CallTipShow(ed->GetControl()->GetCurrentPos(), msg);
    }

    return -5;
}

void PythonCodeCompletion::ShowCallTip()
{
    if (!IsAttached())
        return;

    if (!Manager::Get()->GetEditorManager())
        return;

    cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!ed)
        return;

    wxString filename = ed->GetShortName();
////    if (   ParserCommon::FileType(filename) == ParserCommon::ftOther
////        && Manager::Get()->GetPluginManager()->IsFileExtRegistered(filename) )
////        return;

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

//    wxArrayString items;
//    m_NativeParser.GetCallTips(maxCalltipLineSizeInChars, items, typedCommas);
//    std::set< wxString, std::less<wxString> > unique_tips; // check against this before inserting a new tip in the list
    wxString definition;
//    for (unsigned int i = 0; i < items.GetCount(); ++i)
//    {
//        // allow only unique, non-empty items with equal or more commas than what the user has already typed
//        if (unique_tips.find(items[i]) == unique_tips.end() && // unique
//            !items[i].IsEmpty() && // non-empty
//            typedCommas <= m_NativeParser.CountCommas(items[i], 0)) // commas satisfied
//        {
//            unique_tips.insert(items[i]);
//            if (count != 0)
//                definition << _T('\n'); // add new-line, except for the first line
//            definition << items[i];
//            if (start == 0)
//                m_NativeParser.GetCallTipHighlight(items[i], &start, &end, typedCommas);
//            ++count;
//        }
//    }
//
//    if (definition.empty())
//        return;

    ed->GetControl()->CallTipShow(pos, definition);
    if (start != 0 && end > start)
        ed->GetControl()->CallTipSetHighlight(start, end);
}

void PythonCodeCompletion::EditorEventHook(cbEditor* editor, wxScintillaEvent& event)
{
    if (!IsAttached())
    {
        Manager::Get()->GetLogManager()->Log(_("Not attached"));
        event.Skip();
        return;
    }

    if(editor->GetControl()->GetLexer() != wxSCI_LEX_PYTHON)
        return;

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
//        m_TimerCodeCompletion.Stop();
        Manager::Get()->GetLogManager()->Log(_("Python completion check begin"));
        const wxChar ch = event.GetKey();
        const int pos = control->GetCurrentPos();
        int wordStartPos = control->WordStartPosition(pos, true);
        while(control->GetCharAt(wordStartPos-1)==_T('.'))
            wordStartPos = control->WordStartPosition(wordStartPos-2, true);

        // -2 is used next because the char has already been added and Pos is ahead of it...
        const wxChar prevChar = control->GetCharAt(pos - 2);

        // if more than two chars have been typed, invoke CC
        bool m_CCAutoLaunch = true;
        int m_CCAutoLaunchChars = 2;
        const bool autoCC = m_CCAutoLaunch && (pos - wordStartPos >= m_CCAutoLaunchChars);
        wxString t=autoCC ? _("true") : _("false");
        Manager::Get()->GetLogManager()->Log(_("autoCC is ")+t);

////        // update calltip highlight while we type
////        if (control->CallTipActive())
////        {
////            TRACE(_T("wxEVT_SCI_CHARADDED -> ShowCallTip"));
////            ShowCallTip();
////        }
////
////        // start calltip
////        if (ch == _T('(') || ch == _T(','))
////        {
////            int style = control->GetStyleAt(control->GetCurrentPos() - 1);
////            if (!(control->IsString(style) || control->IsCharacter(style) || control->IsComment(style)))
////            {
////                if (control->CallTipActive())
////                    ++m_ActiveCalltipsNest;
////                TRACE(_T("wxEVT_SCI_CHARADDED -> ShowCallTip"));
////                ShowCallTip();
////            }
////        }
////
////        // support multi-line call tips
////        else if (ch == _T('\n') && m_ActiveCalltipsNest > 0)
////            ShowCallTip();
////
////        // end calltip
////        else if (ch == _T(')') || ch == _T(';'))
////        {
////            int style = control->GetStyleAt(control->GetCurrentPos() - 1);
////            if (!(control->IsString(style) || control->IsCharacter(style) || control->IsComment(style)))
////            {
////                // cancel any active calltip
////                control->CallTipCancel();
////                if (m_ActiveCalltipsNest > 0)
////                {
////                    --m_ActiveCalltipsNest;
////                    TRACE(_T("wxEVT_SCI_CHARADDED -> ShowCallTip"));
////                    ShowCallTip();
////                }
////            }
////        }

        // code completion
        if (   (autoCC && !control->AutoCompActive()) // not already active autocompletion
                 || (ch == _T('.')))
        {
            Manager::Get()->GetLogManager()->Log(_("Checking lexical state"));
            int style = control->GetStyleAt(pos);
//            TRACE(_T("Style at %d is %d (char '%c')"), pos, style, ch);
            if (   style != wxSCI_P_DEFAULT
                && style != wxSCI_P_CLASSNAME
                && style != wxSCI_P_DEFNAME
                && style != wxSCI_P_IDENTIFIER )
            {
                Manager::Get()->GetLogManager()->Log(_("Not a completable lexical state"));
                event.Skip();
                return;
            }
            wxString phrase=control->GetTextRange(wordStartPos,pos);
            Manager::Get()->GetLogManager()->Log(_T("CC: ")+phrase);
            py_server->ExecAsync(_T("complete_phrase"),XmlRpc::XmlRpcValue(phrase.mb_str(wxConvUTF8)),this,ID_COMPLETE_PHRASE);
        }
    }


    // allow others to handle this event
    event.Skip();
}
