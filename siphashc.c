/* 
 * Copyright (c) 2013 Eli Janssen
 * Copyright (c) 2014 Carlo Pires
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
**/

#include <stdlib.h>
#include <string.h>
#include <Python.h>
#include "siphash/siphash.h"

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
	#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
	#define GETSTATE(m) (&_state)
	static struct module_state _state;
#endif

static PyObject *pysiphash(PyObject *self, PyObject *args) {
    const char *key = NULL;
    int key_sz;
    const char *plaintext = NULL;
    int plain_sz;
    uint64_t hash;

    if (!PyArg_ParseTuple(
            args, "s#s#:siphash",
            &key, &key_sz, &plaintext, &plain_sz)) {
        return NULL;
    }

    if (key_sz != 16) {
        PyErr_SetString(
            PyExc_ValueError,
            "key must be exactly 128 bits long (16 chars)");
        return NULL;
    }

    hash = siphash(
        (const unsigned char*)key,
        (const unsigned char*)plaintext,
        plain_sz);

    return PyLong_FromUnsignedLongLong(hash);
}

static char siphash_docstring[] = ""
    "Computes Siphash-2-4 of the given string and key\n\n"
    "siphash(key, plaintext) -> hash\n"
    " - key: must be 128 bit long (16 chars at 8 bit each)\n"
    " - plaintext: text\n"
    "returns 64-bit output (python Long)\n";

static PyMethodDef siphashc_methods[] = {
    {"siphash", pysiphash, METH_VARARGS, siphash_docstring},
    {NULL, NULL, 0, NULL} /* sentinel */
};

#if PY_MAJOR_VERSION >= 3
	static int siphashc_traverse(PyObject *m, visitproc visit, void *arg) {
		Py_VISIT(GETSTATE(m)->error);
		return 0;
	}

	static int siphashc_clear(PyObject *m) {
		Py_CLEAR(GETSTATE(m)->error);
		return 0;
	}

	static struct PyModuleDef moduledef = {
		    PyModuleDef_HEAD_INIT,
		    "siphashc",
		    NULL,
		    sizeof(struct module_state),
		    siphashc_methods,
		    NULL,
		    siphashc_traverse,
		    siphashc_clear,
		    NULL
	};

	#define INITERROR return NULL

	PyObject *
	PyInit_siphashc(void)
#else
	#define INITERROR return

	void
	initsiphashc(void)
#endif
{
	struct module_state *st;

#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
    PyObject *module = Py_InitModule("siphashc", siphashc_methods);
#endif

    if (module == NULL)
        INITERROR;

    st = GETSTATE(module);
    st->error = PyErr_NewException("siphashc.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}

