#include "PyEvalJob.h"

PyObject *PyEvalJob::cb_writestdout(PyObject *self, PyObject *args)
{
    const wxChar *output;
    int sts;

    if (!PyArg_ParseTuple(args, "u", &command))
        return NULL;
    sts = system(command);
    return Py_BuildValue("i", sts);
}

PyEvalJob::PyEvalJob(wxWindow *p, int id, const wxString &evalstr): PyJob(p,id,false);
{
    m_jobstr=evalstr;
    PyCBMethods[0] =
    {"writeoutput",  cb_writestdout, METH_VARARGS,"Write to redirected stdout."};
    PyCBMethods[1] =
    {"readinput",  cb_readstdin, METH_VARARGS,"Read from redirected stdin."};
    PyCBMethods[2] =
    {NULL, NULL, 0, NULL};
    Py_InitModule("cb", PyCBMethods);
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
    //need to release the module here...
}
