/*********************************************************************
eps -- functions to write EPS files.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2015-2022 Free Software Foundation, Inc.

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
#ifndef __GAL_EPS_H__
#define __GAL_EPS_H__


/* Include other headers if necessary here. Note that other header files
   must be included before the C++ preparations below */
#include <gnuastro/data.h>
#include <gnuastro/color.h>



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





/* Definitions */
enum gal_eps_shapes
{
  GAL_EPS_MARK_SHAPE_INVALID,   /* Invalid (=0 by C standard).  */
  GAL_EPS_MARK_SHAPE_CIRCLE,
  GAL_EPS_MARK_SHAPE_PLUS,
  GAL_EPS_MARK_SHAPE_CROSS,     /* The lines are not based on length */
  GAL_EPS_MARK_SHAPE_ELLIPSE,
  GAL_EPS_MARK_SHAPE_POINT,
  GAL_EPS_MARK_SHAPE_SQUARE,
  GAL_EPS_MARK_SHAPE_RECTANGLE,
  GAL_EPS_MARK_SHAPE_LINE,

  /* This will be the total number of shapes (good for scripts). */
  GAL_EPS_MARK_SHAPE_NUMBER
};





/* Expected names of columns */
#define  GAL_EPS_MARK_COLNAME_TEXT       "TEXT"
#define  GAL_EPS_MARK_COLNAME_FONT       "FONT"
#define  GAL_EPS_MARK_COLNAME_XPIX       "X-PIX"
#define  GAL_EPS_MARK_COLNAME_YPIX       "Y-PIX"
#define  GAL_EPS_MARK_COLNAME_SHAPE      "SHAPE"
#define  GAL_EPS_MARK_COLNAME_COLOR      "COLOR"
#define  GAL_EPS_MARK_COLNAME_SIZE1      "SIZE1"
#define  GAL_EPS_MARK_COLNAME_SIZE2      "SIZE2"
#define  GAL_EPS_MARK_COLNAME_ROTATE     "ROTATE"
#define  GAL_EPS_MARK_COLNAME_FONTSIZE   "FONTSIZE"
#define  GAL_EPS_MARK_COLNAME_LINEWIDTH  "LINEWIDTH"




/* Default mark properties. */
#define GAL_EPS_MARK_DEFAULT_SHAPE         GAL_EPS_MARK_SHAPE_CIRCLE
#define GAL_EPS_MARK_DEFAULT_COLOR         GAL_COLOR_RED
#define GAL_EPS_MARK_DEFAULT_SIZE1         5
#define GAL_EPS_MARK_DEFAULT_SIZE2         3
#define GAL_EPS_MARK_DEFAULT_SIZE2_ELLIPSE 0.5
#define GAL_EPS_MARK_DEFAULT_ROTATE        0
#define GAL_EPS_MARK_DEFAULT_LINEWIDTH     1
#define GAL_EPS_MARK_DEFAULT_FONT         "Arial"
#define GAL_EPS_MARK_DEFAULT_FONTSIZE      4




/* Functions */
int
gal_eps_name_is_eps(char *name);

int
gal_eps_suffix_is_eps(char *name);

void
gal_eps_to_pt(float widthincm, size_t *dsize, size_t *w_h_in_pt);

uint8_t
gal_eps_shape_name_to_id(char *name);

char *
gal_eps_shape_id_to_name(uint8_t id);

void
gal_eps_write(gal_data_t *in, char *filename, float widthincm,
              uint32_t borderwidth, int hex, int dontoptimize,
              int forpdf, gal_data_t *marks);



__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_TIFF_H__ */
