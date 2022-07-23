/*********************************************************************
Functions for colors
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
#ifndef __GAL_COLOR_H__
#define __GAL_COLOR_H__

/* Include other headers if necessary here. Note that other header files
   must be included before the C++ preparations below */
#include <gnuastro/data.h>


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



/* From: https://en.wikipedia.org/wiki/Web_colors#Extended_colors */
enum gal_color_all
{
  GAL_COLOR_INVALID,            /* ==0 (in C standard). */

  /* Pink colors. */
  GAL_COLOR_MEDIUMVIOLETRED,
  GAL_COLOR_DEEPPINK,
  GAL_COLOR_PALEVIOLETRED,
  GAL_COLOR_HOTPINK,
  GAL_COLOR_LIGHTPINK,
  GAL_COLOR_PINK,

  /* Red colors. */
  GAL_COLOR_DARKRED,
  GAL_COLOR_RED,
  GAL_COLOR_FIREBRICK,
  GAL_COLOR_CRIMSON,
  GAL_COLOR_INDIANRED,
  GAL_COLOR_LIGHTCORAL,
  GAL_COLOR_SALMON,
  GAL_COLOR_DARKSALMON,
  GAL_COLOR_LIGHTSALMON,

  /* Orange colors */
  GAL_COLOR_ORANGERED,
  GAL_COLOR_TOMATO,
  GAL_COLOR_DARKORANGE,
  GAL_COLOR_CORAL,
  GAL_COLOR_ORANGE,

  /* Yellow colors. */
  GAL_COLOR_DARKKHAKI,
  GAL_COLOR_GOLD,
  GAL_COLOR_KHAKI,
  GAL_COLOR_PEACHPUFF,
  GAL_COLOR_YELLOW,
  GAL_COLOR_PALEGOLDENROD,
  GAL_COLOR_MOCCASIN,
  GAL_COLOR_PAPAYAWHIP,
  GAL_COLOR_LIGHTGOLDENRODYELLOW,
  GAL_COLOR_LEMONCHIFFON,
  GAL_COLOR_LIGHTYELLOW,

  /* Brown colors. */
  GAL_COLOR_MAROON,
  GAL_COLOR_BROWN,
  GAL_COLOR_SADDLEBROWN,
  GAL_COLOR_SIENNA,
  GAL_COLOR_CHOCOLATE,
  GAL_COLOR_DARKGOLDENROD,
  GAL_COLOR_PERU,
  GAL_COLOR_ROSYBROWN,
  GAL_COLOR_GOLDENROD,
  GAL_COLOR_SANDYBROWN,
  GAL_COLOR_TAN,
  GAL_COLOR_BURLYWOOD,
  GAL_COLOR_WHEAT,
  GAL_COLOR_NAVAJOWHITE,
  GAL_COLOR_BISQUE,
  GAL_COLOR_BLANCHEDALMOND,
  GAL_COLOR_CORNSILK,

  /* Green colors. */
  GAL_COLOR_DARKGREEN,
  GAL_COLOR_GREEN,
  GAL_COLOR_DARKOLIVEGREEN,
  GAL_COLOR_FORESTGREEN,
  GAL_COLOR_SEAGREEN,
  GAL_COLOR_OLIVE,
  GAL_COLOR_OLIVEDRAB,
  GAL_COLOR_MEDIUMSEAGREEN,
  GAL_COLOR_LIMEGREEN,
  GAL_COLOR_LIME,
  GAL_COLOR_SPRINGGREEN,
  GAL_COLOR_MEDIUMSPRINGGREEN,
  GAL_COLOR_DARKSEAGREEN,
  GAL_COLOR_MEDIUMAQUAMARINE,
  GAL_COLOR_YELLOWGREEN,
  GAL_COLOR_LAWNGREEN,
  GAL_COLOR_CHARTREUSE,
  GAL_COLOR_LIGHTGREEN,
  GAL_COLOR_GREENYELLOW,
  GAL_COLOR_PALEGREEN,

