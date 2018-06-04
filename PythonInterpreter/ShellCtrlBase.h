#ifndef SHELLCTRL_H
#define SHELLCTRL_H

#include <map>


#include <wx/process.h>
#include <wx/aui/aui.h>

// CLASSES DEFINED IN THIS LIBRARY
class ShellManager; //Manages the collection of Shell Control Widgets allowing user interaction with external processes within a tabbed notepage - usually the main app (or some plugin) will declare a global instance of this manager. See the full declaration below for more detail
class ShellCtrlBase; //The manager manages a set of Shell Control widgets that redirect I/O from an external process - ShellCtrlBase is an abstract base class allowing the developer to create custom controls for handling I/O from their process. Note that "process" is not defined here, it could be a new thread, an external program, a server connection etc
class ShellRegInfo; //Every custom shell control must provide basic info to a global registry
class ShellRegistry; //The global registry stores the info for all known custom shell controls
template<class T> class ShellCtrlRegistrant; //The developer makes their custom Shell Control classes available to the manager (and the main application) by creating an instance of this template class

typedef ShellCtrlBase*(*fnCreate)(wxWindow*, int, const wxString &, ShellManager *); //typedef defining function to create a custom shell control widget
typedef void(*fnFree)(ShellCtrlBase*); //typedef defining function to free a custom shell control widget

//Every type of shell control has the following registration info
struct ShellRegInfo
{
//    wxString name; //unique name of the type
    fnCreate create; //static function call needed to create instance on the heap
    fnFree free; //static function call needed to free instance
};

// Before shells can be used they must be registered in a ShellRegistry, which
// represents a collection of registered shell controls. One global instance is created
// within this library
class ShellRegistry
{
public:
    bool Register(const wxString &name, fnCreate create, fnFree free); //register/deregister are called by the plugin registrant instance
    bool Deregister(const wxString &name);
    ShellCtrlBase *CreateControl(const wxString &type,wxWindow* parent, int id, const wxString &windowname, ShellManager *shellmgr=NULL);
    void FreeControl(ShellCtrlBase *sh); //TODO: Don't think this is necessary?
private:
//    std::vector<ShellRegInfo> m_reginfo;
    std::map<wxString, ShellRegInfo> m_reginfo;
};

extern ShellRegistry& GlobalShellRegistry(); //defined in shellctrlbase.cpp, but accessible to all linking libraries

// every library that creates a new shell control must create an instance of this class with the type of their control as the template paramater T. This will add the new class to the registry of shell controls and the needed functions to create new instances
template<class T> class ShellCtrlRegistrant
{
    public:
        /// @param name The name of the ShellCtrl.
        ShellCtrlRegistrant(const wxString& name)
        {
            m_name=name;
            GlobalShellRegistry().Register(name, &Create, &Free);
        }
        ~ShellCtrlRegistrant()
        {
            GlobalShellRegistry().Deregister(m_name);
        }
        static ShellCtrlBase* Create(wxWindow* parent, int id, const wxString &windowname, ShellManager *shellmgr=NULL) //allocates new shell control object on heap
        {
            return new T(parent, id, windowname, shellmgr);
        }

        static void Free(ShellCtrlBase* sh) // deletes object from heap
        {
            delete sh;
        }
        wxString m_name;

//        static void SDKVersion(int* major, int* minor, int* release)
//        {
//            if (major) *major = PLUGIN_SDK_VERSION_MAJOR;
//            if (minor) *minor = PLUGIN_SDK_VERSION_MINOR;
//            if (release) *release = PLUGIN_SDK_VERSION_RELEASE;
//        }
};



// This is the shell control base class
// All controls derives from a basic wxPanel
// The control must offer services to create and destroy underlying processes
// terminal controls sit inside a tabbed panel managed by the Shell Manager
class ShellCtrlBase : public wxPanel //TODO: make wxPanel a member, not a base??
{
    public:
        ShellCtrlBase():wxPanel() {m_id=-1;}
        ShellCtrlBase(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr=NULL);
        virtual ~ShellCtrlBase() {}

        // Every shell control widget must override the following
        virtual long LaunchProcess(const wxString &processcmd, const wxArrayString &options)=0;
//        virtual void KillWindow()=0; // manager may destroy the window, but will call this before doing so
        virtual void KillProcess()=0; //use this to respond to ShellManager request to kill the process
        virtual void SyncOutput(int maxchars=1000)=0; //use this to respond to ShellManager request to gather output from the running process for display in the panel

        virtual bool IsDead()=0;
        wxString GetName() {return m_name;}
        void SetName(const wxString &name) {m_name=name;}
    protected:
        wxString m_name;
        ShellManager *m_shellmgr;
        int m_id;
//    DECLARE_DYNAMIC_CLASS(ShellCtrlBase)
//    DECLARE_EVENT_TABLE()
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHELL_ADD_CLICKED, -1)
END_DECLARE_EVENT_TYPES()


class ShellManager : public wxPanel
{
    friend class ShellCtrlBase;
    public:
        ShellManager(wxWindow* parent);
        ~ShellManager(); //virtual??
        long LaunchProcess(const wxString &processcmd, const wxString &name, const wxString &type, const wxArrayString &options);
        void KillProcess(int id);
        void KillWindow(int id);
        void RemoveDeadPages();
        ShellCtrlBase *GetPage(size_t i);
        ShellCtrlBase *GetPage(const wxString &name);
        void OnShellTerminate(ShellCtrlBase *term);
        int NumAlive();
    private:
        //Responders to standard wxWidgets Messages
        void OnUserInput(wxKeyEvent& ke);
        void OnPollandSyncOutput(wxTimerEvent& te);
        void OnPageClosing(wxAuiNotebookEvent& event);
        void OnPageChanging(wxAuiNotebookEvent& event);
        bool QueryClose(ShellCtrlBase* sh);
        //Responders to friend class ShellCtrlBase
        size_t GetTermNum(ShellCtrlBase *term);
    protected:
        wxTimer m_synctimer;
        wxAuiNotebook *m_nb;
    DECLARE_EVENT_TABLE()
};



#endif // SHELLCTRL_H
