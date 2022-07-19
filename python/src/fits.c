#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>

#include "gnuastro/fits.h"
#include "gnuastro/python.h"

#include <numpy/arrayobject.h>



static PyObject *
img_read(PyObject *self, PyObject *args, PyObject *keywds)
{
  char *fname, *hdu;
  PyObject *out = NULL;
  gal_data_t *image = NULL;
  // Default values of minmapsize and quietmap
  int minmapsize = -1, quietmap = 1;

  static char *kwlist[] = {"filename", "hdu", "minmapsize",
                           "quietmap", NULL};

  // Parsing the arguments
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "ss|ii", kwlist,
                                   &fname, &hdu, &minmapsize, &quietmap))
    return NULL;

  // Reading the image
  image = gal_fits_img_read(fname, hdu, minmapsize, quietmap);

  // Since dims needs to be a pointer to a const.
  npy_intp* const dims = (npy_intp *)image->dsize;

  out = PyArray_SimpleNewFromData(image->ndim, dims,
                                  gal_npy_datatype_to_type(image->type),
                                  (float *)image->array);

  return out;
}





static PyObject *
img_write(PyObject *self, PyObject *args)
{
  gal_data_t *data;
  PyArrayObject *data_arr = NULL;
  char *filename, *program_string;
  gal_fits_list_key_t *headers = NULL;
  PyObject *arg1 = NULL, *header_list=NULL;
  program_string = "FITS Program";

  if(!PyArg_ParseTuple(args,"Os|Os",&arg1, &filename, 
                       &header_list, &program_string))
    return NULL;
  // printf("Arguments parsed\n");

  data_arr = (PyArrayObject *)PyArray_FROM_OT(arg1, NPY_FLOAT32);
  // printf("Numpy Data Array initialized\n");

  data = gal_data_alloc(PyArray_DATA(data_arr), gal_npy_type_to_datatype(PyArray_TYPE(data_arr)),
                        PyArray_NDIM(data_arr), (size_t *)PyArray_DIMS(data_arr),
                        NULL, 0, -1, 1, NULL, NULL, NULL);
  // printf("gal_data_alloc succedded\n");

  gal_fits_img_write(data, filename, headers, program_string);

  // printf("%s created!\n",filename);

  gal_data_free(data);

  return Py_True;

}

static PyMethodDef FitsMethods[] = {
    {"img_read", (PyCFunction)(void (*)(void))img_read, METH_VARARGS, "Reads an image."},
    {"img_write", (PyCFunction)(void (*)(void))img_write, METH_VARARGS, "Writes an image."},
    {NULL, NULL, 0, NULL}, /* Sentinel */
};

static struct PyModuleDef fits = {
    PyModuleDef_HEAD_INIT,
    "fits",
    "FITS Module",
    -1,
    FitsMethods
};


PyMODINIT_FUNC
PyInit_fits(void)
{
  PyObject *module;
  module = PyModule_Create(&fits);
  if(module==NULL) return NULL;

  import_array();

  if(PyErr_Occurred()) return NULL;

  return module;
}

int main(int argc, char *argv[])
{
  wchar_t *program = Py_DecodeLocale(argv[0], NULL);
  if (program == NULL)
  {
    fprintf(stderr, "Fatal error : cannot decode argv[0]\n");
    exit(1);
  }

  /* Add a built-in module before Py_initialize. */
  if (PyImport_AppendInittab("fits", PyInit_fits) == -1)
  {
    fprintf(stderr, "Error: could not extend in-built modules table\n");
    exit(1);
  }

  /* Pass argv[0] to the Python interpreter */
  Py_SetProgramName(program);

  /* Initialize the Python interpreter.  Required.
     If this step fails, it will be a fatal error. */
  Py_Initialize();
  /* Optionally import the module; alternatively,
     import can be deferred until the embedded script
     imports it. */
  char mod_name[] = "fits";
  PyObject *pmodule = PyImport_ImportModule("fits");
  if (!pmodule)
  {
    PyErr_Print();
    fprintf(stderr, "Error: could not import module %s\n", mod_name);
  }

  PyMem_RawFree(program);
  return 0;
}