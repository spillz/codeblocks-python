#include <wx/wx.h>
#include <Python.h>

#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED


class PyMgr
{
    PyMgr();
    ~PyMgr();
    PyInstance *LaunchInterpreter();
private:
    PyInstance m_Interpreters;
};

class PyInstance
{
    PyInstance();
    ~PyInstance();
    void Lock();
    void Release();
    void EvalString(char *str);
    PyThreadState *tstate;
    wxThread *m_thread;
    // wxString m_commandqueue;
    //void AttachExtension(); //attach a python extension table as an import for this interpreter
};

#endif //PYEMBEDDER_H_INCLUDED
