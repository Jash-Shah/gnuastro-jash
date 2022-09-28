/*********************************************************************
error - error handling throughout the Gnuastro library
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

#include <error.h>

#include <gnuastro/error.h>

#include <gnuastro-internal/checkset.h>






/*************************************************************
 **************           Type codes           ***************
 *************************************************************/
/* Prints all the error messages in the given structure.
   Prints both frontend and backend error messages if present else only
   prints the backend messages. */
void
gal_error_print(gal_error_t **err)
{
  char *newline = "\n";
  gal_error_t *tmperr;
  for(tmperr = *err; tmperr != NULL; tmperr = tmperr->next)
    {
      if(tmperr->front_msg)
        printf("%s: %s%s", tmperr->front_msg, tmperr->back_msg, newline);
      else printf("%s%s", tmperr->back_msg, newline);
    }
}





/* Allocate an error data structure based on the given parameters.
   While allocating no frontend error message should be given. The
   frontend error should only be added using gal_error_add_front_msg. */
gal_error_t *
gal_error_allocate(uint8_t code, char *back_msg)
{
  gal_error_t *outerr;

  /* Allocate the space for the structure. 
     We use calloc here so that the error code and is_warning flags
     are set to 0 indicating generic error type and a breaking error
     by default.  */
  outerr = calloc(outerr ,sizeof *outerr);
  if(outerr == NULL)
    error(EXIT_FAILURE, 0, "%s: %zu bytes for gal_error_t,",
          __func__, sizeof *outerr);
  
  /* Initialize the allocated error data */
  outerr->code = code;
  gal_checkset_allocate_copy(back_msg, &outerr->back_msg);

  printf("Out error allocated!\n");

  /* Return the final structure. */
  return outerr;
}





/* Adds a new error to the top of the given `err` structure with the given
   error code and backend msg. */
void
gal_error_add_back_msg(gal_error_t **err, uint8_t code, char *back_msg)
{
  /* If no back_msg has been provided then return NULL. */
  if (back_msg == NULL) return;
  
  /* Allocate a new error to be added at the top of the error stack. */
  gal_error_t *newerr;
  newerr = gal_error_allocate(code, back_msg);

  printf("Allocated new error!\n");
  
  /* Push the new error to the top of the stack. */
  newerr->next = *err;
  *err = newerr;
}





/* Adds a frontend error message to the top error in the given `err`
   structure. If the `replace` flag is 1 then the front_msg of the top error
   in the given `err` structure is replaced. */
void
gal_error_add_front_msg(gal_error_t **err, char *front_msg, uint8_t replace)
{
  /* If no front_msg has been provided then return NULL. */
  if (front_msg == NULL) return;
  
  if (*err->front_msg && !replace)
    error(EXIT_FAILURE, 0, "%s: A frontend error message already exists
          for the given error %u. If you wish to replace it then pass '1'
          to the replace flag while calling the function.", __func__,
          *err->code);
  else
    gal_checkset_allocate_copy(front_msg,err->front_msg);

  printf("Front error message added!\n");
}
