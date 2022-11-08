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
#ifndef __GAL_ERROR_H__
#define __GAL_ERROR_H__

/* Include other headers if necessary here. Note that other header files
   must be included before the C++ preparations below */
#include <stdint.h>



/* C++ Preparations */
#undef __BEGIN_C_DECLS
#undef __END_C_DECLS
#ifdef __cplusplus
# define __BEGIN_C_DECLS extern "C" {
# define __END_C_DECLS }
#else
# define __BEGIN_C_DECLS                /* empty */
# define __END_C_DECLS                  /* empty */
#endif
/* End of C++ preparations */





/* Actual header contants (the above were for the Pre-processor). */
__BEGIN_C_DECLS  /* From C++ preparations */





/* Given the `lib_code`,`code` and the `is_warning` flag of an error,
   returns the value whose least significant 8 bits represents the
   `is_warning` flag, next 8 bits represent the `code` and next
   significant 8 bits represent the library code(`lib_code`).


                    ┌──────────────────┐
                    │                  │
                    │32 Bit Macro Value│
                    │                  │
                    └─────────┬────────┘
                              │
                              │
          ┌────────────────────┼───────────────────┐
          │                    │                   │
     Bits 16-25           Bits 8-15           Bits 0-7
          │                    │                   │
     ┌───────▼────────┐  ┌────────▼────────┐ ┌────────▼───────┐
     │   lib_code     │  │      code       │ │   is_warning   │
     │                │  │                 │ │                │
     │   0000 0000    │  │    0000 0000    │ │    0000 0000   │
     └────────────────┘  └─────────────────┘ └────────────────┘
*/
#define ERROR_BITSET(lib_code, code, is_warning) ((lib_code << 16) | (code << 8) | is_warning)





/************************************************************
 **************        Error Structure        ***************
 ************************************************************/
/* Data type for storing errors */
typedef struct gal_error_t
{
  uint8_t code;              /* Code of the problem wrt to each library.*/
  uint8_t lib_code;          /* Library which created the error.        */
  uint8_t is_warning;        /* Defines if the error is only a warning. */
  char *back_msg;            /* Detailed message of backend (library)   */
  char *front_msg;           /* Detailed message of front end (caller). */
  struct gal_error_t *next;  /* Next error message.                     */
} gal_error_t;





/************************************************************
 **************        Library Codes        ***************
 ************************************************************/
enum library_codes{
     GAL_ERROR_LIB_INVALID,
     GAL_ERROR_LIB_ARITHMETIC,
     GAL_ERROR_LIB_ARRAY,
     GAL_ERROR_LIB_BINARY,
     GAL_ERROR_LIB_BLANK,
     GAL_ERROR_LIB_BOX,
     GAL_ERROR_LIB_COLOR,
     GAL_ERROR_LIB_CONVOLVE,
     GAL_ERROR_LIB_COSMOLOGY,
     GAL_ERROR_LIB_DATA,
     GAL_ERROR_LIB_DIMENSION,
     GAL_ERROR_LIB_DS9,
     GAL_ERROR_LIB_EPS,
     GAL_ERROR_LIB_ERROR,
     GAL_ERROR_LIB_FITS,
     GAL_ERROR_LIB_GIT,
     GAL_ERROR_LIB_INTERPOLATE,
     GAL_ERROR_LIB_JPEG,
     GAL_ERROR_LIB_KDTREE,
     GAL_ERROR_LIB_LABEL,
     GAL_ERROR_LIB_LIST,
     GAL_ERROR_LIB_MATCH,
     GAL_ERROR_LIB_PDF,
     GAL_ERROR_LIB_PERMUTATION,
     GAL_ERROR_LIB_POINTER,
     GAL_ERROR_LIB_POLYGON,
     GAL_ERROR_LIB_QSORT,
     GAL_ERROR_LIB_SPECLINES,
     GAL_ERROR_LIB_STATISTICS,
     GAL_ERROR_LIB_TABLE,
     GAL_ERROR_LIB_THREADS,
     GAL_ERROR_LIB_TIFF,
     GAL_ERROR_LIB_TILE,
     GAL_ERROR_LIB_TXT,
     GAL_ERROR_LIB_TYPE,
     GAL_ERROR_LIB_UNITS,
     GAL_ERROR_LIB_WCS,
     GAL_ERROR_LIB_NUMBER
};





/****************************************************************
 ************************   Allocation   ************************
 ****************************************************************/
gal_error_t *
gal_error_allocate(uint8_t lib_code, uint8_t code, char *back_msg,
                   uint8_t is_warning);

void
gal_error_add_back_msg(gal_error_t **err, char *back_msg,
                       uint32_t macro_val);

void
gal_error_add_front_msg(gal_error_t **err, char *front_msg,
                        uint8_t replace);



/****************************************************************
 ************************   Priting   ************************
 ****************************************************************/
int
gal_error_print(gal_error_t *err);

void
gal_error_reverse(gal_error_t **err);



/****************************************************************
 ************************   Checking   ************************
 ****************************************************************/
uint8_t
gal_error_check(gal_error_t **err, uint32_t macro_val);

void
gal_error_parse_macro(uint32_t macro_val, uint8_t *lib_code, uint8_t *code,
                      uint8_t *is_warning);

uint8_t
gal_error_occurred(gal_error_t *err);




__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_ERROR_H__ */
