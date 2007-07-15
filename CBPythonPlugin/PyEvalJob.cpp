#include "PyEvalJob.h"


PyEvalJob::PyEvalJob(wxWindow *p, int id, const wxString &evalstr): PyJob(p,id,false);
{
    m_jobstr=evalstr;
}

bool PyEvalJob::operator()()
{
    Lock();
    // send commands to the inerpreter to redirect the output to a routine that will fill m_stderr and m_stdout
    // and request input from m_stdin
    PyRun_SimpleString(m_jobstr.c_str());
    Release();
}

PyEvalJob::~PyEvalJob()
{
}


