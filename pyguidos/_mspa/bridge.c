/***********************************************************************
 * pyGuidos MSPA bridge
 *
 * Thin Python C-API + NumPy C-API shim exposing the miallib MSPA engine
 * (segmentBinaryPatterns) to Python without any file I/O. Input and output
 * cross the boundary as NumPy uint8 arrays; the embedded GTB colormap is
 * returned as a separate NumPy array.
 *
 * The MSPA algorithm itself (segmentBinaryPatterns and all miallib
 * primitives it uses) is the original, unmodified code by Pierre Soille
 * and Peter Vogt (miallib, GPL v3). This bridge only marshals data.
 *
 * Copyright (C) European Union (Joint Research Centre).
 * Distributed under the GNU General Public License v3 (see miallib COPYING).
 ***********************************************************************/

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <string.h>

#include "miallib/miallib.h"
#include "miallib/mialtypes.h"

/* Forward declaration of the MSPA entry point (defined in miallib/mspa.c). */
extern IMAGE *segmentBinaryPatterns(IMAGE *imin, float size, int graphfg,
                                    int transition, int internal);

/*
 * mspa(input, size, graphfg, transition, internal)
 *
 *   input     : 2D uint8 NumPy array, C-contiguous.
 *               Pixel convention (miallib fm_preproc):
 *                 0 = missing/nodata, 1 = background, 2 = foreground.
 *   size      : float, edge width (MSPA "size" parameter).
 *   graphfg   : int, foreground connectivity (8 or 4).
 *   transition: int, 1 to enable transition classes, else 0.
 *   internal  : int, 1 to enable internal (hole) analysis, else 0.
 *
 * Returns a tuple (output_array, colormap_array):
 *   output_array : 2D uint8 NumPy array, same shape as input (MSPA classes).
 *   colormap_array : (256, 3) uint16 NumPy array (R,G,B per class index),
 *                    read from the embedded miallib LUT (0..65535 range),
 *                    or None if the result carries no LUT.
 */
static PyObject *py_mspa(PyObject *self, PyObject *args)
{
    PyObject *in_obj = NULL;
    float size;
    int graphfg, transition, internal;

    if (!PyArg_ParseTuple(args, "Ofiii", &in_obj, &size,
                          &graphfg, &transition, &internal)) {
        return NULL;
    }

    /* Require a C-contiguous, aligned, 2D uint8 array. */
    PyArrayObject *in_arr = (PyArrayObject *)PyArray_FROMANY(
        in_obj, NPY_UINT8, 2, 2,
        NPY_ARRAY_IN_ARRAY /* C-contiguous + aligned + not swapped */);
    if (in_arr == NULL) {
        return NULL;
    }

    npy_intp nrows = PyArray_DIM(in_arr, 0);
    npy_intp ncols = PyArray_DIM(in_arr, 1);

    if (nrows <= 0 || ncols <= 0) {
        Py_DECREF(in_arr);
        PyErr_SetString(PyExc_ValueError, "MSPA input must be a non-empty 2D array");
        return NULL;
    }

    /* Build a miallib IMAGE (UCHAR) and copy the input pixels in. miallib
       owns this buffer; we copy so NumPy and miallib never alias. */
    IMAGE *imin = create_image(t_UCHAR, (long int)ncols, (int)nrows, 1);
    if (imin == NULL) {
        Py_DECREF(in_arr);
        PyErr_NoMemory();
        return NULL;
    }

    {
        /* PyArray_FROMANY guaranteed C-contiguous, so a flat copy is valid.
           miallib stores images row-major (nx = ncols fastest varying). */
        const npy_intp npix = nrows * ncols;
        memcpy(GetImPtr(imin), PyArray_DATA(in_arr), (size_t)npix * sizeof(UCHAR));
    }

    Py_DECREF(in_arr);

    /* Release the GIL for the (potentially long) C computation. The C code
       operates only on its own miallib allocations, so this is safe. */
    IMAGE *out = NULL;
    Py_BEGIN_ALLOW_THREADS
    out = segmentBinaryPatterns(imin, size, graphfg, transition, internal);
    Py_END_ALLOW_THREADS

    free_image(imin);

    if (out == NULL) {
        PyErr_SetString(PyExc_RuntimeError,
                        "segmentBinaryPatterns() failed (returned NULL)");
        return NULL;
    }

    /* The output must be a UCHAR image of the same shape. */
    if (GetImDataType(out) != t_UCHAR ||
        (npy_intp)GetImNx(out) != ncols ||
        (npy_intp)GetImNy(out) != nrows) {
        free_image(out);
        PyErr_SetString(PyExc_RuntimeError,
                        "MSPA output has unexpected type or dimensions");
        return NULL;
    }

    /* Copy output pixels into a fresh NumPy uint8 array. */
    npy_intp dims[2];
    dims[0] = nrows;
    dims[1] = ncols;
    PyArrayObject *out_arr =
        (PyArrayObject *)PyArray_SimpleNew(2, dims, NPY_UINT8);
    if (out_arr == NULL) {
        free_image(out);
        return NULL;
    }
    {
        const npy_intp npix = nrows * ncols;
        memcpy(PyArray_DATA(out_arr), GetImPtr(out),
               (size_t)npix * sizeof(UCHAR));
    }

    /* Extract the embedded colormap (LUT). miallib stores it as USHORT with
       256 entries per channel: lut[i]=R, lut[i+256]=G, lut[i+512]=B. */
    PyObject *cmap_obj = Py_None;
    Py_INCREF(Py_None);

    USHORT *lut = GetImLut(out);
    if (lut != NULL) {
        npy_intp cdims[2];
        cdims[0] = 256;
        cdims[1] = 3;
        PyArrayObject *cmap_arr =
            (PyArrayObject *)PyArray_SimpleNew(2, cdims, NPY_UINT16);
        if (cmap_arr == NULL) {
            Py_DECREF(cmap_obj);
            free_image(out);
            Py_DECREF(out_arr);
            return NULL;
        }
        npy_uint16 *cptr = (npy_uint16 *)PyArray_DATA(cmap_arr);
        for (int i = 0; i < 256; ++i) {
            cptr[i * 3 + 0] = (npy_uint16)lut[i];         /* R */
            cptr[i * 3 + 1] = (npy_uint16)lut[i + 256];   /* G */
            cptr[i * 3 + 2] = (npy_uint16)lut[i + 512];   /* B */
        }
        Py_DECREF(cmap_obj);          /* drop the initial Py_None */
        cmap_obj = (PyObject *)cmap_arr;
    }

    free_image(out);

    return Py_BuildValue("(NN)", (PyObject *)out_arr, cmap_obj);
}

static PyMethodDef mspa_methods[] = {
    {"mspa", py_mspa, METH_VARARGS,
     "mspa(input_uint8_2d, size, graphfg, transition, internal) -> "
     "(output_uint8_2d, colormap_uint16_256x3_or_None)\n\n"
     "Run miallib MSPA (segmentBinaryPatterns) on a binary pattern image.\n"
     "Input convention: 0=missing, 1=background, 2=foreground."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mspa_module = {
    PyModuleDef_HEAD_INIT,
    "_mspa",
    "Internal compiled MSPA engine for pyGuidos (wraps miallib, GPL v3).",
    -1,
    mspa_methods,
    NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit__mspa(void)
{
    PyObject *m = PyModule_Create(&mspa_module);
    if (m == NULL) {
        return NULL;
    }
    import_array();   /* initialise the NumPy C-API */
    if (PyErr_Occurred()) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
