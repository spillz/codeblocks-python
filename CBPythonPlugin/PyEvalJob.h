#ifndef PYEVALJOB_H
#define PYEVALJOB_H

#include "py_embedder.h"

class PyTalker
{
public:
    PyTalker();
    ~PyTalker();
}


class PyEvalJob : public PyJob
{
public:
    PyEvalJob();
    ~PyEvalJob();
    bool operator()();
    wxString &Stderr() {m_errmutex.Lock();}
    void ReleaseStderr() {m_errmutex.Unlock(); return m_stderr;}
    wxString &Stdout() {m_outmutex.Lock();return m_stdout;}
    void ReleaseStdout() {m_outmutex.Unlock();}
    void writestdout(const wxString &s) {m_outmutex.Lock();m_stdout+=s;m_outmutex.Unlock();}
    void writestderr(const wxString &s) {m_errmutex.Lock();m_stderr+=s;m_errmutex.Unlock();}
protected:
private:
    wxString m_jobstr;
    wxString m_stdout;
    wxString m_stderr;
    wxMutex m_outmutex;
    wxMutex m_errmutex;
    PyMethodDef PyCBMethods[3];
};


#endif // PYEVALJOB_H
