/*********************************************************************
ConvertType - Convert between various types of files.
ConvertType is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2016-2022 Free Software Foundation, Inc.

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
#ifndef ARGS_H
#define ARGS_H





/* Array of acceptable options. */
struct argp_option program_options[] =
  {
    /* Input */
    {
      "globalhdu",
      UI_KEY_GLOBALHDU,
      "STR/INT",
      0,
      "Use this HDU for all inputs, ignore '--hdu'.",
      GAL_OPTIONS_GROUP_INPUT,
      &p->globalhdu,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },





    /* Output */
    {
      "quality",
      UI_KEY_QUALITY,
      "INT",
      0,
      "Quality of output JPEG image (1 to 100).",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->quality,
      GAL_TYPE_UINT8,
      GAL_OPTIONS_RANGE_GT_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "widthincm",
      UI_KEY_WIDTHINCM,
      "FLT",
      0,
      "Width in units of centimeters.",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->widthincm,
      GAL_TYPE_FLOAT32,
      GAL_OPTIONS_RANGE_GT_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "hex",
      UI_KEY_HEX,
      0,
      0,
      "Hexadecimal encoding in EPS. Default: ASCII85.",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->hex,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "colormap",
      UI_KEY_COLORMAP,
      "STR[,FLT]",
      0,
      "Color map when only a single channel is given.",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->colormap,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      gal_options_parse_csv_strings
    },
    {
      "rgbtohsv",
      UI_KEY_RGBTOHSV,
      0,
      0,
      "Convert RGB input into HSV (in FITS output)",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->rgbtohsv,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },






    {
      0, 0, 0, 0,
      "Flux:",
      UI_GROUP_FLUX
    },
    {
      "fluxlow",
      UI_KEY_FLUXLOW,
      "FLT",
      0,
      "Lower flux truncation value.",
      UI_GROUP_FLUX,
      &p->fluxlowstr,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "fluxhigh",
      UI_KEY_FLUXHIGH,
      "FLT",
      0,
      "Higher flux truncation value.",
      UI_GROUP_FLUX,
      &p->fluxhighstr,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "maxbyte",
      UI_KEY_MAXBYTE,
      "INT",
      0,
      "Maximum byte value for all color channels.",
      UI_GROUP_FLUX,
      &p->maxbyte,
      GAL_TYPE_UINT8,
      GAL_OPTIONS_RANGE_GE_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "forcemin",
      UI_KEY_FORCEMIN,
      0,
      0,
      "Force --fluxmin, even when smaller than minimum.",
      UI_GROUP_FLUX,
      &p->forcemin,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "forcemax",
      UI_KEY_FORCEMAX,
      0,
      0,
      "Force --fluxmax, even when larger than maximum.",
      UI_GROUP_FLUX,
      &p->forcemax,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "change",
      UI_KEY_CHANGE,
      "STR",
      0,
      "Change pixel values 'from_1:to_1,from_2:to_2'.",
      UI_GROUP_FLUX,
      &p->changestr,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "changeaftertrunc",
      UI_KEY_CHANGEAFTERTRUNC,
      0,
      0,
      "First truncate then change pixel values.",
      UI_GROUP_FLUX,
      &p->changeaftertrunc,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "invert",
      UI_KEY_INVERT,
      0,
      0,
      "Invert the values in JPEG and EPS/PDF.",
      UI_GROUP_FLUX,
      &p->invert,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },




    {
      0, 0, 0, 0,
      "Vector graphics (only for EPS or PDF outputs)",
      UI_GROUP_VECTOR
    },
    {
      "borderwidth",
      UI_KEY_BORDERWIDTH,
      "INT",
      0,
      "Border width in units of points (1/72 inch).",
      UI_GROUP_VECTOR,
      &p->borderwidth,
      GAL_TYPE_UINT32,
      GAL_OPTIONS_RANGE_GE_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "bordercolor",
      UI_GROUP_VECTOR,
      "STR",
      0,
      "Name of color to use for the border.",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->bordercolor,
      GAL_TYPE_UINT8,
      GAL_OPTIONS_RANGE_GE_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      gal_options_read_color
    },
    {
      "marks",
      UI_KEY_MARKS,
      "STR",
      0,
      "Name of mark information table.",
      UI_GROUP_VECTOR,
      &p->marksname,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markshdu",
      UI_KEY_MARKSHDU,
      "STR",
      0,
      "HDU in '--marks' (if its a FITS file).",
      UI_GROUP_VECTOR,
      &p->markshdu,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markcoords",
      UI_KEY_MARKCOORDS,
      "STR,STR",
      0,
      "Name or Number of columns with coordinates.",
      UI_GROUP_VECTOR,
      &p->markcoords,
      GAL_TYPE_STRLL,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
    },
    {
      "mode",
      UI_KEY_MODE,
      "STR",
      0,
      "Coordinate mode for marks ('wcs' or 'img').",
      UI_GROUP_VECTOR,
      &p->mode,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markshape",
      UI_KEY_MARKSHAPE,
      "STR",
      0,
      "Name or Number of col. with mark shapes: circle (1), "
      "plus (2), cross (3), ellipse (4), point(5), square (6) "
      "rectangle (7) and line (8).",
      UI_GROUP_VECTOR,
      &p->markshape,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markrotate",
      UI_KEY_MARKROTATE,
      "STR",
      0,
      "Name or Num. of col. with mark rotation.",
      UI_GROUP_VECTOR,
      &p->markrotate,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "marksize",
      UI_KEY_MARKSIZE,
      "STR[,STR]",
      0,
      "Name or Number of cols. with mark size(s).",
      UI_GROUP_VECTOR,
      &p->marksize,
      GAL_TYPE_STRLL,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "sizeinpix",
      UI_KEY_SIZEINPIX,
      0,
      0,
      "Size col. values are in pixels (in WCS-mode).",
      UI_GROUP_VECTOR,
      &p->sizeinpix,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "sizeinarcsec",
      UI_KEY_SIZEINARCSEC,
      0,
      0,
      "Size col. values are in arcsec (in WCS-mode).",
      UI_GROUP_VECTOR,
      &p->sizeinarcsec,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "sizeinarcmin",
      UI_KEY_SIZEINARCMIN,
      0,
      0,
      "Size col. values are in arcmin (in WCS-mode).",
      UI_GROUP_VECTOR,
      &p->sizeinarcmin,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "marklinewidth",
      UI_KEY_MARKLINEWIDTH,
      "STR",
      0,
      "Name or Number of col. with line width.",
      UI_GROUP_VECTOR,
      &p->marklinewidth,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markcolor",
      UI_KEY_MARKCOLOR,
      "STR",
      0,
      "Name or Number of col. with mark color. ",
      UI_GROUP_VECTOR,
      &p->markcolor,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "listcolors",
      UI_KEY_LISTCOLORS,
      0,
      0,
      "List names and RGB info of all colors.",
      UI_GROUP_VECTOR,
      &p->listcolors,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "marktext",
      UI_KEY_MARKTEXT,
      "STR",
      0,
      "Name or Num. of col. with mark text.",
      UI_GROUP_VECTOR,
      &p->marktext,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "marktextprecision",
      UI_KEY_MARKTEXTPRECISION,
      "INT",
      0,
      "Number decimals when text is float column.",
      UI_GROUP_VECTOR,
      &p->marktextprecision,
      GAL_TYPE_UINT8,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markfont",
      UI_KEY_MARKFONT,
      "STR",
      0,
      "Name or Num. of col. with mark font name.",
      UI_GROUP_VECTOR,
      &p->markfont,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "markfontsize",
      UI_KEY_MARKFONTSIZE,
      "STR",
      0,
      "Name or Num. of col. with mark font size.",
      UI_GROUP_VECTOR,
      &p->markfontsize,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "showfonts",
      UI_KEY_SHOWFONTS,
      0,
      0,
      "Show all fonts in a PDF.",
      UI_GROUP_VECTOR,
      &p->showfonts,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "listfonts",
      UI_KEY_LISTFONTS,
      0,
      0,
      "List names of available fonts.",
      UI_GROUP_VECTOR,
      &p->listfonts,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },

    {0}
  };





/* Define the child argp structure. */
struct argp
gal_options_common_child = {gal_commonopts_options,
                            gal_options_common_argp_parse,
                            NULL, NULL, NULL, NULL, NULL};

/* Use the child argp structure in list of children (only one for now). */
struct argp_child
children[]=
{
  {&gal_options_common_child, 0, NULL, 0},
  {0, 0, 0, 0}
};

/* Set all the necessary argp parameters. */
struct argp
thisargp = {program_options, parse_opt, args_doc, doc, children, NULL, NULL};
#endif
