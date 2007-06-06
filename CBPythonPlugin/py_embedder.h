#include <Python.h>

#ifndef PYEMBEDDER_H_INCLUDED
#define PYEMBEDDER_H_INCLUDED


class PyMgr
{
    PyMgr();
    ~PyMgr();
};

class PyInstance
{
    PyInstance();
    ~PyInstance();
    void Lock();
    void Release();
    void EvalString(char *str);
    PyThreadState *tstate;
};

#endif //PYEMBEDDER_H_INCLUDED
