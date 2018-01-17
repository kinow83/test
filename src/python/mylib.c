#include <Python.h>
#include <stdio.h>


static PyObject *ErrorObject;

static PyObject* write_log(PyObject *self, PyObject *args)
{
	char *msg;
	FILE *fp;

	if ( !PyArg_ParseTuple( args, "s", &msg ) ) {
		return NULL;
	}

	fp = fopen("/tmp/pylog.txt", "a+");
	fprintf(fp, "%s", msg);
	fclose(fp);

	return Py_BuildValue( "i", 0);
}

static struct PyMethodDef methods[] = {
	{"wlog", write_log, METH_VARARGS},
	{NULL, NULL},
};

void initmylib()
{
	PyObject *m;
	m = Py_InitModule("mylib", methods);

	ErrorObject = Py_BuildValue("s", "error");
}