  /* Cyan colors. */
  GAL_COLOR_TEAL,
  GAL_COLOR_DARKCYAN,
  GAL_COLOR_LIGHTSEAGREEN,
  GAL_COLOR_CADETBLUE,
  GAL_COLOR_DARKTURQUOISE,
  GAL_COLOR_MEDIUMTURQUOISE,
  GAL_COLOR_TURQUOISE,
  GAL_COLOR_AQUA,
  GAL_COLOR_CYAN,
  GAL_COLOR_AQUAMARINE,
  GAL_COLOR_PALETURQUOISE,
  GAL_COLOR_LIGHTCYAN,

  /* Blue colors. */
  GAL_COLOR_MIDNIGHTBLUE,
  GAL_COLOR_NAVY,
  GAL_COLOR_DARKBLUE,
  GAL_COLOR_MEDIUMBLUE,
  GAL_COLOR_BLUE,
  GAL_COLOR_ROYALBLUE,
  GAL_COLOR_STEELBLUE,
  GAL_COLOR_DODGERBLUE,
  GAL_COLOR_DEEPSKYBLUE,
  GAL_COLOR_CORNFLOWERBLUE,
  GAL_COLOR_SKYBLUE,
  GAL_COLOR_LIGHTSKYBLUE,
  GAL_COLOR_LIGHTSTEELBLUE,
  GAL_COLOR_LIGHTBLUE,
  GAL_COLOR_POWDERBLUE,

  /* Purple colors. */
  GAL_COLOR_INDIGO,
  GAL_COLOR_PURPLE,
  GAL_COLOR_DARKMAGENTA,
  GAL_COLOR_DARKVIOLET,
  GAL_COLOR_DARKSLATEBLUE,
  GAL_COLOR_BLUEVIOLET,
  GAL_COLOR_DARKORCHID,
  GAL_COLOR_FUCHSIA,
  GAL_COLOR_MAGENTA,
  GAL_COLOR_SLATEBLUE,
  GAL_COLOR_MEDIUMSLATEBLUE,
  GAL_COLOR_MEDIUMORCHID,
  GAL_COLOR_MEDIUMPURPLE,
  GAL_COLOR_ORCHID,
  GAL_COLOR_VIOLET,
  GAL_COLOR_PLUM,
  GAL_COLOR_THISTLE,
  GAL_COLOR_LAVENDER,

  /* White colors */
  GAL_COLOR_MISTYROSE,
  GAL_COLOR_ANTIQUEWHITE,
  GAL_COLOR_LINEN,
  GAL_COLOR_BEIGE,
  GAL_COLOR_WHITESMOKE,
  GAL_COLOR_LAVENDERBLUSH,
  GAL_COLOR_OLDLACE,
  GAL_COLOR_ALICEBLUE,
  GAL_COLOR_SEASHELL,
  GAL_COLOR_GHOSTWHITE,
  GAL_COLOR_HONEYDEW,
  GAL_COLOR_FLORALWHITE,
  GAL_COLOR_AZURE,
  GAL_COLOR_MINTCREAM,
  GAL_COLOR_SNOW,
  GAL_COLOR_IVORY,
  GAL_COLOR_WHITE,

  /* Gray and black colors. */
  GAL_COLOR_BLACK,
  GAL_COLOR_DARKSLATEGRAY,
  GAL_COLOR_DIMGRAY,
  GAL_COLOR_SLATEGRAY,
  GAL_COLOR_GRAY,
  GAL_COLOR_LIGHTSLATEGRAY,
  GAL_COLOR_DARKGRAY,
  GAL_COLOR_SILVER,
  GAL_COLOR_LIGHTGRAY,
  GAL_COLOR_GAINSBORO,

  /* Final number of pre-defined colors. */
  GAL_COLOR_NUMBER,
};





/* Functions */
uint8_t
gal_color_name_to_id(char *n);

char *
gal_color_id_to_name(uint8_t color);

void
gal_color_in_rgb(uint8_t color, float *f);




__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_COLOR_H__ */
