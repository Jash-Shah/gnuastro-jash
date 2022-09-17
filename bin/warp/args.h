/*********************************************************************
Warp - Warp images using projective mapping.
Warp is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
     Pedram Ashofteh Ardakani <pedramardakani@pm.me>
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

    /* Input. */
    {
      "hstartwcs",
      UI_KEY_HSTARTWCS,
      "INT",
      0,
      "Header keyword number to start reading WCS.",
      GAL_OPTIONS_GROUP_INPUT,
      &p->hstartwcs,
      GAL_TYPE_SIZE_T,
      GAL_OPTIONS_RANGE_GT_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "hendwcs",
      UI_KEY_HENDWCS,
      "INT",
      0,
      "Header keyword number to end reading WCS.",
      GAL_OPTIONS_GROUP_INPUT,
      &p->hendwcs,
      GAL_TYPE_SIZE_T,
      GAL_OPTIONS_RANGE_GT_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },





    /* Output. */
    {
      "keepwcs",
      UI_KEY_KEEPWCS,
      0,
      0,
      "Do not apply warp to input's WCS",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->keepwcs,
      GAL_OPTIONS_NO_ARG_TYPE,
      GAL_OPTIONS_RANGE_0_OR_1,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },
    {
      "coveredfrac",
      UI_KEY_COVEREDFRAC,
      "FLT",
      0,
      "Acceptable fraction of output pixel covered.",
      GAL_OPTIONS_GROUP_OUTPUT,
      &p->coveredfrac,
      GAL_TYPE_FLOAT64,
      GAL_OPTIONS_RANGE_GE_0_LE_1,
      GAL_OPTIONS_MANDATORY,
      GAL_OPTIONS_NOT_SET
    },





    {
      0, 0, 0, 0,
      "Align with WCS coordinates (correcting distortion, default mode)",
      UI_GROUP_ALIGN
    },
    {
      "center",
      UI_KEY_CENTER,
      "FLT,FLT",
      0,
      "Center coordinate of the output image in RA,DEC.",
      UI_GROUP_ALIGN,
      &p->wa.center,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      gal_options_parse_csv_float64
    },
    {
      "widthinpix",
      UI_KEY_WIDTHINPIX,
      "INT,INT",
      0,
      "Output image width and height in pixels.",
      UI_GROUP_ALIGN,
      &p->wa.widthinpix,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_GT_0_ODD,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      gal_options_parse_csv_float64
    },
    {
      "ctype",
      UI_KEY_CTYPE,
      "STR[,STR]",
      0,
      "FITS standard CTYPE value (e.g., 'RA---TAN').",
      UI_GROUP_ALIGN,
      &p->wa.ctype,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      gal_options_parse_csv_strings
    },
    {
      "cdelt",
      UI_KEY_CDELT,
      "FLT[,FLT]",
      0,
      "Pixel scale of output (usually degrees/pixel).",
      UI_GROUP_ALIGN,
      &p->cdelt,
      GAL_TYPE_STRING,
      GAL_OPTIONS_RANGE_GT_0,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      gal_options_parse_csv_float64
    },
    {
      "edgesampling",
      UI_KEY_EDGESAMPLING,
      "INT",
      0,
      "Number of extra samplings in pixel sides.",
      UI_GROUP_ALIGN,
      &p->wa.edgesampling,
      GAL_TYPE_SIZE_T,
      GAL_OPTIONS_RANGE_GE_0,
      GAL_OPTIONS_MANDATORY,
      GAL_OPTIONS_NOT_SET,
    },





    {
      0, 0, 0, 0,
      "Linear warps (must be called explicitly on command-line)",
      UI_GROUP_WARPS
    },
    {
      "rotate",
      UI_KEY_ROTATE,
      "FLT",
      0,
      "Rotate by the given angle in degrees.",
      UI_GROUP_WARPS,
      0,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "scale",
      UI_KEY_SCALE,
      "FLT[,FLT]",
      0,
      "Scale along the given axis(es).",
      UI_GROUP_WARPS,
      0,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "flip",
      UI_KEY_FLIP,
      "INT[,INT]",
      0,
      "Flip along the given axis(es).",
      UI_GROUP_WARPS,
      0,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "shear",
      UI_KEY_SHEAR,
      "FLT[,FLT]",
      0,
      "Shear along the given axis(es).",
      UI_GROUP_WARPS,
      0,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "translate",
      UI_KEY_TRANSLATE,
      "FLT[,FLT]",
      0,
      "Translate along the given axis(es).",
      UI_GROUP_WARPS,
      0,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "project",
      UI_KEY_PROJECT,
      "FLT[,FLT]",
      0,
      "Project along the given axis(es).",
      UI_GROUP_WARPS,
      0,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "matrix",
      UI_KEY_MATRIX,
      "STR",
      0,
      "Raw transformation matrix, highest priority.",
      UI_GROUP_WARPS,
      &p->matrix,
      GAL_TYPE_INVALID,
      GAL_OPTIONS_RANGE_ANY,
      GAL_OPTIONS_NOT_MANDATORY,
      GAL_OPTIONS_NOT_SET,
      ui_add_to_modular_warps_ll
    },
    {
      "centeroncorner",
      UI_KEY_CENTERONCORNER,
      0,
      0,
      "Center of coordinates on first pixel corner.",
      UI_GROUP_WARPS,
      &p->centeroncorner,
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
