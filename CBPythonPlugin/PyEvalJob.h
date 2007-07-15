#ifndef PYEVALJOB_H
#define PYEVALJOB_H

#include "py_embedder.h"

class PyEvalJob : public PyJob
{
public:
    PyEvalJob();
    ~PyEvalJob();
    bool operator()();
    wxString &Stderr() {m_errmutex.Lock();}
    void ReleaseStderr() {m_errmutex.Unlock();}
    wxString &Stdout() {m_outmutex.Lock();return }
    void ReleaseStdout() {m_outmutex.Unlock();}
protected:
private:
    wxString m_jobstr;
    wxString m_stdout;
    wxString m_stderr;
    wxMutex m_outmutex;
    wxMutex m_errmutex;
};



#endif // PYEVALJOB_H
