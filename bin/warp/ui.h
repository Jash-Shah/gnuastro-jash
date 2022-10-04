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
#ifndef UI_H
#define UI_H

/* For common options groups. */
#include <gnuastro-internal/options.h>





/* Option groups particular to this program. */
enum program_args_groups
{
  UI_GROUP_ALIGN = GAL_OPTIONS_GROUP_AFTER_COMMON,
  UI_GROUP_WARPS
};





/* Available letters for short options:

   a b d g i j l n u v z
   A B E J L O Q R W X Y
*/
enum option_keys_enum
{
  /* With short-option version. */
  UI_KEY_KEEPWCS         = 'k',
  UI_KEY_COVEREDFRAC     = 'C',
  UI_KEY_ROTATE          = 'r',
  UI_KEY_SCALE           = 's',
  UI_KEY_FLIP            = 'f',
  UI_KEY_SHEAR           = 'e',
  UI_KEY_TRANSLATE       = 't',
  UI_KEY_PROJECT         = 'p',
  UI_KEY_MATRIX          = 'm',
  UI_KEY_CDELT           = 'x',
  UI_KEY_INTERPSAMPLING  = 'y',
  UI_KEY_CENTER          = 'c',
  UI_KEY_WIDTHINPIX      = 'w',
  UI_KEY_GRIDFILE        = 'G',
  UI_KEY_GRIDHDU         = 'H',

  /* Only with long version (start with a value 1000, the rest will be set
     automatically). */
  UI_KEY_CENTERONCORNER = 1000,
  UI_KEY_EDGESAMPLING,
  UI_KEY_HSTARTWCS,
  UI_KEY_HENDWCS,
  UI_KEY_CTYPE,
};





/* Functions */
void
ui_read_check_inputs_setup(int argc, char *argv[], struct warpparams *p);

void
ui_free_report(struct warpparams *p, struct timeval *t1);

#endif
