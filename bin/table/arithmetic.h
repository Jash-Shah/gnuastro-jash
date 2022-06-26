/*********************************************************************
Table - View and manipulate a FITS table structures.
Table is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2020-2022 Free Software Foundation, Inc.

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
#ifndef ARITHMETIC_H
#define ARITHMETIC_H


#include <gnuastro/arithmetic.h>


/* Basic constants. */
#define ARITHMETIC_CALL "arith "
#define ARITHMETIC_CALL_LENGTH strlen(ARITHMETIC_CALL)


/* Operators used for arithmetic on columns. */
enum arithmetic_operators
{
  ARITHMETIC_TABLE_OP_SET = GAL_ARITHMETIC_OP_LAST_CODE,
  ARITHMETIC_TABLE_OP_WCSTOIMG,
  ARITHMETIC_TABLE_OP_IMGTOWCS,
  ARITHMETIC_TABLE_OP_DATETOSEC,
  ARITHMETIC_TABLE_OP_DISTANCEFLAT,
  ARITHMETIC_TABLE_OP_DATETOMILLISEC,
  ARITHMETIC_TABLE_OP_DISTANCEONSPHERE,
};







/* Functions */
void
arithmetic_init(struct tableparams *p, struct arithmetic_token **arith,
                gal_list_str_t **colstoread, size_t *totcalled,
                char *expression, gal_data_t *colinfo, size_t numcols);

void
arithmetic_token_free(struct arithmetic_token *list);

void
arithmetic_operate(struct tableparams *p);


#endif
