#include <sdk.h> // Code::Blocks SDK
#include <editorbase.h>
#include <configurationpanel.h>
#include "permissions_preserver.h"

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<PermissionsPreserver> reg(_T("PermissionsPreserver"));
}



// constructor
PermissionsPreserver::PermissionsPreserver()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("PermissionsPreserver.zip")))
    {
        NotifyMissingFile(_T("PermissionsPreserver.zip"));
    }
}

// destructor
PermissionsPreserver::~PermissionsPreserver()
{
}

void PermissionsPreserver::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...

    // register event sinks
    Manager* pm = Manager::Get();
    pm->RegisterEventSink(cbEVT_EDITOR_BEFORE_SAVE, new cbEventFunctor<PermissionsPreserver, CodeBlocksEvent>(this, &PermissionsPreserver::OnEditorBeforeSave));
    pm->RegisterEventSink(cbEVT_EDITOR_SAVE, new cbEventFunctor<PermissionsPreserver, CodeBlocksEvent>(this, &PermissionsPreserver::OnEditorSave));

}

void PermissionsPreserver::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
}

void PermissionsPreserver::OnEditorBeforeSave(CodeBlocksEvent& event)
{
    EditorBase *ed = event.GetEditor();
    m_filename = ed->GetFilename();
    struct stat s;
    if (stat(m_filename.mb_str(wxConvUTF8),&s) == 0)
        m_permissions = s.st_mode;
    else
    {
        m_permissions = 0;
        m_filename = wxEmptyString;
    }
    event.Skip();
}

void PermissionsPreserver::OnEditorSave(CodeBlocksEvent& event)
{
    EditorBase *ed = event.GetEditor();
    wxString filename = ed->GetFilename();
    //m_filename will be empty if original permissions could not be established
    //(e.g. because file does not exist), so no chmod will occur in this case
    if (filename == m_filename)
        chmod(m_filename.mb_str(wxConvUTF8),m_permissions);
    event.Skip();
}

