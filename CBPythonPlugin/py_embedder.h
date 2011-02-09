#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dynarray.h>
#include <wx/process.h>
#include <wx/process.h>

//#include <memory>
#include "PyEvents.h"
#include <iostream>
#include "XmlRpc.h"

class PyJob;
class PyInstance;

//////////////////////////////////////////////////////
// PyJob: An abstract class for a python job to be
// run by an interpreter instance. The job is defined
// in the pure virtual method operator(). The job
// should try to restrict itself to writing to its
// own non-GUI data members for thread safety.
//////////////////////////////////////////////////////
class PyJob: public wxThread
{
public:
    PyJob(PyInstance *pyinst, wxWindow *p, int id, bool selfdestroy=true);
    virtual ~PyJob();
    void Abort();
    virtual bool operator()()=0;// {return false;}
protected:
    virtual void *Entry();
    PyInstance *pyinst;
    wxWindow *parent;
    int id;
    bool finished;
    bool started;
    bool killonexit;
    friend class PyInstance;
//    DECLARE_DYNAMIC_CLASS(PyJob);
};

WX_DECLARE_LIST(PyJob, PyJobQueue);

/////////////////////////////////////////////////////////////////////////////////////
// PyInstance: The interface to an instance of a running interpreter
// each instantance launches an external python process then
// connects via some sort of socket interface (xml-rpc currently)
// The interface maintains a queue of jobs for the interpreter,
// which are run in sequence as worker threads. a job is a single/multiple
// xml-rpc method request
// jobs must not interact with objects, esp. gui objects, on the main thread.
/////////////////////////////////////////////////////////////////////////////////////
class PyInstance: public wxEvtHandler
{
    friend class PyJob;
public:
    PyInstance(const wxString &processcmd, const wxString &hostaddress, int port, wxWindow *parent=NULL);
    long LaunchProcess(const wxString &processcmd);
    PyInstance(const PyInstance &copy);
    ~PyInstance();
    void EvalString(char *str, bool wait=true);
    long GetPid() {if(m_proc) return m_proc_id; else return -1;}
    void KillProcess(bool force=false);
    void Break();
    bool AddJob(PyJob *job);
    void OnJobNotify(wxCommandEvent &event);
    void PauseJobs();
    void ClearJobs();
    bool IsJobRunning() {return m_jobrunning;}
    bool IsDead() {return m_proc_dead;}
    bool Exec(const wxString &method, XmlRpc::XmlRpcValue &inarg, XmlRpc::XmlRpcValue &result);
    void OnEndProcess(wxProcessEvent &event);
private:
    wxWindow *m_parent;
    wxMutex exec_mutex;
    wxProcess *m_proc; // external python process
    long m_proc_id;
    int  m_proc_killlevel;
    bool m_proc_dead;
    PyJobQueue m_queue;
    bool m_paused; //if paused is true, new jobs in the queue will not be processed automatically
    wxString m_hostaddress; //address for python server process
    int m_port; // port number for server
    bool m_jobrunning;
    XmlRpc::XmlRpcClient *m_client;
    //void AttachExtension(); //attach a python extension table as an import for this interpreter
    //DECLARE_DYNAMIC_CLASS(PyInstance)
    DECLARE_CLASS(PyInstance)
    DECLARE_EVENT_TABLE()
};

WX_DECLARE_OBJARRAY(PyInstance, PyInstanceCollection);

/////////////////////////////////////////////////////////////////////////////////////
// PyMgr: manages the collection of interpreters
/////////////////////////////////////////////////////////////////////////////////////
class PyMgr
{
public:
    PyInstance *LaunchInterpreter();
    static PyMgr &Get();
    ~PyMgr();
protected:
    PyMgr();
private:
    PyInstanceCollection m_Interpreters;
    static std::auto_ptr<PyMgr> theSingleInstance;
// todo: create an xmlrpc server?

};


#endif //PYEMBEDDER_H_INCLUDED
