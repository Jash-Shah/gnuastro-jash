#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "gnuastro/cosmology.h"





#define H0_DEFAULT 67.66
#define OLAMBDA_DEFAULT 0.6889
#define OMATTER_DEFAULT 0.3111
#define ORADIATION_DEFAULT 0.000

 /* The names of the arguments as a static array.
    So that they can be accessed as keyword arguments. */
// static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};



















// Functions
// =========
static PyObject
*velocity_from_z(PyObject *self, PyObject *args)
{
  double z, vel;

  if (!PyArg_ParseTuple(args, "d", &z))
    return NULL;

  vel = gal_cosmology_velocity_from_z(z);

  return PyFloat_FromDouble(vel);
}





static PyObject *proper_distance(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_proper_distance(z, H0, o_lambda_0, o_matter_0,
                                      o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyObject *comoving_volume(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};

  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */                               
    return NULL;

  res = gal_cosmology_comoving_volume(z, H0, o_lambda_0, o_matter_0,
                                      o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyObject *critical_density(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};
  
  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_critical_density(z, H0, o_lambda_0, o_matter_0,
                                       o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyObject *angular_distance(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};
  
  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_angular_distance(z, H0, o_lambda_0, o_matter_0,
                                       o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyObject *luminosity_distance(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};
  
  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_luminosity_distance(z, H0, o_lambda_0, o_matter_0,
                                          o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyObject *distance_modulus(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};
  
  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_distance_modulus(z, H0, o_lambda_0, o_matter_0,
                                       o_radiation_0);

  return PyFloat_FromDouble(res);

}





static PyObject *z_from_velocity(PyObject *self, PyObject *args)
{
  double z, vel;

  if (!PyArg_ParseTuple(args, "d", &vel))
    return NULL;

  z = gal_cosmology_z_from_velocity(vel);

  return PyFloat_FromDouble(z);
}





static PyObject *to_absolute_mag(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};
  
  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_to_absolute_mag(z, H0, o_lambda_0, o_matter_0,
                                      o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyObject *age(PyObject *self, PyObject *args, PyObject *keywds)
{
  double z, res;
  double H0 = H0_DEFAULT;
  double o_lambda_0 = OLAMBDA_DEFAULT;
  double o_matter_0 = OMATTER_DEFAULT;
  double o_radiation_0 = ORADIATION_DEFAULT;

  // static char *kwlist[] = {"z", "H0", "olambda", "omatter", "oradiation", NULL};
  
  // "d|ddd" indicates that only the first argument
  // i.e z is the required, and rest are optional args.
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "d|dddd", kwlist,
                                   &z, &H0, &o_lambda_0, &o_matter_0,
                                   &o_radiation_0))
    /* the arguments passed don't correspond to the signature
           described */
    return NULL;

  res = gal_cosmology_age(z, H0, o_lambda_0, o_matter_0,
                          o_radiation_0);

  return PyFloat_FromDouble(res);
}





static PyMethodDef
CosmologyMethods[] = {
                      {
                        "age",
                       (PyCFunction)(void (*)(void))age,
                       METH_VARARGS | METH_KEYWORDS,
                       "Returns the age of the universe at redshift z in units of Giga years."
                      },
                      {
                        "proper_distance",
                        (PyCFunction)(void (*)(void))proper_distance,
                        METH_VARARGS | METH_KEYWORDS,
                        "Returns the proper distance to an object at redshift z in units of Mega parsecs."
                      },
                      {
                        "comoving_volume",
                        (PyCFunction)(void (*)(void))comoving_volume,
                        METH_VARARGS | METH_KEYWORDS,
                        "Returns the comoving volume over 4pi stradian to z in units of Mega parsecs cube."
                      },
                      {
                        "critical_density",
                        (PyCFunction)(void (*)(void))critical_density,
                        METH_VARARGS | METH_KEYWORDS,
                        "Returns the critical density at redshift z in units of g/cm3."
                      },
                      {
                        "angular_distance",
                        (PyCFunction)(void (*)(void))angular_distance,
                        METH_VARARGS | METH_KEYWORDS,
                        "Return the angular diameter distance to an object at redshift z in units of Mega parsecs."
                      },
                      {
                        "luminosity_distance",
                        (PyCFunction)(void (*)(void))luminosity_distance,
                        METH_VARARGS | METH_KEYWORDS,
                        "Return the luminosity diameter distance to an object at redshift z in units of Mega parsecs."
                      },
                      {
                        "distance_modulus",
                        (PyCFunction)(void (*)(void))distance_modulus,
                        METH_VARARGS | METH_KEYWORDS,
                        "Return the distance modulus at redshift z (with no units)."
                      },
                      {
                        "to_absolute_mag",
                        (PyCFunction)(void (*)(void))to_absolute_mag,
                        METH_VARARGS | METH_KEYWORDS,
                        "Return the conversion from apparent to absolute magnitude for an object at redshift z. This value has to be added to the apparent magnitude to give the absolute magnitude of an object at redshift z."
                      },
                      {
                        "velocity_from_z",
                        velocity_from_z,
                        METH_VARARGS,
                        "Return the velocity (in km/s) corresponding to the given redshift (z)."
                      },
                      {
                        "z_from_velocity",
                        z_from_velocity,
                        METH_VARARGS,
                        "Return the redshift corresponding to the given velocity (v in km/s)."
                      },
                      {NULL, NULL, 0, NULL} /* Sentinel */
};





static struct PyModuleDef cosmology = {
                                       PyModuleDef_HEAD_INIT,
                                       "cosmology",
                                       "This library does the main cosmological calculations that are commonly necessary in extra-galactic astronomical studies. The main variable in this context is the redshift (z). The cosmological input parameters in the functions below are H0, o_lambda_0, o_matter_0, o_radiation_0 which respectively represent the current (at redshift 0) expansion rate (Hubble constant in units of km/sec/Mpc), cosmological constant (Λ), matter and radiation densities.",
                                       -1,
                                       CosmologyMethods};

PyMODINIT_FUNC
PyInit_cosmology(void)
{
  return PyModule_Create(&cosmology);
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
  if (PyImport_AppendInittab("spam", PyInit_cosmology) == -1)
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
  char mod_name[] = "cosmology";
  PyObject *pmodule = PyImport_ImportModule("cosmology");
  if (!pmodule)
    {
      PyErr_Print();
      fprintf(stderr, "Error: could not import module %s\n", mod_name);
    }

  PyMem_RawFree(program);
  return 0;
}
