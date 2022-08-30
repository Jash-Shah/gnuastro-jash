/*********************************************************************
python -- Functions to assist Python wrappers using Gnuastro's library.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Jash Shah <jash28582@gmail.com>
Contributing author(s):
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Copyright (C) 2022 Free Software Foundation, Inc.

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

/* This macro needs to be defined before including any NumPy headers
   to avoid the compiler from raising a warning message. */
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

/* Gnuastro's Python headers. */
#include <gnuastro/python.h>





/*************************************************************
 **************           Type codes           ***************
 *************************************************************/
/* Convert Gnuastro type to NumPy datatype. Currently only converting types
   directly compatible between the two. */
int
gal_python_type_to_numpy(uint8_t type)
{
  switch (type)
    {
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





/* Convert Numpy datatype to Gnuastro type. Currently only converting types
   directly compatible between the two. */
uint8_t
gal_python_type_from_numpy(int type)
{
  switch (type)
    {
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
    case NPY_COMPLEX64:      return GAL_TYPE_COMPLEX64;
    case NPY_STRING:         return GAL_TYPE_STRING;
    default:
      error(EXIT_FAILURE, 0, "%s: type code %d is not convertible"
            "to Gnuastro.", __func__, type);
    }
  return GAL_TYPE_INVALID;
}
