#include "py_embedder.h"
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(PyJobQueue);

using namespace std;

PyMgr::PyMgr()
{
    Py_Initialize();// need to init threads?
}

PyMgr::~PyMgr()
{
    Py_Finalize();
}

PyInstance::PyInstance()
{
    PyEval_AcquireLock();
    tstate=Py_NewInterpreter();
    PyEval_ReleaseLock();
}

PyInstance::~PyInstance()
{
    PyEval_AcquireLock();
    Py_EndInterpreter(tstate);
    PyEval_ReleaseLock();
}


// Not sure about this locking stuff...
void PyInstance::Lock()
{
    PyEval_AcquireThread(tstate);
}

void PyInstance::EvalString(char *str)
{
    PyRun_SimpleString(str);
}

void PyInstance::Release()
{
    PyEval_ReleaseThread(tstate);
}



void exec_pycode(const char* code)
{
  PyRun_SimpleString(code);
}


void exec_interactive_interpreter(int argc, char** argv)
{
  Py_Initialize();
  Py_Main(argc, argv);
  Py_Finalize();
}


void process_expression(char* filename,int num,char** exp)
{
    FILE*       exp_file;
    // Initialize a global variable for
    // display of expression results
    PyRun_SimpleString("x = 0");
    // Open and execute the file of
    // functions to be made available
    // to user expressions
    exp_file = fopen(filename, "r");
//    PyRun_SimpleFile(exp_file, exp);
    // Iterate through the expressions
    // and execute them
    while(num--) {
        PyRun_SimpleString(*exp++);
        PyRun_SimpleString("print x");
    }
}
int main2(int argc, char** argv)
{
    Py_Initialize();
    if(argc != 3) {
        printf("Usage: \%s FILENAME EXPRESSION+\n");
        return 1;
    }
    process_expression(argv[1], argc - 1, argv + 2);
    return 0;
}

int main1(int argc, char** argv)
{
//	cout << "Starting python shell..." << endl;
	exec_interactive_interpreter(argc, argv);

	return 0;
}
