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

// To avoid problem with non-clang compilers not having this macro.
#if defined(__has_include)
    #if __has_include(<numpy/arrayobject.h>)
        # include <numpy/arrayobject.h>
    #endif
#endif

#include <gnuastro/python.h>










/*************************************************************
 **************           Type codes           ***************
 *************************************************************/
int
gal_npy_datatype_to_type(uint8_t type)
{
  switch (type)
  {
  // Currently only converting types directly compatible
  // between the two.
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
  case GAL_TYPE_COMPLEX64: return NPY_COMPLEX64;
  case GAL_TYPE_STRING:    return NPY_STRING;
  default:                 return GAL_TYPE_INVALID;
  }

  return GAL_TYPE_INVALID;
}





int
gal_npy_type_to_datatype(uint8_t type)
{
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
  default:                 return GAL_TYPE_INVALID;
  }
  return GAL_TYPE_INVALID;
}