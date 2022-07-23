/*********************************************************************
Functions for working with colors.
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
#include <stdlib.h>
#include <string.h>

#include <gnuastro/color.h>
#include <gnuastro/blank.h>





/***********************************************************************/
/**************             Names and values           *****************/
/***********************************************************************/
/* Return the macros for the extended HTML/CSS colors based on their names:
   'https://en.wikipedia.org/wiki/Web_colors#HTML_color_names'.*/
uint8_t
gal_color_name_to_id(char *n)
{
  if(      !strcasecmp("aliceblue",  n)) return GAL_COLOR_ALICEBLUE;
  else if( !strcasecmp("antiquewhite", n)) return GAL_COLOR_ANTIQUEWHITE;
  else if( !strcasecmp("aqua",       n)) return GAL_COLOR_AQUA;
  else if( !strcasecmp("aquamarine", n)) return GAL_COLOR_AQUAMARINE;
  else if( !strcasecmp("azure",      n)) return GAL_COLOR_AZURE;
  else if( !strcasecmp("beige",      n)) return GAL_COLOR_BEIGE;
  else if( !strcasecmp("bisque",     n)) return GAL_COLOR_BISQUE;
  else if( !strcasecmp("black",      n)) return GAL_COLOR_BLACK;
  else if( !strcasecmp("blanchedalmond", n)) return GAL_COLOR_BLANCHEDALMOND;
  else if( !strcasecmp("blue",       n)) return GAL_COLOR_BLUE;
  else if( !strcasecmp("blueviolet", n)) return GAL_COLOR_BLUEVIOLET;
  else if( !strcasecmp("brown",      n)) return GAL_COLOR_BROWN;
  else if( !strcasecmp("burlywood",  n)) return GAL_COLOR_BURLYWOOD;
  else if( !strcasecmp("cadetblue",  n)) return GAL_COLOR_CADETBLUE;
  else if( !strcasecmp("chartreuse", n)) return GAL_COLOR_CHARTREUSE;
  else if( !strcasecmp("chocolate",  n)) return GAL_COLOR_CHOCOLATE;
  else if( !strcasecmp("coral",      n)) return GAL_COLOR_CORAL;
  else if( !strcasecmp("cornflowerblue", n)) return GAL_COLOR_CORNFLOWERBLUE;
  else if( !strcasecmp("cornsilk",   n)) return GAL_COLOR_CORNSILK;
  else if( !strcasecmp("crimson",    n)) return GAL_COLOR_CRIMSON;
  else if( !strcasecmp("cyan",       n)) return GAL_COLOR_CYAN;
  else if( !strcasecmp("darkblue",   n)) return GAL_COLOR_DARKBLUE;
  else if( !strcasecmp("darkcyan",   n)) return GAL_COLOR_DARKCYAN;
  else if( !strcasecmp("darkgoldenrod", n)) return GAL_COLOR_DARKGOLDENROD;
  else if( !strcasecmp("darkgray",   n)) return GAL_COLOR_DARKGRAY;
  else if( !strcasecmp("darkgreen",  n)) return GAL_COLOR_DARKGREEN;
  else if( !strcasecmp("darkkhaki",  n)) return GAL_COLOR_DARKKHAKI;
  else if( !strcasecmp("darkmagenta", n)) return GAL_COLOR_DARKMAGENTA;
  else if( !strcasecmp("darkolivegreen", n)) return GAL_COLOR_DARKOLIVEGREEN;
  else if( !strcasecmp("darkorange", n)) return GAL_COLOR_DARKORANGE;
  else if( !strcasecmp("darkorchid", n)) return GAL_COLOR_DARKORCHID;
  else if( !strcasecmp("darkred",    n)) return GAL_COLOR_DARKRED;
  else if( !strcasecmp("darksalmon", n)) return GAL_COLOR_DARKSALMON;
  else if( !strcasecmp("darkseagreen", n)) return GAL_COLOR_DARKSEAGREEN;
  else if( !strcasecmp("darkslateblue", n)) return GAL_COLOR_DARKSLATEBLUE;
  else if( !strcasecmp("darkslategray", n)) return GAL_COLOR_DARKSLATEGRAY;
  else if( !strcasecmp("darkturquoise", n)) return GAL_COLOR_DARKTURQUOISE;
  else if( !strcasecmp("darkviolet", n)) return GAL_COLOR_DARKVIOLET;
  else if( !strcasecmp("deeppink",   n)) return GAL_COLOR_DEEPPINK;
  else if( !strcasecmp("deepskyblue", n)) return GAL_COLOR_DEEPSKYBLUE;
  else if( !strcasecmp("dimgray",    n)) return GAL_COLOR_DIMGRAY;
  else if( !strcasecmp("dodgerblue", n)) return GAL_COLOR_DODGERBLUE;
  else if( !strcasecmp("firebrick",  n)) return GAL_COLOR_FIREBRICK;
  else if( !strcasecmp("floralwhite", n)) return GAL_COLOR_FLORALWHITE;
  else if( !strcasecmp("forestgreen", n)) return GAL_COLOR_FORESTGREEN;
  else if( !strcasecmp("fuchsia",    n)) return GAL_COLOR_FUCHSIA;
  else if( !strcasecmp("gainsboro",  n)) return GAL_COLOR_GAINSBORO;
  else if( !strcasecmp("ghostwhite", n)) return GAL_COLOR_GHOSTWHITE;
  else if( !strcasecmp("gold",       n)) return GAL_COLOR_GOLD;
  else if( !strcasecmp("goldenrod",  n)) return GAL_COLOR_GOLDENROD;
  else if( !strcasecmp("gray",       n)) return GAL_COLOR_GRAY;
  else if( !strcasecmp("green",      n)) return GAL_COLOR_GREEN;
  else if( !strcasecmp("greenyellow", n)) return GAL_COLOR_GREENYELLOW;
  else if( !strcasecmp("honeydew",   n)) return GAL_COLOR_HONEYDEW;
  else if( !strcasecmp("hotpink",    n)) return GAL_COLOR_HOTPINK;
  else if( !strcasecmp("indianred",  n)) return GAL_COLOR_INDIANRED;
  else if( !strcasecmp("indigo",     n)) return GAL_COLOR_INDIGO;
  else if( !strcasecmp("ivory",      n)) return GAL_COLOR_IVORY;
  else if( !strcasecmp("khaki",      n)) return GAL_COLOR_KHAKI;
  else if( !strcasecmp("lavender",   n)) return GAL_COLOR_LAVENDER;
  else if( !strcasecmp("lavenderblush", n)) return GAL_COLOR_LAVENDERBLUSH;
  else if( !strcasecmp("lawngreen",  n)) return GAL_COLOR_LAWNGREEN;
  else if( !strcasecmp("lemonchiffon", n)) return GAL_COLOR_LEMONCHIFFON;
  else if( !strcasecmp("lightblue",  n)) return GAL_COLOR_LIGHTBLUE;
  else if( !strcasecmp("lightcoral", n)) return GAL_COLOR_LIGHTCORAL;
  else if( !strcasecmp("lightcyan",  n)) return GAL_COLOR_LIGHTCYAN;
  else if( !strcasecmp("lightgoldenrodyellow", n)) return GAL_COLOR_LIGHTGOLDENRODYELLOW;
  else if( !strcasecmp("lightgray",  n)) return GAL_COLOR_LIGHTGRAY;
  else if( !strcasecmp("lightgreen", n)) return GAL_COLOR_LIGHTGREEN;
  else if( !strcasecmp("lightpink",  n)) return GAL_COLOR_LIGHTPINK;
  else if( !strcasecmp("lightsalmon", n)) return GAL_COLOR_LIGHTSALMON;
  else if( !strcasecmp("lightseagreen", n)) return GAL_COLOR_LIGHTSEAGREEN;
  else if( !strcasecmp("lightskyblue", n)) return GAL_COLOR_LIGHTSKYBLUE;
  else if( !strcasecmp("lightslategray", n)) return GAL_COLOR_LIGHTSLATEGRAY;
  else if( !strcasecmp("lightsteelblue", n)) return GAL_COLOR_LIGHTSTEELBLUE;
  else if( !strcasecmp("lightyellow", n)) return GAL_COLOR_LIGHTYELLOW;
  else if( !strcasecmp("lime",       n)) return GAL_COLOR_LIME;
  else if( !strcasecmp("limegreen",  n)) return GAL_COLOR_LIMEGREEN;
  else if( !strcasecmp("linen",      n)) return GAL_COLOR_LINEN;
  else if( !strcasecmp("magenta",    n)) return GAL_COLOR_MAGENTA;
  else if( !strcasecmp("maroon",     n)) return GAL_COLOR_MAROON;
  else if( !strcasecmp("mediumaquamarine", n)) return GAL_COLOR_MEDIUMAQUAMARINE;
  else if( !strcasecmp("mediumblue", n)) return GAL_COLOR_MEDIUMBLUE;
  else if( !strcasecmp("mediumorchid", n)) return GAL_COLOR_MEDIUMORCHID;
  else if( !strcasecmp("mediumpurple", n)) return GAL_COLOR_MEDIUMPURPLE;
  else if( !strcasecmp("mediumseagreen", n)) return GAL_COLOR_MEDIUMSEAGREEN;
  else if( !strcasecmp("mediumslateblue", n)) return GAL_COLOR_MEDIUMSLATEBLUE;
  else if( !strcasecmp("mediumspringgreen", n)) return GAL_COLOR_MEDIUMSPRINGGREEN;
  else if( !strcasecmp("mediumturquoise", n)) return GAL_COLOR_MEDIUMTURQUOISE;
  else if( !strcasecmp("mediumvioletred", n)) return GAL_COLOR_MEDIUMVIOLETRED;
  else if( !strcasecmp("midnightblue", n)) return GAL_COLOR_MIDNIGHTBLUE;
  else if( !strcasecmp("mintcream",  n)) return GAL_COLOR_MINTCREAM;
  else if( !strcasecmp("mistyrose",  n)) return GAL_COLOR_MISTYROSE;
  else if( !strcasecmp("moccasin",   n)) return GAL_COLOR_MOCCASIN;
  else if( !strcasecmp("navajowhite", n)) return GAL_COLOR_NAVAJOWHITE;
  else if( !strcasecmp("navy",       n)) return GAL_COLOR_NAVY;
  else if( !strcasecmp("oldlace",    n)) return GAL_COLOR_OLDLACE;
  else if( !strcasecmp("olive",      n)) return GAL_COLOR_OLIVE;
  else if( !strcasecmp("olivedrab",  n)) return GAL_COLOR_OLIVEDRAB;
  else if( !strcasecmp("orange",     n)) return GAL_COLOR_ORANGE;
  else if( !strcasecmp("orangered",  n)) return GAL_COLOR_ORANGERED;
  else if( !strcasecmp("orchid",     n)) return GAL_COLOR_ORCHID;
  else if( !strcasecmp("palegoldenrod", n)) return GAL_COLOR_PALEGOLDENROD;
  else if( !strcasecmp("palegreen",  n)) return GAL_COLOR_PALEGREEN;
  else if( !strcasecmp("paleturquoise", n)) return GAL_COLOR_PALETURQUOISE;
  else if( !strcasecmp("palevioletred", n)) return GAL_COLOR_PALEVIOLETRED;
  else if( !strcasecmp("papayawhip", n)) return GAL_COLOR_PAPAYAWHIP;
  else if( !strcasecmp("peachpuff",  n)) return GAL_COLOR_PEACHPUFF;
  else if( !strcasecmp("peru",       n)) return GAL_COLOR_PERU;
  else if( !strcasecmp("pink",       n)) return GAL_COLOR_PINK;
  else if( !strcasecmp("plum",       n)) return GAL_COLOR_PLUM;
  else if( !strcasecmp("powderblue", n)) return GAL_COLOR_POWDERBLUE;
  else if( !strcasecmp("purple",     n)) return GAL_COLOR_PURPLE;
  else if( !strcasecmp("red",        n)) return GAL_COLOR_RED;
  else if( !strcasecmp("rosybrown",  n)) return GAL_COLOR_ROSYBROWN;
  else if( !strcasecmp("royalblue",  n)) return GAL_COLOR_ROYALBLUE;
  else if( !strcasecmp("saddlebrown", n)) return GAL_COLOR_SADDLEBROWN;
  else if( !strcasecmp("salmon",     n)) return GAL_COLOR_SALMON;
  else if( !strcasecmp("sandybrown", n)) return GAL_COLOR_SANDYBROWN;
  else if( !strcasecmp("seagreen",   n)) return GAL_COLOR_SEAGREEN;
  else if( !strcasecmp("seashell",   n)) return GAL_COLOR_SEASHELL;
  else if( !strcasecmp("sienna",     n)) return GAL_COLOR_SIENNA;
  else if( !strcasecmp("silver",     n)) return GAL_COLOR_SILVER;
  else if( !strcasecmp("skyblue",    n)) return GAL_COLOR_SKYBLUE;
  else if( !strcasecmp("slateblue",  n)) return GAL_COLOR_SLATEBLUE;
  else if( !strcasecmp("slategray",  n)) return GAL_COLOR_SLATEGRAY;
  else if( !strcasecmp("snow",       n)) return GAL_COLOR_SNOW;
  else if( !strcasecmp("springgreen", n)) return GAL_COLOR_SPRINGGREEN;
  else if( !strcasecmp("steelblue",  n)) return GAL_COLOR_STEELBLUE;
  else if( !strcasecmp("tan",        n)) return GAL_COLOR_TAN;
  else if( !strcasecmp("teal",       n)) return GAL_COLOR_TEAL;
  else if( !strcasecmp("thistle",    n)) return GAL_COLOR_THISTLE;
  else if( !strcasecmp("tomato",     n)) return GAL_COLOR_TOMATO;
  else if( !strcasecmp("turquoise",  n)) return GAL_COLOR_TURQUOISE;
  else if( !strcasecmp("violet",     n)) return GAL_COLOR_VIOLET;
  else if( !strcasecmp("wheat",      n)) return GAL_COLOR_WHEAT;
  else if( !strcasecmp("white",      n)) return GAL_COLOR_WHITE;
  else if( !strcasecmp("whitesmoke", n)) return GAL_COLOR_WHITESMOKE;
  else if( !strcasecmp("yellow",     n)) return GAL_COLOR_YELLOW;
  else if( !strcasecmp("yellowgreen", n)) return GAL_COLOR_YELLOWGREEN;
  else
    printf("%s: name '%s' is not recognized. Gnuastro uses the "
           "extended HTML color names described in the following "
           "link (the names are not case sensitive in Gnuastro):"
           "https://en.wikipedia.org/wiki/Web_colors#Extended_colors\n",
           __func__, n);

  /* Control should not reach here. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to find "
        "and solve the problem. Control should not reach this part "
        "of the function", __func__, PACKAGE_BUGREPORT);
  return GAL_COLOR_INVALID;
}





/* Return the name of each color. */
char *
gal_color_id_to_name(uint8_t color)
{
  switch(color)
    {
    case GAL_COLOR_ALICEBLUE:            return "aliceblue";
    case GAL_COLOR_ANTIQUEWHITE:         return "antiquewhite";
    case GAL_COLOR_AQUA:                 return "aqua";
    case GAL_COLOR_AQUAMARINE:           return "aquamarine";
    case GAL_COLOR_AZURE:                return "azure";
    case GAL_COLOR_BEIGE:                return "beige";
    case GAL_COLOR_BISQUE:               return "bisque";
    case GAL_COLOR_BLACK:                return "black";
    case GAL_COLOR_BLANCHEDALMOND:       return "blanchedalmond";
    case GAL_COLOR_BLUE:                 return "blue";
    case GAL_COLOR_BLUEVIOLET:           return "blueviolet";
    case GAL_COLOR_BROWN:                return "brown";
    case GAL_COLOR_BURLYWOOD:            return "burlywood";
    case GAL_COLOR_CADETBLUE:            return "cadetblue";
    case GAL_COLOR_CHARTREUSE:           return "chartreuse";
    case GAL_COLOR_CHOCOLATE:            return "chocolate";
    case GAL_COLOR_CORAL:                return "coral";
    case GAL_COLOR_CORNFLOWERBLUE:       return "cornflowerblue";
    case GAL_COLOR_CORNSILK:             return "cornsilk";
    case GAL_COLOR_CRIMSON:              return "crimson";
    case GAL_COLOR_CYAN:                 return "cyan";
    case GAL_COLOR_DARKBLUE:             return "darkblue";
    case GAL_COLOR_DARKCYAN:             return "darkcyan";
    case GAL_COLOR_DARKGOLDENROD:        return "darkgoldenrod";
    case GAL_COLOR_DARKGRAY:             return "darkgray";
    case GAL_COLOR_DARKGREEN:            return "darkgreen";
    case GAL_COLOR_DARKKHAKI:            return "darkkhaki";
    case GAL_COLOR_DARKMAGENTA:          return "darkmagenta";
    case GAL_COLOR_DARKOLIVEGREEN:       return "darkolivegreen";
    case GAL_COLOR_DARKORANGE:           return "darkorange";
    case GAL_COLOR_DARKORCHID:           return "darkorchid";
    case GAL_COLOR_DARKRED:              return "darkred";
    case GAL_COLOR_DARKSALMON:           return "darksalmon";
    case GAL_COLOR_DARKSEAGREEN:         return "darkseagreen";
    case GAL_COLOR_DARKSLATEBLUE:        return "darkslateblue";
    case GAL_COLOR_DARKSLATEGRAY:        return "darkslategray";
    case GAL_COLOR_DARKTURQUOISE:        return "darkturquoise";
    case GAL_COLOR_DARKVIOLET:           return "darkviolet";
    case GAL_COLOR_DEEPPINK:             return "deeppink";
    case GAL_COLOR_DEEPSKYBLUE:          return "deepskyblue";
    case GAL_COLOR_DIMGRAY:              return "dimgray";
    case GAL_COLOR_DODGERBLUE:           return "dodgerblue";
    case GAL_COLOR_FIREBRICK:            return "firebrick";
    case GAL_COLOR_FLORALWHITE:          return "floralwhite";
    case GAL_COLOR_FORESTGREEN:          return "forestgreen";
    case GAL_COLOR_FUCHSIA:              return "fuchsia";
    case GAL_COLOR_GAINSBORO:            return "gainsboro";
    case GAL_COLOR_GHOSTWHITE:           return "ghostwhite";
    case GAL_COLOR_GOLD:                 return "gold";
    case GAL_COLOR_GOLDENROD:            return "goldenrod";
    case GAL_COLOR_GRAY:                 return "gray";
    case GAL_COLOR_GREEN:                return "green";
    case GAL_COLOR_GREENYELLOW:          return "greenyellow";
    case GAL_COLOR_HONEYDEW:             return "honeydew";
    case GAL_COLOR_HOTPINK:              return "hotpink";
    case GAL_COLOR_INDIANRED:            return "indianred";
    case GAL_COLOR_INDIGO:               return "indigo";
    case GAL_COLOR_IVORY:                return "ivory";
    case GAL_COLOR_KHAKI:                return "khaki";
    case GAL_COLOR_LAVENDER:             return "lavender";
    case GAL_COLOR_LAVENDERBLUSH:        return "lavenderblush";
    case GAL_COLOR_LAWNGREEN:            return "lawngreen";
    case GAL_COLOR_LEMONCHIFFON:         return "lemonchiffon";
    case GAL_COLOR_LIGHTBLUE:            return "lightblue";
    case GAL_COLOR_LIGHTCORAL:           return "lightcoral";
    case GAL_COLOR_LIGHTCYAN:            return "lightcyan";
    case GAL_COLOR_LIGHTGOLDENRODYELLOW: return "lightgoldenrodyellow";
    case GAL_COLOR_LIGHTGRAY:            return "lightgray";
    case GAL_COLOR_LIGHTGREEN:           return "lightgreen";
    case GAL_COLOR_LIGHTPINK:            return "lightpink";
    case GAL_COLOR_LIGHTSALMON:          return "lightsalmon";
    case GAL_COLOR_LIGHTSEAGREEN:        return "lightseagreen";
    case GAL_COLOR_LIGHTSKYBLUE:         return "lightskyblue";
    case GAL_COLOR_LIGHTSLATEGRAY:       return "lightslategray";
    case GAL_COLOR_LIGHTSTEELBLUE:       return "lightsteelblue";
    case GAL_COLOR_LIGHTYELLOW:          return "lightyellow";
    case GAL_COLOR_LIME:                 return "lime";
    case GAL_COLOR_LIMEGREEN:            return "limegreen";
    case GAL_COLOR_LINEN:                return "linen";
    case GAL_COLOR_MAGENTA:              return "magenta";
    case GAL_COLOR_MAROON:               return "maroon";
    case GAL_COLOR_MEDIUMAQUAMARINE:     return "mediumaquamarine";
    case GAL_COLOR_MEDIUMBLUE:           return "mediumblue";
    case GAL_COLOR_MEDIUMORCHID:         return "mediumorchid";
    case GAL_COLOR_MEDIUMPURPLE:         return "mediumpurple";
    case GAL_COLOR_MEDIUMSEAGREEN:       return "mediumseagreen";
    case GAL_COLOR_MEDIUMSLATEBLUE:      return "mediumslateblue";
    case GAL_COLOR_MEDIUMSPRINGGREEN:    return "mediumspringgreen";
    case GAL_COLOR_MEDIUMTURQUOISE:      return "mediumturquoise";
    case GAL_COLOR_MEDIUMVIOLETRED:      return "mediumvioletred";
    case GAL_COLOR_MIDNIGHTBLUE:         return "midnightblue";
    case GAL_COLOR_MINTCREAM:            return "mintcream";
    case GAL_COLOR_MISTYROSE:            return "mistyrose";
    case GAL_COLOR_MOCCASIN:             return "moccasin";
    case GAL_COLOR_NAVAJOWHITE:          return "navajowhite";
    case GAL_COLOR_NAVY:                 return "navy";
    case GAL_COLOR_OLDLACE:              return "oldlace";
    case GAL_COLOR_OLIVE:                return "olive";
    case GAL_COLOR_OLIVEDRAB:            return "olivedrab";
    case GAL_COLOR_ORANGE:               return "orange";
    case GAL_COLOR_ORANGERED:            return "orangered";
    case GAL_COLOR_ORCHID:               return "orchid";
    case GAL_COLOR_PALEGOLDENROD:        return "palegoldenrod";
    case GAL_COLOR_PALEGREEN:            return "palegreen";
    case GAL_COLOR_PALETURQUOISE:        return "paleturquoise";
    case GAL_COLOR_PALEVIOLETRED:        return "palevioletred";
    case GAL_COLOR_PAPAYAWHIP:           return "papayawhip";
    case GAL_COLOR_PEACHPUFF:            return "peachpuff";
    case GAL_COLOR_PERU:                 return "peru";
    case GAL_COLOR_PINK:                 return "pink";
    case GAL_COLOR_PLUM:                 return "plum";
    case GAL_COLOR_POWDERBLUE:           return "powderblue";
    case GAL_COLOR_PURPLE:               return "purple";
    case GAL_COLOR_RED:                  return "red";
    case GAL_COLOR_ROSYBROWN:            return "rosybrown";
    case GAL_COLOR_ROYALBLUE:            return "royalblue";
    case GAL_COLOR_SADDLEBROWN:          return "saddlebrown";
    case GAL_COLOR_SALMON:               return "salmon";
    case GAL_COLOR_SANDYBROWN:           return "sandybrown";
    case GAL_COLOR_SEAGREEN:             return "seagreen";
    case GAL_COLOR_SEASHELL:             return "seashell";
    case GAL_COLOR_SIENNA:               return "sienna";
    case GAL_COLOR_SILVER:               return "silver";
    case GAL_COLOR_SKYBLUE:              return "skyblue";
    case GAL_COLOR_SLATEBLUE:            return "slateblue";
    case GAL_COLOR_SLATEGRAY:            return "slategray";
    case GAL_COLOR_SNOW:                 return "snow";
    case GAL_COLOR_SPRINGGREEN:          return "springgreen";
    case GAL_COLOR_STEELBLUE:            return "steelblue";
    case GAL_COLOR_TAN:                  return "tan";
    case GAL_COLOR_TEAL:                 return "teal";
    case GAL_COLOR_THISTLE:              return "thistle";
    case GAL_COLOR_TOMATO:               return "tomato";
    case GAL_COLOR_TURQUOISE:            return "turquoise";
    case GAL_COLOR_VIOLET:               return "violet";
    case GAL_COLOR_WHEAT:                return "wheat";
    case GAL_COLOR_WHITE:                return "white";
    case GAL_COLOR_WHITESMOKE:           return "whitesmoke";
    case GAL_COLOR_YELLOW:               return "yellow";
    case GAL_COLOR_YELLOWGREEN:          return "yellowgreen";
    default:
      error(EXIT_FAILURE, 0, "%s: color id '%u' is not recognized",
            __func__, color);
    }

  /* Control should not reach here. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to find "
        "and solve the problem. Control should not reach this part "
        "of the function", __func__, PACKAGE_BUGREPORT);
  return GAL_BLANK_STRING;
}





/* Return the fractional RGB color space of pre-defined colors. We are
   using 'n' for "name" and 'f' for RGB-Fractional value to help in
   readability. The names and values are taken from the extended HTML/CSS
   colors: https://en.wikipedia.org/wiki/Web_colors#HTML_color_names.

   After copying the names and values into a plain-text file (by mouse,
   group by group, 'color.txt'), I converted it to the format used in this
   function with the following AWK command:

      awk '{printf "case GAL_COLOR_%s: ", toupper($1); \
            printf "f[0]=%.2f; f[1]=%.2f; f[2]=%.2f; break;\n", \
                   $5/255, $6/255, $7/255}' colors.txt \
                   | sort \
                   | awk '{printf "%s %-25s %s %s %s %s\n", \
                                  $1, $2, $3, $4, $5, $6}'
*/
void
gal_color_in_rgb(uint8_t color, float *f)
{
  switch(color)
    {
    case GAL_COLOR_ALICEBLUE:      f[0]=0.94; f[1]=0.97; f[2]=1.00; break;
    case GAL_COLOR_ANTIQUEWHITE:   f[0]=0.98; f[1]=0.92; f[2]=0.84; break;
    case GAL_COLOR_AQUA:           f[0]=0.00; f[1]=1.00; f[2]=1.00; break;
    case GAL_COLOR_AQUAMARINE:     f[0]=0.50; f[1]=1.00; f[2]=0.83; break;
    case GAL_COLOR_AZURE:          f[0]=0.94; f[1]=1.00; f[2]=1.00; break;
    case GAL_COLOR_BEIGE:          f[0]=0.96; f[1]=0.96; f[2]=0.86; break;
    case GAL_COLOR_BISQUE:         f[0]=1.00; f[1]=0.89; f[2]=0.77; break;
    case GAL_COLOR_BLACK:          f[0]=0.00; f[1]=0.00; f[2]=0.00; break;
    case GAL_COLOR_BLANCHEDALMOND: f[0]=1.00; f[1]=0.92; f[2]=0.80; break;
    case GAL_COLOR_BLUE:           f[0]=0.00; f[1]=0.00; f[2]=1.00; break;
    case GAL_COLOR_BLUEVIOLET:     f[0]=0.54; f[1]=0.17; f[2]=0.89; break;
    case GAL_COLOR_BROWN:          f[0]=0.65; f[1]=0.16; f[2]=0.16; break;
    case GAL_COLOR_BURLYWOOD:      f[0]=0.87; f[1]=0.72; f[2]=0.53; break;
    case GAL_COLOR_CADETBLUE:      f[0]=0.37; f[1]=0.62; f[2]=0.63; break;
    case GAL_COLOR_CHARTREUSE:     f[0]=0.50; f[1]=1.00; f[2]=0.00; break;
    case GAL_COLOR_CHOCOLATE:      f[0]=0.82; f[1]=0.41; f[2]=0.12; break;
    case GAL_COLOR_CORAL:          f[0]=1.00; f[1]=0.50; f[2]=0.31; break;
    case GAL_COLOR_CORNFLOWERBLUE: f[0]=0.39; f[1]=0.58; f[2]=0.93; break;
    case GAL_COLOR_CORNSILK:       f[0]=1.00; f[1]=0.97; f[2]=0.86; break;
    case GAL_COLOR_CRIMSON:        f[0]=0.86; f[1]=0.08; f[2]=0.24; break;
    case GAL_COLOR_CYAN:           f[0]=0.00; f[1]=1.00; f[2]=1.00; break;
    case GAL_COLOR_DARKBLUE:       f[0]=0.00; f[1]=0.00; f[2]=0.55; break;
    case GAL_COLOR_DARKCYAN:       f[0]=0.00; f[1]=0.55; f[2]=0.55; break;
    case GAL_COLOR_DARKGOLDENROD:  f[0]=0.72; f[1]=0.53; f[2]=0.04; break;
    case GAL_COLOR_DARKGRAY:       f[0]=0.66; f[1]=0.66; f[2]=0.66; break;
    case GAL_COLOR_DARKGREEN:      f[0]=0.00; f[1]=0.39; f[2]=0.00; break;
    case GAL_COLOR_DARKKHAKI:      f[0]=0.74; f[1]=0.72; f[2]=0.42; break;
    case GAL_COLOR_DARKMAGENTA:    f[0]=0.55; f[1]=0.00; f[2]=0.55; break;
    case GAL_COLOR_DARKOLIVEGREEN: f[0]=0.33; f[1]=0.42; f[2]=0.18; break;
    case GAL_COLOR_DARKORANGE:     f[0]=1.00; f[1]=0.55; f[2]=0.00; break;
    case GAL_COLOR_DARKORCHID:     f[0]=0.60; f[1]=0.20; f[2]=0.80; break;
    case GAL_COLOR_DARKRED:        f[0]=0.55; f[1]=0.00; f[2]=0.00; break;
    case GAL_COLOR_DARKSALMON:     f[0]=0.91; f[1]=0.59; f[2]=0.48; break;
    case GAL_COLOR_DARKSEAGREEN:   f[0]=0.56; f[1]=0.74; f[2]=0.56; break;
    case GAL_COLOR_DARKSLATEBLUE:  f[0]=0.28; f[1]=0.24; f[2]=0.55; break;
    case GAL_COLOR_DARKSLATEGRAY:  f[0]=0.18; f[1]=0.31; f[2]=0.31; break;
    case GAL_COLOR_DARKTURQUOISE:  f[0]=0.00; f[1]=0.81; f[2]=0.82; break;
    case GAL_COLOR_DARKVIOLET:     f[0]=0.58; f[1]=0.00; f[2]=0.83; break;
    case GAL_COLOR_DEEPPINK:       f[0]=1.00; f[1]=0.08; f[2]=0.58; break;
    case GAL_COLOR_DEEPSKYBLUE:    f[0]=0.00; f[1]=0.75; f[2]=1.00; break;
    case GAL_COLOR_DIMGRAY:        f[0]=0.41; f[1]=0.41; f[2]=0.41; break;
    case GAL_COLOR_DODGERBLUE:     f[0]=0.12; f[1]=0.56; f[2]=1.00; break;
    case GAL_COLOR_FIREBRICK:      f[0]=0.70; f[1]=0.13; f[2]=0.13; break;
    case GAL_COLOR_FLORALWHITE:    f[0]=1.00; f[1]=0.98; f[2]=0.94; break;
    case GAL_COLOR_FORESTGREEN:    f[0]=0.13; f[1]=0.55; f[2]=0.13; break;
    case GAL_COLOR_FUCHSIA:        f[0]=1.00; f[1]=0.00; f[2]=1.00; break;
    case GAL_COLOR_GAINSBORO:      f[0]=0.86; f[1]=0.86; f[2]=0.86; break;
    case GAL_COLOR_GHOSTWHITE:     f[0]=0.97; f[1]=0.97; f[2]=1.00; break;
    case GAL_COLOR_GOLDENROD:      f[0]=0.85; f[1]=0.65; f[2]=0.13; break;
    case GAL_COLOR_GOLD:           f[0]=1.00; f[1]=0.84; f[2]=0.00; break;
    case GAL_COLOR_GRAY:           f[0]=0.50; f[1]=0.50; f[2]=0.50; break;
    case GAL_COLOR_GREEN:          f[0]=0.00; f[1]=0.50; f[2]=0.00; break;
    case GAL_COLOR_GREENYELLOW:    f[0]=0.68; f[1]=1.00; f[2]=0.18; break;
    case GAL_COLOR_HONEYDEW:       f[0]=0.94; f[1]=1.00; f[2]=0.94; break;
    case GAL_COLOR_HOTPINK:        f[0]=1.00; f[1]=0.41; f[2]=0.71; break;
    case GAL_COLOR_INDIANRED:      f[0]=0.80; f[1]=0.36; f[2]=0.36; break;
    case GAL_COLOR_INDIGO:         f[0]=0.29; f[1]=0.00; f[2]=0.51; break;
    case GAL_COLOR_IVORY:          f[0]=1.00; f[1]=1.00; f[2]=0.94; break;
    case GAL_COLOR_KHAKI:          f[0]=0.94; f[1]=0.90; f[2]=0.55; break;
    case GAL_COLOR_LAVENDERBLUSH:  f[0]=1.00; f[1]=0.94; f[2]=0.96; break;
    case GAL_COLOR_LAVENDER:       f[0]=0.90; f[1]=0.90; f[2]=0.98; break;
    case GAL_COLOR_LAWNGREEN:      f[0]=0.49; f[1]=0.99; f[2]=0.00; break;
    case GAL_COLOR_LEMONCHIFFON:   f[0]=1.00; f[1]=0.98; f[2]=0.80; break;
    case GAL_COLOR_LIGHTBLUE:      f[0]=0.68; f[1]=0.85; f[2]=0.90; break;
    case GAL_COLOR_LIGHTCORAL:     f[0]=0.94; f[1]=0.50; f[2]=0.50; break;
    case GAL_COLOR_LIGHTCYAN:      f[0]=0.88; f[1]=1.00; f[2]=1.00; break;
    case GAL_COLOR_LIGHTGOLDENRODYELLOW: f[0]=0.98; f[1]=0.98; f[2]=0.82; break;
    case GAL_COLOR_LIGHTGRAY:      f[0]=0.83; f[1]=0.83; f[2]=0.83; break;
    case GAL_COLOR_LIGHTGREEN:     f[0]=0.56; f[1]=0.93; f[2]=0.56; break;
    case GAL_COLOR_LIGHTPINK:      f[0]=1.00; f[1]=0.71; f[2]=0.76; break;
    case GAL_COLOR_LIGHTSALMON:    f[0]=1.00; f[1]=0.63; f[2]=0.48; break;
    case GAL_COLOR_LIGHTSEAGREEN:  f[0]=0.13; f[1]=0.70; f[2]=0.67; break;
    case GAL_COLOR_LIGHTSKYBLUE:   f[0]=0.53; f[1]=0.81; f[2]=0.98; break;
    case GAL_COLOR_LIGHTSLATEGRAY: f[0]=0.47; f[1]=0.53; f[2]=0.60; break;
    case GAL_COLOR_LIGHTSTEELBLUE: f[0]=0.69; f[1]=0.77; f[2]=0.87; break;
    case GAL_COLOR_LIGHTYELLOW:    f[0]=1.00; f[1]=1.00; f[2]=0.88; break;
    case GAL_COLOR_LIME:           f[0]=0.00; f[1]=1.00; f[2]=0.00; break;
    case GAL_COLOR_LIMEGREEN:      f[0]=0.20; f[1]=0.80; f[2]=0.20; break;
    case GAL_COLOR_LINEN:          f[0]=0.98; f[1]=0.94; f[2]=0.90; break;
    case GAL_COLOR_MAGENTA:        f[0]=1.00; f[1]=0.00; f[2]=1.00; break;
    case GAL_COLOR_MAROON:         f[0]=0.50; f[1]=0.00; f[2]=0.00; break;
    case GAL_COLOR_MEDIUMAQUAMARINE: f[0]=0.40; f[1]=0.80; f[2]=0.67; break;
    case GAL_COLOR_MEDIUMBLUE:     f[0]=0.00; f[1]=0.00; f[2]=0.80; break;
    case GAL_COLOR_MEDIUMORCHID:   f[0]=0.73; f[1]=0.33; f[2]=0.83; break;
    case GAL_COLOR_MEDIUMPURPLE:   f[0]=0.58; f[1]=0.44; f[2]=0.86; break;
    case GAL_COLOR_MEDIUMSEAGREEN: f[0]=0.24; f[1]=0.70; f[2]=0.44; break;
    case GAL_COLOR_MEDIUMSLATEBLUE: f[0]=0.48; f[1]=0.41; f[2]=0.93; break;
    case GAL_COLOR_MEDIUMSPRINGGREEN: f[0]=0.00; f[1]=0.98; f[2]=0.60; break;
    case GAL_COLOR_MEDIUMTURQUOISE: f[0]=0.28; f[1]=0.82; f[2]=0.80; break;
    case GAL_COLOR_MEDIUMVIOLETRED: f[0]=0.78; f[1]=0.08; f[2]=0.52; break;
    case GAL_COLOR_MIDNIGHTBLUE:   f[0]=0.10; f[1]=0.10; f[2]=0.44; break;
    case GAL_COLOR_MINTCREAM:      f[0]=0.96; f[1]=1.00; f[2]=0.98; break;
    case GAL_COLOR_MISTYROSE:      f[0]=1.00; f[1]=0.89; f[2]=0.88; break;
    case GAL_COLOR_MOCCASIN:       f[0]=1.00; f[1]=0.89; f[2]=0.71; break;
    case GAL_COLOR_NAVAJOWHITE:    f[0]=1.00; f[1]=0.87; f[2]=0.68; break;
    case GAL_COLOR_NAVY:           f[0]=0.00; f[1]=0.00; f[2]=0.50; break;
    case GAL_COLOR_OLDLACE:        f[0]=0.99; f[1]=0.96; f[2]=0.90; break;
    case GAL_COLOR_OLIVEDRAB:      f[0]=0.42; f[1]=0.56; f[2]=0.14; break;
    case GAL_COLOR_OLIVE:          f[0]=0.50; f[1]=0.50; f[2]=0.00; break;
    case GAL_COLOR_ORANGE:         f[0]=1.00; f[1]=0.65; f[2]=0.00; break;
    case GAL_COLOR_ORANGERED:      f[0]=1.00; f[1]=0.27; f[2]=0.00; break;
    case GAL_COLOR_ORCHID:         f[0]=0.85; f[1]=0.44; f[2]=0.84; break;
    case GAL_COLOR_PALEGOLDENROD:  f[0]=0.93; f[1]=0.91; f[2]=0.67; break;
    case GAL_COLOR_PALEGREEN:      f[0]=0.60; f[1]=0.98; f[2]=0.60; break;
    case GAL_COLOR_PALETURQUOISE:  f[0]=0.69; f[1]=0.93; f[2]=0.93; break;
    case GAL_COLOR_PALEVIOLETRED:  f[0]=0.86; f[1]=0.44; f[2]=0.58; break;
    case GAL_COLOR_PAPAYAWHIP:     f[0]=1.00; f[1]=0.94; f[2]=0.84; break;
    case GAL_COLOR_PEACHPUFF:      f[0]=1.00; f[1]=0.85; f[2]=0.73; break;
    case GAL_COLOR_PERU:           f[0]=0.80; f[1]=0.52; f[2]=0.25; break;
    case GAL_COLOR_PINK:           f[0]=1.00; f[1]=0.75; f[2]=0.80; break;
    case GAL_COLOR_PLUM:           f[0]=0.87; f[1]=0.63; f[2]=0.87; break;
    case GAL_COLOR_POWDERBLUE:     f[0]=0.69; f[1]=0.88; f[2]=0.90; break;
    case GAL_COLOR_PURPLE:         f[0]=0.50; f[1]=0.00; f[2]=0.50; break;
    case GAL_COLOR_RED:            f[0]=1.00; f[1]=0.00; f[2]=0.00; break;
    case GAL_COLOR_ROSYBROWN:      f[0]=0.74; f[1]=0.56; f[2]=0.56; break;
    case GAL_COLOR_ROYALBLUE:      f[0]=0.25; f[1]=0.41; f[2]=0.88; break;
    case GAL_COLOR_SADDLEBROWN:    f[0]=0.55; f[1]=0.27; f[2]=0.07; break;
    case GAL_COLOR_SALMON:         f[0]=0.98; f[1]=0.50; f[2]=0.45; break;
    case GAL_COLOR_SANDYBROWN:     f[0]=0.96; f[1]=0.64; f[2]=0.38; break;
    case GAL_COLOR_SEAGREEN:       f[0]=0.18; f[1]=0.55; f[2]=0.34; break;
    case GAL_COLOR_SEASHELL:       f[0]=1.00; f[1]=0.96; f[2]=0.93; break;
    case GAL_COLOR_SIENNA:         f[0]=0.63; f[1]=0.32; f[2]=0.18; break;
    case GAL_COLOR_SILVER:         f[0]=0.75; f[1]=0.75; f[2]=0.75; break;
    case GAL_COLOR_SKYBLUE:        f[0]=0.53; f[1]=0.81; f[2]=0.92; break;
    case GAL_COLOR_SLATEBLUE:      f[0]=0.42; f[1]=0.35; f[2]=0.80; break;
    case GAL_COLOR_SLATEGRAY:      f[0]=0.44; f[1]=0.50; f[2]=0.56; break;
    case GAL_COLOR_SNOW:           f[0]=1.00; f[1]=0.98; f[2]=0.98; break;
    case GAL_COLOR_SPRINGGREEN:    f[0]=0.00; f[1]=1.00; f[2]=0.50; break;
    case GAL_COLOR_STEELBLUE:      f[0]=0.27; f[1]=0.51; f[2]=0.71; break;
    case GAL_COLOR_TAN:            f[0]=0.82; f[1]=0.71; f[2]=0.55; break;
    case GAL_COLOR_TEAL:           f[0]=0.00; f[1]=0.50; f[2]=0.50; break;
    case GAL_COLOR_THISTLE:        f[0]=0.85; f[1]=0.75; f[2]=0.85; break;
    case GAL_COLOR_TOMATO:         f[0]=1.00; f[1]=0.39; f[2]=0.28; break;
    case GAL_COLOR_TURQUOISE:      f[0]=0.25; f[1]=0.88; f[2]=0.82; break;
    case GAL_COLOR_VIOLET:         f[0]=0.93; f[1]=0.51; f[2]=0.93; break;
    case GAL_COLOR_WHEAT:          f[0]=0.96; f[1]=0.87; f[2]=0.70; break;
    case GAL_COLOR_WHITE:          f[0]=1.00; f[1]=1.00; f[2]=1.00; break;
    case GAL_COLOR_WHITESMOKE:     f[0]=0.96; f[1]=0.96; f[2]=0.96; break;
    case GAL_COLOR_YELLOW:         f[0]=1.00; f[1]=1.00; f[2]=0.00; break;
    case GAL_COLOR_YELLOWGREEN:    f[0]=0.60; f[1]=0.80; f[2]=0.20; break;
    default:
      error(EXIT_FAILURE, 0, "%s: code '%u' doesn't correspond to "
            "any recognized color", __func__, color);
    }
}
