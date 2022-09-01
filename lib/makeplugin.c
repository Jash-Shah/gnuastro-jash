/*********************************************************************
Extensions to GNU Make for working with FITS files.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
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

#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

#include <gnumake.h>

#include <gnuastro/txt.h>

#include <gnuastro-internal/options.h>
#include <gnuastro-internal/checkset.h>





/* Necessary for GNU Make. */
int plugin_is_GPL_compatible=1;





/* Names of the separate functions */
#define MAKEPLUGIN_FUNC_PREFIX "ast"
static char *text_contains_name=MAKEPLUGIN_FUNC_PREFIX"-text-contains";
static char *fits_with_keyvalue_name=MAKEPLUGIN_FUNC_PREFIX"-fits-with-keyvalue";
static char *fits_unique_keyvalues_name=MAKEPLUGIN_FUNC_PREFIX"-fits-unique-keyvalues";










/**********************************************************************/
/***************             Text utilities             ***************/
/**********************************************************************/

/* Return any of the input strings that contain the given string. It takes
   two arguments:
      0. String to check.
      1. List of text.*/
static char *
makeplugin_text_contains(const char *caller, unsigned int argc,
                         char **argv)
{
  char *out=NULL;
  gal_list_str_t *tmp, *outlist=NULL;
  char *match=argv[0]; /* No trimming the white space before/after, as in */
  gal_list_str_t *strings=gal_list_str_extract(argv[1]); /* Make itself.  */

  printf("%s: HERE\n", __func__);
  gal_list_str_print(strings);

  /* Parse the input strings and find the ones that match. */
  for(tmp=strings; tmp!=NULL; tmp=tmp->next)
    if( gal_txt_contains_string(tmp->v, match) )
      gal_list_str_add(&outlist, tmp->v, 0);

  /* Write the list into one string. */
  out=gal_list_str_cat(outlist);

  /* Clean up and return. */
  gal_list_str_free(strings, 1);
  gal_list_str_free(outlist, 0); /* We didn't allocate these. */
  return out;
}




















/**********************************************************************/
/***************             FITS utilities             ***************/
/**********************************************************************/

/* Select the input files that have the requested value(s) in the requested
   keywords. */
static int
makeplugin_fits_check_input(char **argv, size_t numargs, char *name)
{
  char *c;
  size_t i;

  /* If the HDU is empty, print a warning and don't continue. */
  for(i=0;i<numargs;++i)
    {
      /* We need to skip white-space characters. */
      c=argv[i]; do if(isspace(*c)) ++c; else break; while(1);
      if(*c=='\0')
        {
          if(i>0) /* Message only necessary for first argument. */
            error(EXIT_SUCCESS, 0, "%s: argument %zu is empty",
                  name, i+1);
          return 0;
        }
    }

  /* If control reaches here, everything is good. */
  return 1;
}





/* Select files, were a certain keyword has a certain value. It takes four
   arguments:
       0. Keyword name.
       1. Keyword value(s).
       2. HDU (fixed in all files).
       3. List of files. */
static char *
makeplugin_fits_with_keyvalue(const char *caller, unsigned int argc,
                              char **argv)
{
  gal_list_str_t *outlist=NULL;
  char *name=gal_txt_trim_space(argv[0]);
  gal_list_str_t *files=NULL, *values=NULL;
  char *out, *hdu=gal_txt_trim_space(argv[2]);

  /* If any of the inputs are empty, then don't bother continuing. */
  if( makeplugin_fits_check_input(argv, 4, fits_with_keyvalue_name)==0 )
    return NULL;

  /* Extract the components in the arguments with possibly multiple
     values and find the output files.*/
  files=gal_list_str_extract(argv[3]);
  values=gal_list_str_extract(argv[1]);
  outlist=gal_fits_with_keyvalue(files, hdu, name, values);

  /* Write the output string */
  out=gal_list_str_cat(outlist);

  /* Clean up and return. */
  gal_list_str_free(files, 1);
  gal_list_str_free(values, 1);
  gal_list_str_free(outlist, 1);
  return out;
}




/* Return the unique values given to a certain keyword in many FITS
   files. It takes three arguments.
       0. Keyword name.
       1. HDU (fixed in all files).
       2. List of files. */
static char *
makeplugin_fits_unique_keyvalues(const char *caller, unsigned int argc,
                                 char **argv)
{
  gal_list_str_t *files=NULL;
  gal_list_str_t *outlist=NULL;
  char *name=gal_txt_trim_space(argv[0]);
  char *out, *hdu=gal_txt_trim_space(argv[1]);

  /* If any of the inputs are empty, then don't bother continuing. */
  if( makeplugin_fits_check_input(argv, 3, fits_unique_keyvalues_name)==0 )
    return NULL;

  /* Extract the components in the arguments with possibly multiple
     values and find the output files.*/
  files=gal_list_str_extract(argv[2]);
  outlist=gal_fits_unique_keyvalues(files, hdu, name);

  /* Write the output value. */
  out=gal_list_str_cat(outlist);

  /* Clean up and return. */
  gal_list_str_free(files, 1);
  gal_list_str_free(outlist, 1);
  return out;
}





/* Top-level function (that should have this name). */
int
libgnuastro_make_gmk_setup()
{
  /* Return any of the input strings that contain the given string. */
  gmk_add_function(text_contains_name,
                   makeplugin_text_contains,
                   2, 2, GMK_FUNC_DEFAULT);

  /* Select files, were a certain keyword has a certain value. It takes
     four arguments. */
  gmk_add_function(fits_with_keyvalue_name, makeplugin_fits_with_keyvalue,
                   4, 4, GMK_FUNC_DEFAULT);

  /* Return the unique values given to a certain keyword in many FITS
     files.*/
  gmk_add_function(fits_unique_keyvalues_name,
                   makeplugin_fits_unique_keyvalues,
                   3, 3, GMK_FUNC_DEFAULT);

  /* Everything is good, return 1 (success). */
  return 1;
}
