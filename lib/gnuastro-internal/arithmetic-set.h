/*********************************************************************
Arithmetic operations on data structures.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2021, Free Software Foundation, Inc.

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
#ifndef __ARITHMETIC_SET_H__
#define __ARITHMETIC_SET_H__

#define GAL_ARITHMETIC_SET_PREFIX         "set-"
#define GAL_ARITHMETIC_SET_PREFIX_LENGTH  strlen(GAL_ARITHMETIC_SET_PREFIX)


struct gal_arithmetic_set_params
{
  void               *tokens;   /* Full list of tokens.            */
  size_t        tokencounter;   /* Counter of current token.       */
  gal_data_t          *named;   /* List of named datasets.         */
  void               *params;   /* Internal parameters in caller.  */
  gal_data_t  * (*pop)(void *in_prm);  /* Function to use.         */
  int (*used_later)(void *in_prm, char *name); /* Function to use. */
};

void
gal_arithmetic_set_name(struct gal_arithmetic_set_params *p,
                        char *token);

int
gal_arithmetic_set_is_name(gal_data_t *named, char *token);

gal_data_t *
gal_arithmetic_set_copy_named(struct gal_arithmetic_set_params *p,
                              char *name);

#endif
