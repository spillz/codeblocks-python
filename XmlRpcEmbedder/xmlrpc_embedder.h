#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED

#include <wx/wx.h>
#include <wx/app.h>
//#include <wx/dynarray.h>
#include <wx/process.h>

#include <memory>
#include <iostream>
#include "XmlRpcEvents.h"
#include "XmlRpc.h"

class XmlRpcJob;
class XmlRpcInstance;

//////////////////////////////////////////////////////
// XmlRpcJob: An abstract class for a python job to be
// run by an interpreter instance. The job is defined
// in the pure virtual method operator(). The job
// should try to restrict itself to writing to its
// own non-GUI data members for thread safety.
//////////////////////////////////////////////////////
class XmlRpcJob: public wxThread
{
public:
    XmlRpcJob(XmlRpcInstance *xmlrpc_instance, wxEvtHandler *p, int id, bool selfdestroy=true);
    virtual ~XmlRpcJob();
    void Abort();
    virtual bool operator()()=0;// {return false;}
protected:
    virtual void *Entry();
    XmlRpcInstance *xmlrpc_instance;
    wxEvtHandler *parent;
    int id;
    bool finished;
    bool started;
    bool killonexit;
    friend class XmlRpcInstance;
//    DECLARE_DYNAMIC_CLASS(XmlRpcJob);
};

WX_DECLARE_LIST(XmlRpcJob, XmlRpcJobQueue);

class XmlRpcPipeClient;

/////////////////////////////////////////////////////////////////////////////////////
// XmlRpcInstance: The interface to an instance of a runninng XMLRPC server
// each instance launches an external server process then
// connects to the server via XMLRPC
// The interface maintains a queue of jobs for the interpreter,
// which are run in sequence as worker threads. a job is a single/multiple
// xml-rpc method request
// jobs must not interact with objects, esp. gui objects, on the main thread.
/////////////////////////////////////////////////////////////////////////////////////
class XmlRpcInstance: public wxEvtHandler
{
    friend class XmlRpcJob;
public:
    // Standard constructor -- generally should use the Manager instance to create these
    XmlRpcInstance(const wxString &processcmd, int port=-1, const wxString &hostaddress=_T("localhost"), wxWindow *parent=NULL);

    // Copy constructor
    XmlRpcInstance(const XmlRpcInstance &copy);

    // Destructor
    ~XmlRpcInstance();

    // Returns the unique system PID of the server process
    long GetPid() {if(m_proc) return m_proc_id; else return -1;}

    // Kills the server process (need to use force on windows)
    void KillProcess(bool force=false);

    // Send a break signal to the process (probably not all that useful)
    void Break();

    // Queue a Job that will run custom XMLRPC requests asynchously
    bool AddJob(XmlRpcJob *job);

    // Stop threaded jobs from being process
    void PauseJobs();

    // Clear all threaded jobs that have not started yet
    void ClearJobs();

    // Returns true if a job is active
    bool IsJobRunning() {return m_jobrunning;}

    // Returns true if the process is no longer active
    bool IsDead() {return m_proc_dead;}

    // Make a synchronous XMLRPC call to the server
    bool Exec(const wxString &method, const XmlRpc::XmlRpcValue &inarg, XmlRpc::XmlRpcValue &result);

    // Make a synchronous XMLRPC call to the server
    bool ExecAsync(const wxString &method, const XmlRpc::XmlRpcValue &inarg, wxEvtHandler *hndlr, int id=wxID_ANY);
protected:
    // XmlRpcJobs threads send a signal that is processed by this thread on completion
    void OnJobNotify(wxCommandEvent &event);

    // Signal processed if the server process terminates
    void OnEndProcess(wxProcessEvent &event);

    // Retrieves the next threaded job from the queue and starts it
    bool NextJob();
private:
    // Deletes process object and empties queue
    void CleanupTerminatedProcess();
    // Called by the constructor to actually start the process running
    long LaunchProcess(const wxString &processcmd);
    wxWindow *m_parent;
    wxMutex exec_mutex;
    wxProcess *m_proc; // handle to XMLRPC server process
    long m_proc_id;
    int  m_proc_killlevel;
    bool m_proc_dead;
    XmlRpcJobQueue m_queue;
    bool m_paused; //if paused is true, new jobs in the queue will not be processed automatically
    wxString m_hostaddress; //address for python server process
    int m_port; // port number for server
    bool m_jobrunning;
    XmlRpc::XmlRpcClient *m_client;
    XmlRpcPipeClient *m_pipeclient;
    DECLARE_CLASS(XmlRpcInstance)
    DECLARE_EVENT_TABLE()
};

//WX_DECLARE_OBJARRAY(XmlRpcInstance, XmlRpcInstanceCollection);
//
///////////////////////////////////////////////////////////////////////////////////////
//// XmlRpcMgr: manages the collection of interpreters
///////////////////////////////////////////////////////////////////////////////////////
//class XmlRpcMgr
//{
//public:
//    XmlRpcInstance *LaunchProcess(const wxString &cmd,int port=-1,const wxString &address=_("localhost"));
//    void Remove(XmlRpcInstance *p);
//    static XmlRpcMgr &Get();
//    ~XmlRpcMgr();
//protected:
//    XmlRpcMgr();
//private:
//    XmlRpcInstanceCollection m_Interpreters;
//    static std::auto_ptr<XmlRpcMgr> theSingleInstance;
//// todo: create an xmlrpc server?
//
//};
//

#endif //PYEMBEDDER_H_INCLUDED
