#ifndef SHELLCTRL_H
#define SHELLCTRL_H

#include <map>


#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif


#include <wx/process.h>
#include <wx/wxFlatNotebook/wxFlatNotebook.h>
#include <sdk.h>

// CLASSES DEFINED IN THIS LIBRARY
class ShellManager; //Manages the collection of Shell Control Widgets allowing user interaction with external processes within a tabbed notepage - usually the main app (or some plugin) will declare a global instance of this manager. See the full declaration below for more detail
class ShellCtrlBase; //The manager manages a set of Shell Control widgets that redirect I/O from an external process - ShellCtrlBase is an abstract base class allowing the developer to create their own custom controls for handling I/O from their process. Note that "process" is not defined here, it could be a new thread, an external program, a server connection etc
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
    bool Register(const wxString &name, fnCreate create, fnFree free) //register/deregister are called by the plugin registrant instance
    {
        std::map<wxString, ShellRegInfo>::iterator it;
        if(m_reginfo.find(name)!=m_reginfo.end())
            return false;
        ShellRegInfo sri;
        sri.create=create;
        sri.free=free;
        m_reginfo[name]=sri;
        return true;
    }
    bool Deregister(const wxString &name)
    {
        std::map<wxString, ShellRegInfo>::iterator it
            =m_reginfo.find(name);
        if(it==m_reginfo.end())
            return false;
        m_reginfo.erase(it);
        return true;
    }
    ShellCtrlBase *CreateControl(const wxString &type,wxWindow* parent, int id, const wxString &windowname, ShellManager *shellmgr=NULL)
    {
        std::map<wxString, ShellRegInfo>::iterator it
            =m_reginfo.find(type);
        if(it==m_reginfo.end())
            return NULL;
        return it->second.create(parent, id, windowname, shellmgr);
    }
    void FreeControl(ShellCtrlBase *sh) //TODO: Don't think this is necessary?
    {
//        std::map<wxString, ShellRegInfo>::iterator it
//            =m_reginfo.find(type);
//        if(it!=m_reginfo.end())
//            it.second->free(); //TODO: Can't compile
    }

//    std::vector<ShellRegInfo> m_reginfo;
    std::map<wxString, ShellRegInfo> m_reginfo;
};

extern ShellRegistry GlobalShellRegistry; //defined in shellctrlbase.cpp, but accessible to all linking libraries

// every library that creates a new shell control must create an instance of this class with the type of their control as the template paramater T. This will add the new class to the registry of shell controls and the needed functions to create new instances
template<class T> class ShellCtrlRegistrant
{
    public:
        /// @param name The name of the ShellCtrl.
        ShellCtrlRegistrant(const wxString& name)
        {
            m_name=name;
            GlobalShellRegistry.Register(name, &Create, &Free);
        }
        ~ShellCtrlRegistrant()
        {
            GlobalShellRegistry.Deregister(m_name);
        }
        static ShellCtrlBase* Create(wxWindow* parent, int id, const wxString &windowname, ShellManager *shellmgr=NULL) //allocates new shell control object on heap
        {
            return new T;
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
// All controls derives from a basic wxWindow
// The control must offer services to create and destroy underlying processes
// terminal controls sit inside a tabbed panel managed by the Shell Manager
class ShellCtrlBase : public wxWindow //TODO: make wxWindow a member, not a base??
{
    public:
        //ShellCtrlBase() {m_dead=true; m_id=-1}
        ShellCtrlBase(wxWindow* parent, int id, const wxString &name, ShellManager *shellmgr=NULL)
                : wxWindow(parent, id)
            {m_parent=parent;
             m_id=id; m_dead=true;
             m_shellmgr=shellmgr; }
        virtual ~ShellCtrlBase();

        // Every shell control widget must override the following
        virtual long LaunchProcess(const wxString &processcmd, const wxArrayString &options)=0;
        virtual void KillFrame()=0; // manager may destroy the window, but will call this before doing so
        virtual void KillProcess()=0;
        virtual void SyncOutput(int maxchars=1000)=0; //use this to respond to ShellManager request to gather output from the running process for display in the frame

        wxString GetName() {return m_name;}
        void SetName(const wxString &name) {m_name=name;}
        bool IsDead() {return m_dead;}
    private:
        wxString m_name;
    protected:
        ShellManager *m_shellmgr;
        bool m_dead;
        int m_id;
    DECLARE_DYNAMIC_CLASS(wxFrame)
//    DECLARE_EVENT_TABLE()
};

// abstract base class to manage collections of shell terms
//class ShellManager
//{
//    friend class ShellCtrlBase;
//    public:
//        virtual ~ShellManager();
//    private:
//        virtual void OnShellTerminate(ShellCtrlBase *term); //notifies manager of shell termination
//};


class ShellManager : public wxPanel
{
    friend class ShellCtrlBase;
    public:
        ShellManager(wxWindow* parent);
        ~ShellManager(); //virtual??
        long LaunchProcess(const wxString &processcmd, const wxString &name, const wxString &type, const wxArrayString &options);
        void KillProcess(int id);
        void KillWindow(int id);
        ShellCtrlBase *GetPage(size_t i);
        ShellCtrlBase *GetPage(const wxString &name);
        int NumAlive();
    private:
        //Responders to standard wxWidgets Messages
        void OnUserInput(wxKeyEvent& ke);
        void OnPollandSyncOutput(wxTimerEvent& te);
        void OnPageClosing(wxFlatNotebookEvent& event);
        bool QueryClose(ShellCtrlBase* sh);
        //Responders to friend class ShellCtrlBase
        void OnShellTerminate(ShellCtrlBase *term);
        size_t GetTermNum(ShellCtrlBase *term);
    protected:
        wxTimer m_synctimer;
        wxFlatNotebook *m_nb;
    DECLARE_EVENT_TABLE()
};



#endif // SHELLCTRL_H
