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





/* Prints all the error messages in the given structure.
   Print format:
   "Frontend msg: code: Backend msg: [BREAKING]"
   Frontend msg and [BREAKING] is printed only if they exist.
   Returns an int denoting if no. of breaking errors is more than 1, thus giving
   the user the option to EXIT_FAILURE themselves. */
int
gal_error_print(gal_error_t *err)
{
  /* If error structure is empty return -1. */
  if (!err) return -1;

  char *errsuffix="";
  gal_error_t *tmperr = NULL;

  /* Number of errors in the structure. */
  uint8_t count_err = 0;
  for(tmperr = err; tmperr!=NULL; tmperr = tmperr->next)
    {
      /* If an error is found which is NOT a warning. */
      if(!tmperr->is_warning) 
      {
        count_err++;
        errsuffix=" [BREAKING]";
      }
       
      if(tmperr->front_msg)
        error(EXIT_SUCCESS, 0,
              "%s: %d: %d: %s%s",tmperr->front_msg, tmperr->lib_code,
              tmperr->code, tmperr->back_msg, errsuffix);
      else
        error(EXIT_SUCCESS, 0,
              "%d: %d: %s%s", tmperr->lib_code, tmperr->code,
              tmperr->back_msg, errsuffix);
    }
  return count_err;
}





/* Returns 0 or 1 depending on whether a breaking error occurred on the
   last function call i.e if the error on top of the stack is a breaking
   error. */
uint8_t
gal_error_occurred(gal_error_t *err)
{
  /* Return 0 when empty or a warning, return 1 otherwise. */
  return (!err || err->is_warning) ? 0 : 1;
}





/* Allocate an error data structure based on the given parameters.
   While allocating no frontend error message should be given. The
   frontend error should only be added using gal_error_add_front_msg. */
gal_error_t *
gal_error_allocate(uint8_t lib_code, uint8_t code, char *back_msg,
                   uint8_t is_warning)
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
  outerr->lib_code = lib_code;
  outerr->code = code;
  outerr->is_warning = is_warning;
  gal_checkset_allocate_copy(back_msg, &outerr->back_msg);

  /* Return the final structure. */
  return outerr;
}





/* Adds a new error to the top of the given `err` structure given the
   error `macro_val` as the third argument. From it extract the `code`
   and the `is_warning` flags and save them in the structure. */
void
gal_error_add_back_msg(gal_error_t **err, char *back_msg,
                       uint32_t macro_val)
{
  /* If no back_msg has been provided then return NULL. */
  if (back_msg == NULL) return;

  uint8_t code = 0;
  uint8_t lib_code = 0;
  uint8_t is_warning = 0;

  gal_error_parse_macro(macro_val, &lib_code, &code, &is_warning);

  /* Allocate a new error to be added at the top of the error stack. */
  gal_error_t *newerr;
  newerr = gal_error_allocate(lib_code, code, back_msg, is_warning);
  
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
  /* Don't do anything if error structure is empty or no message provided. */
  if (!*err || front_msg==NULL) 
       return;
     
  /* Only allocate if an error already exists. */
  if ((*err)->front_msg && !replace)
    error(EXIT_FAILURE, 0, "%s: A frontend error message already exists "
          "for the given error %d. If you wish to replace it then pass "
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
  /* If error structure is empty return -1. */
  if (!*err) return;

  /* Structure which will store the correct/reversed order. */
  gal_error_t *correctorder = NULL;
  /* `macro_val` has to be constructed from `code` & `is_warning`. */
  uint32_t macro_val = 0;

  /* Only do the reversal if there is more than one element. */
  if( *err && (*err)->next )
    {
      while(*err!=NULL)
        {
          /* The least significant 8 bits represents the `is_warning` flag,
             next 8 bits represent the `code` and next significant 8 bits
             represent the library code(`lib_code`). */
          macro_val = (*err)->lib_code;
          macro_val = (macro_val << 8) | (*err)->code;
          macro_val = (macro_val << 8) | (*err)->is_warning;

          /* Pop top element and add to new list */
          gal_error_add_back_msg(&correctorder,
                                 (*err)->back_msg, macro_val);
          gal_error_add_front_msg(&correctorder, (*err)->front_msg, 0);
          (*err) = (*err)->next;
        }
      *err = correctorder;
    }
}





/* Takes in a 32-bit integer (value of an error macro) and extracts the
   error `lib_code`, `code` and `is_warning` flag values. */
void
gal_error_parse_macro(uint32_t macro_val, uint8_t *lib_code, uint8_t *code,
                      uint8_t *is_warning)
{
  /* The value of an error macro is a 32-bit integer. The first(starting
     from the LSB) 8 bits denote the `is_warning` flag status. Since the
     status is either 0 or 1, if the macro value is odd then `is_warning`
     flag is true.
     
      00000000 00000000 00000000 00000000
              |      | |      | |      |
              -------  -------  -------
                |         |        |
            {lib_code} {code} {is_warning}
*/
  if (macro_val % 2 != 0) *is_warning = 1;
  else *is_warning = 0;
  /* The next 8 bits (i.e. from bit 9-16) represent the `code`. */
  *code = (macro_val >> 8) & 255;
  /* The next 8 bits (i.e. from bit 17-24) represent the `code`. */
  *lib_code = (macro_val >> 16) & 255;
}





/* Given an `err` structure and a `macro_val`, return 1 or 0 based on
   whether an error of the given type exists within the structure. */
uint8_t
gal_error_check(gal_error_t **err, uint32_t macro_val)
{
  gal_error_t *tmperr = NULL;
  uint8_t lib_code = 0;
  uint8_t code = 0;
  uint8_t is_warning = -1;

  gal_error_parse_macro(macro_val, &lib_code, &code, &is_warning);

  for(tmperr = *err; tmperr != NULL; tmperr = tmperr->next)
  {
    if(tmperr->code == code) return 1;
  }

  return 0;
}
