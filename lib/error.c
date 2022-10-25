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
   prints the backend messages. Returns an int denoting if no. of errors
   thus giving the user the option to EXIT_FAILURE themselves. */
int
gal_error_print(gal_error_t *err)
{
  /* If error structure is empty return -1. */
  if (!err) return -1;

  gal_error_t *tmperr = NULL;
  /* Number of errors in the structure. */
  uint8_t is_error = 0;
  for(tmperr = err; tmperr!=NULL; tmperr = tmperr->next)
    {
      if(tmperr->front_msg)
        {
          fprintf(stderr, "%s\n",tmperr->front_msg);
        }
      fprintf(stderr, "%s: %s\n", tmperr->code, tmperr->back_msg);

      /* If atleast one error is found which is NOT a warning. */
      if(!tmperr->is_warning) is_error++;  
    }
  return is_error;
}





/* In the given error structure, finds the first error which is not a 
   warning, prints it and exits the program with an EXIT_FAILURE. */
void
gal_error_exit(gal_error_t **err)
{
  gal_error_t *tmperr = NULL;

  for(tmperr = *err; tmperr != NULL; tmperr = tmperr->next)
    {
      if(!tmperr->is_warning)
        {
          if(tmperr->front_msg)
            error(EXIT_FAILURE, 0,
                  "%s\n%s: %s\n", tmperr->front_msg,
                  tmperr->code, tmperr->back_msg);
          else
            error(EXIT_FAILURE, 0,
                  "\n%s: %s\n", tmperr->code, tmperr->back_msg);
        }
    }
}





/* Allocate an error data structure based on the given parameters.
   While allocating no frontend error message should be given. The
   frontend error should only be added using gal_error_add_front_msg. */
gal_error_t *
gal_error_allocate(char *code, char *back_msg, uint8_t is_warning)
{
  gal_error_t *outerr;

  /* Allocate the space for the structure. 
     We use calloc here so that the error code and is_warning flags
     are set to 0 indicating generic error type and a breaking error
     by default. */
  outerr = calloc(1, sizeof *outerr);
  if(outerr == NULL)
    error(EXIT_FAILURE, 0, "%s: %zu bytes for gal_error_t,",
          __func__, sizeof *outerr);
  
  /* Initialize the allocated error data */
  outerr->code = code;
  outerr->is_warning = is_warning;
  gal_checkset_allocate_copy(back_msg, &outerr->back_msg);

  /* Return the final structure. */
  return outerr;
}





/* Adds a new error to the top of the given `err` structure with the given
   error code, backend msg and is_warning flag. */
void
gal_error_add_back_msg(gal_error_t **err, char *code,
                       char *back_msg, uint8_t is_warning)
{
  /* If no back_msg has been provided then return NULL. */
  if (back_msg == NULL) return;
  
  /* Allocate a new error to be added at the top of the error stack. */
  gal_error_t *newerr;
  newerr = gal_error_allocate(code, back_msg, is_warning);
  
  /* Push the new error to the top of the stack. */
  newerr->next = *err;
  *err = newerr;
}





/* Adds a frontend error message to the top error in the given `err`
   structure. If the `replace` flag is 1 then the front_msg of the top
   error in the given `err` structure is replaced. */
void
gal_error_add_front_msg(gal_error_t **err, char *front_msg,
                        uint8_t replace)
{
  /* If no front_msg has been provided then return NULL. */
  if (front_msg == NULL) return;
  
  if ((*err)->front_msg && !replace)
    error(EXIT_FAILURE, 0, "%s: A frontend error message already exists "
          "for the given error %s. If you wish to replace it then pass "
          "'1' to the replace flag while calling the function.", __func__,
          (*err)->code);
  else
    gal_checkset_allocate_copy(front_msg,&(*err)->front_msg);
}





/* Reverse the errors in the list. This is needed since we are treating
   the gal_error_t structure like a stack. */
void
gal_error_reverse(gal_error_t **err)
{
  /* Structure which will store the correct/reversed order. */
  gal_error_t *correctorder = NULL;

  /* Only do the reversal if there is more than one element. */
  if( *err && (*err)->next )
    {
      while(*err!=NULL)
        {
          /* Pop top element and add to new list */
          gal_error_add_back_msg(&correctorder, (*err)->code,
                                 (*err)->back_msg, (*err)->is_warning);
          gal_error_add_front_msg(&correctorder, (*err)->front_msg, 0);
          (*err) = (*err)->next;
        }
      *err = correctorder;
    }
}