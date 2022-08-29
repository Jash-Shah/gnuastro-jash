/*********************************************************************
python -- Functions to assist the python wrappers.
This is part of GNU Astronomy Utilities (Gnuastro) package.
Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
     Sachin Kumar Singh <sachinkumarsingh092@gmail.com>
Copyright (C) 2017-2022 Free Software Foundation, Inc.
Gnuastro is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.
Gnuastro is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.
You should have received a copy of the GNU General Public License
along with Gnuastro. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
#include <config.h>

#include <errno.h>
#include <error.h>

#ifdef HAVE_PYTHON
  // This macro needs to be defined before including any NumPy headers
  // to avoid the compiler from raising a warning message.
  #define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
  #include <numpy/arrayobject.h>
#endif

#include <gnuastro/python.h>




















/*************************************************************
 **************           Type codes           ***************
 *************************************************************/
#ifdef HAVE_PYTHON
/* Convert Gnuastro type to NumPy datatype. */
int
gal_py_galtype_to_npytype(uint8_t type)
{
  switch (type)
  {
  /* Types directly convertible between Gnuastro and NumPy. */
  case GAL_TYPE_INT8:      return NPY_INT8;
  case GAL_TYPE_INT16:     return NPY_INT16;
  case GAL_TYPE_INT32:     return NPY_INT32;
  case GAL_TYPE_INT64:     return NPY_LONG;
  case GAL_TYPE_UINT8:     return NPY_UINT8;
  case GAL_TYPE_UINT16:    return NPY_UINT16;
  case GAL_TYPE_UINT32:    return NPY_UINT32;
  case GAL_TYPE_UINT64:    return NPY_UINT64;
  case GAL_TYPE_FLOAT32:   return NPY_FLOAT32;
  case GAL_TYPE_FLOAT64:   return NPY_FLOAT64;
  case GAL_TYPE_STRING:    return NPY_STRING;
  default:
    error(EXIT_FAILURE, 0, "%s: type code %d is not convertible"
            "to NumPy.", __func__, type);
  }

  return GAL_TYPE_INVALID;
}





/* Convert Numpy datatype to Gnuastro type. */
int
gal_py_npytype_to_galtype(uint8_t type)
{
  /* Types directly convertible between NumPy and Gnuastro. */
  switch (type)
  {
  // Currently only converting types directly compatible
  // between the two.
  case NPY_INT8:           return GAL_TYPE_INT8;
  case NPY_INT16:          return GAL_TYPE_INT16;
  case NPY_INT32:          return GAL_TYPE_INT32;
  case NPY_LONG:           return GAL_TYPE_INT64;
  case NPY_UINT8:          return GAL_TYPE_UINT8;
  case NPY_UINT16:         return GAL_TYPE_UINT16;
  case NPY_UINT32:         return GAL_TYPE_UINT32;
  case NPY_UINT64:         return GAL_TYPE_UINT64;
  case NPY_FLOAT32:        return GAL_TYPE_FLOAT32;
  case NPY_FLOAT64:        return GAL_TYPE_FLOAT64;
  case GAL_TYPE_COMPLEX64: return NPY_COMPLEX64;
  case GAL_TYPE_STRING:    return NPY_STRING;
  default:
    error(EXIT_FAILURE, 0, "%s: type code %d is not convertible"
            "to Gnuastro.", __func__, type);
  }
  return GAL_TYPE_INVALID;
}
#endif
