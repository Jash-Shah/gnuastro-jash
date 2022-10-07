/*********************************************************************
Warp - Warp images using projective mapping.
Warp is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
     Pedram Ashofteh Ardakani <pedramardakani@pm.me>
Copyright (C) 2016-2022 Free Software Foundation, Inc.

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
#ifndef MAIN_H
#define MAIN_H

/* Include necessary headers */
#include <gnuastro/data.h>
#include <gnuastro/warp.h>

#include <gnuastro-internal/options.h>

/* Progarm names.  */
#define PROGRAM_NAME   "Warp"        /* Program full name.       */
#define PROGRAM_EXEC   "astwarp"     /* Program executable name. */
#define PROGRAM_STRING PROGRAM_NAME" (" PACKAGE_NAME ") " PACKAGE_VERSION





/* Main program structure. */
struct warpparams
{
  /* From command-line */
  struct gal_options_common_params cp; /* Common parameters.             */
  gal_warp_wcsalign_t  wa;  /* Nonlinear-specific parameters.            */
  char         *inputname;  /* Name of input file.                       */
  size_t        hstartwcs;  /* Header keyword No. to start reading WCS.  */
  size_t          hendwcs;  /* Header keyword No. to end reading WCS.    */
  uint8_t         keepwcs;  /* Wrap the warped/transfomed pixels.        */
  uint8_t  centeroncorner;  /* Shift center by 0.5 before and after.     */
  double      coveredfrac;  /* Acceptable fraction of output covered.    */
  gal_data_t       *width;  /* Width of final image.                     */
  uint8_t      widthinpix;  /* If the given width is in units of pixels. */
  char           *gridhdu;  /* Extension to use for output's WCS.        */
  char          *gridfile;  /* File to use for output's WCS.             */

  /* Internal parameters: */
  gal_data_t       *input;  /* Input data structure.                     */
  gal_data_t      *output;  /* output data structure.                    */
  gal_data_t      *matrix;  /* Warp/Transformation matrix.               */
  gal_data_t   *modularll;  /* List of modular warpings.                 */
  double     *inwcsmatrix;  /* Input WCS matrix.                         */
  double         *inverse;  /* Inverse of the input matrix.              */
  time_t          rawtime;  /* Starting time of the program.             */
  size_t       ordinds[4];  /* Indexs of anticlockwise vertices.         */
  size_t       extinds[4];  /* Indexs of the minimum and maximum values. */
  double    outfpixval[2];  /* Pixel value of first output pixel.        */
  double         opixarea;  /* Area of output pix in units of input pix. */
  uint8_t        wcsalign;  /* If warp must work in WCS-align mode.      */
  uint8_t  distortiontype;  /* Store distortion type in nonlinear mode.  */
};

#endif
