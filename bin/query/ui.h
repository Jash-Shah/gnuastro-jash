/*********************************************************************
Query - Retreive data from a remote data server.
Query is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2020-2021, Free Software Foundation, Inc.

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
  UI_GROUP_GENQUERY = GAL_OPTIONS_GROUP_AFTER_COMMON,
};





/* Available letters for short options:

   a e f j m n p t u x y z
   A B E G J R W X Y
*/
enum option_keys_enum
{
  /* With short-option version. */
  UI_KEY_KEEPRAWDOWNLOAD = 'k',
  UI_KEY_INFORMATION     = 'i',
  UI_KEY_LIMITINFO       = 'L',
  UI_KEY_DATABASE        = 'd',
  UI_KEY_QUERY           = 'Q',
  UI_KEY_DATASET         = 's',
  UI_KEY_CENTER          = 'C',
  UI_KEY_OVERLAPWITH     = 'v',
  UI_KEY_RADIUS          = 'r',
  UI_KEY_RANGE           = 'g',
  UI_KEY_NOBLANK         = 'b',
  UI_KEY_COLUMN          = 'c',
  UI_KEY_WIDTH           = 'w',
  UI_KEY_HEAD            = 'H',

  /* Only with long version (start with a value 1000, the rest will be set
     automatically). */
  UI_KEY_CCOL            = 1000,
  UI_KEY_SORT,
};



char *
ui_strlist_to_str(gal_list_str_t *input);

void
ui_read_check_inputs_setup(int argc, char *argv[], struct queryparams *p);

void
ui_free_report(struct queryparams *p, struct timeval *t1);

#endif
