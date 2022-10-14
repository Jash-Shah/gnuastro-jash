/*********************************************************************
Warp - Warp images using projective mapping.
Warp is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
     Pedram Ashofteh-Ardakani <pedramardakani@pm.me>
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
#include <config.h>

#include <argp.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

#include <gnuastro/wcs.h>
#include <gnuastro/warp.h>
#include <gnuastro/fits.h>
#include <gnuastro/array.h>
#include <gnuastro/table.h>
#include <gnuastro/threads.h>
#include <gnuastro/pointer.h>

#include <gnuastro-internal/timing.h>
#include <gnuastro-internal/options.h>
#include <gnuastro-internal/checkset.h>
#include <gnuastro-internal/fixedstringmacros.h>

#include "main.h"

#include "ui.h"
#include "authors-cite.h"





/**************************************************************/
/*********      Argp necessary global entities     ************/
/**************************************************************/
/* Definition parameters for the Argp: */
const char *
argp_program_version = PROGRAM_STRING "\n"
                       GAL_STRINGS_COPYRIGHT
                       "\n\nWritten/developed by "PROGRAM_AUTHORS;

const char *
argp_program_bug_address = PACKAGE_BUGREPORT;

static char
args_doc[] = "ASTRdata";

const char
doc[] = GAL_STRINGS_TOP_HELP_INFO PROGRAM_NAME" will resample the pixel "
  "grid of an input image. By default (if no special linear warping is "
  "requested), it will align the image to the WCS coordinates in any"
  "and remove any possible distortion. Linear warping (like '--rotate' "
  "or '--scale') should be explicitly requested with the options under "
  "the \"Linear warps\" group below. \n"
  GAL_STRINGS_MORE_HELP_INFO
  /* After the list of options: */
  "\v"
  PACKAGE_NAME" home page: "PACKAGE_URL;




















/**************************************************************/
/*********    Initialize & Parse command-line    **************/
/**************************************************************/
static void
ui_initialize_options(struct warpparams *p,
                      struct argp_option *program_options,
                      struct argp_option *gal_commonopts_options)
{
  size_t i;
  struct gal_options_common_params *cp=&p->cp;


  /* Set the necessary common parameters structure. */
  cp->program_struct     = p;
  cp->program_name       = PROGRAM_NAME;
  cp->program_exec       = PROGRAM_EXEC;
  cp->program_bibtex     = PROGRAM_BIBTEX;
  cp->program_authors    = PROGRAM_AUTHORS;
  cp->poptions           = program_options;
  cp->numthreads         = gal_threads_number();
  cp->coptions           = gal_commonopts_options;

  /* Program specific initializations. */
  p->wa.edgesampling     = GAL_BLANK_SIZE_T;

  /* Set the mandatory common options. */
  for(i=0; !gal_options_is_last(&cp->coptions[i]); ++i)
    {
      /* Select individually. */
      switch(cp->coptions[i].key)
        {
        case GAL_OPTIONS_KEY_MINMAPSIZE:
          cp->coptions[i].mandatory=GAL_OPTIONS_MANDATORY;
          break;

        case GAL_OPTIONS_KEY_SEARCHIN:
        case GAL_OPTIONS_KEY_IGNORECASE:
        case GAL_OPTIONS_KEY_TABLEFORMAT:
        case GAL_OPTIONS_KEY_STDINTIMEOUT:
          cp->coptions[i].flags=OPTION_HIDDEN;
          break;
        }

      /* Select by group. */
      switch(cp->coptions[i].group)
        {
        case GAL_OPTIONS_GROUP_TESSELLATION:
          cp->coptions[i].doc=NULL; /* Necessary to remove title. */
          cp->coptions[i].flags=OPTION_HIDDEN;
          break;
        }
    }
}





/* Parse a single option: */
error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
  struct warpparams *p = state->input;

  /* Pass 'gal_options_common_params' into the child parser.  */
  state->child_inputs[0] = &p->cp;

  /* In case the user incorrectly uses the equal sign (for example
     with a short format or with space in the long format, then 'arg'
     start with (if the short version was called) or be (if the long
     version was called with a space) the equal sign. So, here we
     check if the first character of arg is the equal sign, then the
     user is warned and the program is stopped: */
  if(arg && arg[0]=='=')
    argp_error(state, "incorrect use of the equal sign ('='). For short "
               "options, '=' should not be used and for long options, "
               "there should be no space between the option, equal sign "
               "and value");

  /* Set the key to this option. */
  switch(key)
    {
    /* Read the non-option tokens (arguments): */
    case ARGP_KEY_ARG:
      /* The user may give a shell variable that is empty! In that case
         'arg' will be an empty string! We don't want to account for such
         cases (and give a clear error that no input has been given). */
      if(p->inputname)
        argp_error(state, "only one argument (input file) should be given");
      else
        if(arg[0]!='\0') p->inputname=arg;
      break;

    /* This is an option, set its value. */
    default:
      return gal_options_set_from_key(key, arg, p->cp.poptions, &p->cp);
    }

  return 0;
}



















/**************************************************************/
/**********      Modular matrix linked list       *************/
/**************************************************************/
/* Save the codes of the user's desired modular warpings into the linked
   list. Because the types of these options are 'GAL_TYPE_INVALID', this
   function will not be called when printing the full list of parameters
   and their values. */
static void *
ui_add_to_modular_warps_ll(struct argp_option *option, char *arg,
                           char *filename, size_t lineno, void *params)
{
  size_t i;
  double tmp;
  gal_data_t *new;
  struct warpparams *p=(struct warpparams *)params;

  /* When an argument is necessary, make sure we actually have a string to
     parse. Also note that if an argument is necessary, but none is given
     Argp will automatically abort the program with an informative
     error. */
  if(arg && *arg=='\0')
    error(EXIT_FAILURE, 0, "empty string given to '--%s'", option->name);

  /* Parse the (possible) arguments. */
  new=gal_options_parse_list_of_numbers(arg, filename, lineno);


  /* If this was a matrix, then put it in the matrix element of the main
     data structure. Otherwise, add the list of given values to the modular
     warpings list. */
  if(option->key==UI_KEY_MATRIX)
    {
      /* Some sanity checks. */
      if(p->matrix)
        error_at_line(EXIT_FAILURE, 0, filename, lineno, "only one matrix "
                      "may be given, you can use multiple modular warpings");
      if(new->size!=4 && new->size!=9)
        error_at_line(EXIT_FAILURE, 0, filename, lineno, "only a 4 or 9 "
                      "element 'matrix' is currently acceptable. '%s' has "
                      "%zu elements", arg, new->size);

      /* Keep the matrix in the main structure. */
      p->matrix=new;
    }
  else
    {
      /* No more than two numbers should be given for the modular
         warpings. */
      if(new->size>2)
        error_at_line(EXIT_FAILURE, 0, filename, lineno, "%zu numbers "
                      "given to the '%s' option. Modular warpings can "
                      "accept 2 numbers at the most currently (for 2D "
                      "datasets)", new->size, option->name);

      /* Some modular-warp specific sanity checks: rotate only needs one
         number, and flip's values should only be 0 and 1. */
      if(option->key==UI_KEY_ROTATE)
        {
          if(new->size!=1)
            error_at_line(EXIT_FAILURE, 0, filename, lineno, "the 'rotate' "
                      "option only takes one value (the angle of rotation). "
                      "You have given: '%s'", arg);
        }
      else if (option->key==UI_KEY_FLIP)
        {
          for(i=0;i<new->size;++i)
            {
              tmp=((double *)(new->array))[i];
              if(tmp!=0.0f && tmp!=1.0f)
                error_at_line(EXIT_FAILURE, 0, filename, lineno, "'flip' "
                              "only takes values of '1' and '0'. You have "
                              "given '%s'", arg);
            }
        }

      /* Keep the final value. */
      new->status=option->key;
      new->next=p->modularll;
      p->modularll=new;
    }
  return NULL;
}




















/**************************************************************/
/***************       Sanity Check         *******************/
/**************************************************************/
static void
ui_check_gridfile(struct warpparams *p)
{
  int nwcs;
  size_t ndim, *dsize=NULL;
  gal_warp_wcsalign_t *wa=&p->wa;

  /* Make sure that the file given to '--gridfile' is a recognized FITS
     file. */
  if(gal_fits_file_recognized(p->gridfile)==0)
    error(EXIT_FAILURE, 0, "'%s' (given to '--gridfile') must be "
          "in FITS format with a recognizable FITS format suffix",
          p->gridfile);
  if(p->gridhdu==NULL)
    error(EXIT_FAILURE, 0, "%s no HDU/extension specified (file given "
          "to '--gridfile')! Please use '--gridhdu' to specify a "
          "HDU/extension to read from", p->gridfile);

  /* Read the WCS and save to a standard PC convention WCS struct. */
  wa->twcs=gal_wcs_read(p->gridfile, p->gridhdu,
                        GAL_WCS_LINEAR_MATRIX_PC, 0, 0, &nwcs);
  if(wa->twcs==NULL)
    error(EXIT_FAILURE, 0, "%s (hdu %s): no readable WCS structure",
          p->gridfile, p->gridhdu);

  /* Correct the WCS dimensions if necessary. */
  dsize=gal_fits_img_info_dim(p->gridfile, p->gridhdu, &ndim);
  ndim=gal_dimension_remove_extra(ndim, dsize, wa->twcs);
  if(ndim!=2)
    error(EXIT_FAILURE, 0, "%s (hdu %s): the target WCS must "
          "contain 2 dimensions, but warp detected %zu dimensions",
          p->gridfile, p->gridhdu, ndim);

  /* If '--width' has already been given, inform the user that it will be
     ignored (because '--gridfile' has been given). */
  if(p->width)
    {
      error(EXIT_SUCCESS, 0, "WARNING: '--width' will be ignored "
            "because '--gridfile' takes precedence");
      gal_data_free(p->width);
    }

  /* We don't need to free the input 'dsize' array since it will be freed
     by gal_data_free at the clean up stage. */
  wa->widthinpix=gal_data_alloc(dsize, GAL_TYPE_SIZE_T, 1, &ndim, NULL, 1,
                                p->cp.minmapsize, p->cp.quietmmap, NULL, NULL,
                                NULL);

  /* If any of the non-default WCS-defining options are given, free them
     and print a warning. We don't want to print any warning for the
     default one ('--ctype') because most users may not set it. */
  if(wa->ctype) { gal_data_free(wa->ctype); wa->ctype=NULL; }
  if(wa->cdelt)
    {
      error(EXIT_SUCCESS, 0, "WARNING: '--cdelt' will be ignored "
            "because '--gridfile' takes precedence");
      gal_data_free(wa->cdelt);
      wa->cdelt=NULL;
    }
  if(wa->center)
    {
      error(EXIT_SUCCESS, 0, "WARNING: '--center' will be ignored "
            "because '--gridfile' takes precedence");
      gal_data_free(wa->center);
      wa->center=NULL;
    }
}





static void
ui_check_wcsalign_cdelt(struct warpparams *p)
{
  size_t two=2, i;
  gal_data_t *olddata=NULL;
  double *tmp=NULL, *cdelt=NULL;
  gal_warp_wcsalign_t *wa=&p->wa;

  /* '--cdelt' is given. */
  if(wa->cdelt)
    {
      /* CDELT is given, make sure there are no more than two values */
      if(wa->cdelt->size > 2)
        error(EXIT_FAILURE, 0, "%zu values given to '--cdelt', "
              "however this option takes no more than 2 values",
              wa->cdelt->size);

      /* If only one value given to CDELT, use it for both dimensions. */
      if(wa->cdelt->size == 1)
        {
          olddata=wa->cdelt; tmp=olddata->array;
          wa->cdelt=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1,
                                   &two, NULL, 0,
                                   p->cp.minmapsize,
                                   p->cp.quietmmap, NULL, NULL,
                                   NULL);
          cdelt=p->wa.cdelt->array;
          cdelt[0]=cdelt[1]=tmp[0];
          gal_data_free(olddata);
        }

      /* Check if the CDELT is in a reasonable range of degrees (in case
         '--widthinpix' wasn't given). */
      if(p->widthinpix==0)
        {
          cdelt=wa->cdelt->array;
          for(i=wa->cdelt->size; i--;)
            if(cdelt[i]>0.01)
              error(EXIT_SUCCESS, 0, "WARNING: CDELT on dimension %zu has "
                    "the unusual value of %f degrees. If you meant to define "
                    "CDELT in arcseconds please use: '--cdelt=%g/3600'", i,
                    cdelt[i], cdelt[i]);
        }
    }

  /* '--cdelt' not given. */
  else
    {
      /* CDELT is not given, try to deduce from WCS */
      cdelt=gal_wcs_pixel_scale(p->input->wcs);
      if(!cdelt)
        error(EXIT_FAILURE, 0, "%s (hdu %s): the pixel scale couldn't "
              "be deduced from the WCS.", p->inputname, p->cp.hdu);

      /* Set CDELT to the maximum value of the dimensions. */
      cdelt[0] = ( cdelt[0] > cdelt[1] ? cdelt[0] : cdelt[1] );
      cdelt[1] = cdelt[0];
      wa->cdelt=gal_data_alloc(cdelt, GAL_TYPE_FLOAT64, 1, &two, NULL, 0,
                               p->cp.minmapsize, p->cp.quietmmap, NULL, NULL,
                               NULL);
    }

}





static void
ui_check_wcsalign_width(struct warpparams *p)
{
  /* Low-level variable (used in other variable definitions). */
  gal_warp_wcsalign_t *wa=&p->wa;

  /* High-level variables. */
  gal_data_t *tmpw;
  size_t i, two=2, stemp, *sarray=NULL;
  double *cdelt=wa->cdelt->array, *darray, *tdarray;

  /* Make sure only two values are given. */
  if( p->width->size > 2 )
    error(EXIT_FAILURE, 0, "%zu value(s) given to '--width', however "
          "this option takes 1 or two values on a 2D image: the output "
          "image width and height in WCS units (if you want to enter "
          "the width in pixels, please also call '--widthinpix'). If a "
          "single value is given the size will be a square", p->width->size);

  /* If a single value is given, use it for both dimensions. */
  if(p->width->size==1)
    {
      /* Allocate a new one and add the values. */
      tmpw=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &two, NULL, 0,
                          p->cp.minmapsize, p->cp.quietmmap,
                          NULL, NULL, NULL);

      /* Put the values in. */
      tdarray=tmpw->array;
      darray=p->width->array;
      tdarray[0]=tdarray[1]=darray[0];

      /* Clean up and set the new pointer. */
      gal_data_free(p->width);
      p->width=tmpw;
    }


  /* When '--widthinpix' is called we can directly use the values. */
  darray=p->width->array;
  if(p->widthinpix)
    {
      /* Make sure they are integers. */
      if( darray[0]!=ceil(darray[0]) || darray[1]!=ceil(darray[1]) )
        error(EXIT_FAILURE, 0, "value to '--width' must be integers, "
              "but they are: %g, %g", darray[0], darray[1]);
      p->width=gal_data_copy_to_new_type(p->width, GAL_TYPE_SIZE_T);
    }
  else
    {
      /* Allocate the size_t array to keep the final values. */
      tmpw=gal_data_alloc(NULL, GAL_TYPE_SIZE_T, 1, &two, NULL, 0,
                          p->cp.minmapsize, p->cp.quietmmap,
                          NULL, NULL, NULL);

      /* Convert the given values to pixels using the pixel scale
         information, and make sure they are reasonable. */
      sarray=tmpw->array;
      for(i=0;i<p->width->size;++i)
        {
          sarray[i] = darray[i]/cdelt[i];
          if(sarray[i]>GAL_OPTIONS_WIDTH_TOO_LARGE_SIZE)
            gal_options_width_too_large(darray[i], i+1, sarray[i],
                                        cdelt[i]);
        }

      /* Free the old dataset and use the new one. */
      gal_data_free(p->width);
      p->width=tmpw;
    }

  /* Image size must be ODD */
  sarray=p->width->array;
  if( sarray[0]%2==0 || sarray[1]%2==0 )
    {
      /* Let the user know that we are updating the output size (only
         relevant if the user directly requested pixel coordinates). */
      if(p->widthinpix)
        error(EXIT_SUCCESS, 0, "WARNING: '--widthinpix' must be odd: "
              "updating %zux%zu to %zux%zu", sarray[0], sarray[1],
              sarray[0]%2 ? sarray[0] : sarray[0]+1,
              sarray[1]%2 ? sarray[1] : sarray[1]+1);

      /* Keep the final values. */
      if(sarray[0]%2==0) sarray[0]+=1;
      if(sarray[1]%2==0) sarray[1]+=1;
    }

  /* To keep the later steps consistent with the C ordering of
     dimensions, we'll swap the fast and slow axis from FITS (which has
     the same convention as FORTRAN), because the user sees a FITS
     file. In an aligned image, sarray[0] is the horizontal axis and
     sarray[1] the vertical one.

     NAXIS1 -> dsize[1] -> horizontal axis
     NAXIS2 -> dsize[0] -> vertical axis        */
  stemp=sarray[0]; sarray[0]=sarray[1]; sarray[1]=stemp;

  /* Put the final widthinpix dataset into the warpalign structure. */
  p->wa.widthinpix=p->width;
  p->width=NULL;
}





static void *
ui_check_options_and_arguments_wcsalign(struct warpparams *p)
{
  size_t two=2, indim=0;
  gal_warp_wcsalign_t *wa=&p->wa;
  double *icenter=NULL, *iwidth=NULL, *imin=NULL, *imax=NULL, *tmp=NULL;

  /* Copy necessary parameters for the nonlinear warp (independent of
     having a WCS or not). */
  wa->input=p->input;
  wa->coveredfrac=p->coveredfrac;
  wa->numthreads=p->cp.numthreads;

  /* If using a WCS file for the target grid (with '--gridfile' and
     '--gridhdu' options) check the file and ignore the other given
     parameters. */
  if(p->gridfile)
    {
      ui_check_gridfile(p);
      return NULL;
    }

  /* Sanity check user's possibly given '--center'. */
  if(wa->center)
    {
      if(wa->center->size !=2)
        error(EXIT_FAILURE, 0, "%zu value(s) given to '--center', "
              "however this option takes exactly 2 values to specify "
              "the output image center", wa->center->size);

      tmp=wa->center->array;
      if(tmp[0] < 0 || tmp[0] > 360 || tmp[1] < -90 || tmp[1] > 90 )
        error(EXIT_FAILURE, 0, "the first value '--center' should be "
              "between 0 and 360 (inclusive, because it is the RA) "
              "and the second value should be between -90 and 90 "
              "(inclusive, because it is the Dec), however the given "
              "values are: %g and %g", tmp[0], tmp[1]);
    }
  else   /* --center not given, use the input image to define it. */
    {
      /* Get sky coverage information about the input image. Note that
         this allocates the pointers that have to be freed later. */
      if( gal_wcs_coverage(p->inputname, p->cp.hdu, &indim, &icenter,
                           &iwidth, &imin, &imax)==0 )
        error(EXIT_FAILURE, 0, "%s (hdu %s): is not usable for finding "
              "sky coverage", p->inputname, p->cp.hdu);


      /* Store the center array and clean up the rest. */
      wa->center=gal_data_alloc(icenter, GAL_TYPE_FLOAT64, 1,
                                &two, NULL, 0, p->cp.minmapsize,
                                p->cp.quietmmap, NULL, NULL,
                                NULL);

      free(imin);
      free(imax);
      free(iwidth);
    }

  /* Prepare the output pixel scale and width. Note that the pixel scale
     should be checked before the width because the width may need the
     pixel scale. */
  ui_check_wcsalign_cdelt(p);
  if(p->width) ui_check_wcsalign_width(p);

  /* Check CTYPE */
  if(!wa->ctype)
    error(EXIT_FAILURE, 0, "no output projection CTYPE specified, "
          "you can use the '--ctype' option and give it a comma "
          "separated CTYPE value recognized by the WCSLIB (e.g. "
          "--ctype=RA---TAN,DEC--TAN)");
  if(wa->ctype->size != p->input->ndim)
    error(EXIT_FAILURE, 0, "%zu value(s) given to '--ctype', but it "
          "takes exactly 2 values", wa->ctype->size);
  return NULL;
}





static void
ui_check_options_and_arguments(struct warpparams *p)
{
  /* Read the input.*/
  if(p->inputname==NULL)
    error(EXIT_FAILURE, 0, "no input file is specified");

  /* Make sure a HDU is given. */
  if( gal_fits_file_recognized(p->inputname) && p->cp.hdu==NULL )
    error(EXIT_FAILURE, 0, "no HDU specified, you can use the '--hdu' "
          "('-h') option and give it the HDU number (starting from "
          "zero), or extension name (generally, anything acceptable "
          "by CFITSIO)");

  /* Make sure mandatory options are provided. */
  if(p->modularll==NULL && p->matrix==NULL)
    {
      p->wcsalign=1;
      if(p->wa.edgesampling==GAL_BLANK_SIZE_T)
        error(EXIT_FAILURE, 0, "no '--edgesampling' provided");
    }

  /* Read the input image as double type and its WCS structure. */
  p->input=gal_array_read_one_ch_to_type(p->inputname, p->cp.hdu,
                                         NULL, GAL_TYPE_FLOAT64,
                                         p->cp.minmapsize,
                                         p->cp.quietmmap);

  /* Read the WCS and remove one-element wide dimension(s). */
  p->input->wcs=gal_wcs_read(p->inputname, p->cp.hdu,
                             p->cp.wcslinearmatrix, p->hstartwcs,
                             p->hendwcs, &p->input->nwcs);
  p->input->ndim=gal_dimension_remove_extra(p->input->ndim,
                                            p->input->dsize,
                                            p->input->wcs);

  /* Currently Warp only works on 2D images. */
  if(p->input->ndim!=2)
    error(EXIT_FAILURE, 0, "input has %zu dimensions but Warp currently "
          "only works on 2D datasets (images).\n\n"
          "We do plan to add 3D functionality (see "
          "https://savannah.gnu.org/task/?15729), so please get in "
          "touch if you need it (any further interest, support or help "
          "would be useful)", p->input->ndim);

  /* Get basic WCS information. */
  if(p->input->wcs)
    p->inwcsmatrix=gal_wcs_warp_matrix(p->input->wcs);

  /* Do all the distortion correction sanity-checks.*/
  if(p->wcsalign)
    ui_check_options_and_arguments_wcsalign(p);
}




















/**************************************************************/
/***************     Matrix preparations     ******************/
/**************************************************************/
static void
ui_error_no_warps()
{
  error(EXIT_FAILURE, 0, "no warping specified, you can either use the "
        "'--matrix' option for any low-level warp, or specify multipole "
        "modular warpings with options like '--rotate', '--scale' and etc. "
        "You can see the full list with the '--help' option");
}





/* This function is mainly for easy checking/debugging. */
static void
ui_matrix_print(double *matrix)
{
  printf("%-10.3f%-10.3f%-10.3f\n", matrix[0], matrix[1], matrix[2]);
  printf("%-10.3f%-10.3f%-10.3f\n", matrix[3], matrix[4], matrix[5]);
  printf("%-10.3f%-10.3f%-10.3f\n", matrix[6], matrix[7], matrix[8]);
}





static void
ui_matrix_prepare_raw(struct warpparams *p)
{
  size_t *dsize;
  double *in=p->matrix->array, *final;

  /* If the matrix was 2D, then convert it to 3D. Note that we done a size
     check when reading the matrix, so at this point, it either has 9
     elements, or 4. */
  if(p->matrix->size==4)
    {
      /* Allocate the final matrix. */
      final=gal_pointer_allocate(GAL_TYPE_FLOAT64, 9, 0, __func__, "final");

      /* Fill in the final 3x3 matrix from the 2x2 matrix. */
      final[0]=in[0];    final[1]=in[1];   final[2]=0.0f;
      final[3]=in[2];    final[4]=in[3];   final[5]=0.0f;
      final[6]=0.0f;     final[7]=0.0f;    final[8]=1.0f;

      /* Free the old matrix array and put in the new one. */
      free(p->matrix->array);
      p->matrix->size=9;
      p->matrix->array=final;
    }

  /* Correct the dimensional information, because the matrix was read as a
     single dimensional list of numbers. */
  free(p->matrix->dsize);
  dsize=p->matrix->dsize=gal_pointer_allocate(GAL_TYPE_SIZE_T, 2, 0,
                                               __func__, "dsize");
  dsize[0]=dsize[1]=3;
  p->matrix->ndim=2;
}





static void
ui_matrix_inplacw_multiply(double *in, double *with)
{
  /* 'tin' will keep the values of the input array because we want to
     write the multiplication result in the input array. */
  double tin[9]={in[0],in[1],in[2],in[3],in[4],in[5],in[6],in[7],in[8]};

  /* For easy checking, here are the matrix/memory layouts:
          tin[0] tin[1] tin[2]     with[0] with[1] with[2]
          tin[3] tin[4] tin[5]  *  with[3] with[4] with[5]
          tin[6] tin[7] tin[8]     with[6] with[7] with[8]   */
  in[0] = tin[0]*with[0] + tin[1]*with[3] + tin[2]*with[6];
  in[1] = tin[0]*with[1] + tin[1]*with[4] + tin[2]*with[7];
  in[2] = tin[0]*with[2] + tin[1]*with[5] + tin[2]*with[8];

  in[3] = tin[3]*with[0] + tin[4]*with[3] + tin[5]*with[6];
  in[4] = tin[3]*with[1] + tin[4]*with[4] + tin[5]*with[7];
  in[5] = tin[3]*with[2] + tin[4]*with[5] + tin[5]*with[8];

  in[6] = tin[6]*with[0] + tin[7]*with[3] + tin[8]*with[6];
  in[7] = tin[6]*with[1] + tin[7]*with[4] + tin[8]*with[7];
  in[8] = tin[6]*with[2] + tin[7]*with[5] + tin[8]*with[8];
}






static void
ui_matrix_from_modular(struct warpparams *p)
{
  gal_data_t *pop;
  size_t dsize[]={3,3};
  double s, c, v1, v2, *final, module[9]={1,0,0,  0,1,0,  0,0,1};

  /* Reverse the list of modular warpings to be in the same order as the
     user specified.*/
  gal_list_data_reverse(&p->modularll);

  /* Allocate space for the final matrix. */
  p->matrix=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 2, dsize, NULL, 0,
                           p->cp.minmapsize, p->cp.quietmmap,
                           NULL, NULL, NULL);
  final=p->matrix->array;

  /* Fill in the final matrix to start with. */
  final[0]=1.0f;     final[1]=0.0f;     final[2]=0.0f;
  final[3]=0.0f;     final[4]=1.0f;     final[5]=0.0f;
  final[6]=0.0f;     final[7]=0.0f;     final[8]=1.0f;

  /* Apply all modular warps. */
  while(p->modularll)
    {
      /* Pop the top element. */
      pop=gal_list_data_pop(&p->modularll);

      /* Set the (possibly) two values given for this warp. */
      v1 = pop->ndim   ? ((double *)(pop->array))[0] : 0.0f;
      v2 = pop->size>1 ? ((double *)(pop->array))[1] : v1;

      /* Depending on the type of the modular warp do it. Recall that the
         code for the warp, was stored in the 'status' element of the data
         structure.*/
      switch(pop->status)
        {
        case UI_KEY_ROTATE:
          s = sin( v1 * M_PI / 180 );
          c = cos( v1 * M_PI / 180 );
          module[0]=c;          module[1]=-1.0f*s;  module[2]=0.0f;
          module[3]=s;          module[4]=c;        module[5]=0.0f;
          module[6]=0.0f;       module[7]=0.0f;     module[8]=1.0f;
          break;

        case UI_KEY_SCALE:
          module[0]=v1;         module[1]=0.0f;   module[2]=0.0f;
          module[3]=0.0f;       module[4]=v2;     module[5]=0.0f;
          module[6]=0.0f;       module[7]=0.0f;   module[8]=1.0f;
          break;

        case UI_KEY_FLIP:
          if      ( v1==1.0f && v2==0.0f )
            {
              module[0]=1.0f;   module[1]=0.0f;
              module[3]=0.0f;   module[4]=-1.0f;
            }
          else if ( v1==0.0f && v2==1.0f )
            {
              module[0]=-1.0f;  module[1]=0.0f;
              module[3]=0.0f;   module[4]=1.0f;
            }
          else if ( v1==1.0f && v2==1.0f )
            {
              module[0]=-1.0f;  module[1]=0.0f;
              module[3]=0.0f;   module[4]=-1.0f;
            }
          else                  /* When both are zero, just in case! */
            {
              module[0]=1.0f;   module[1]=0.0f;
              module[3]=0.0f;   module[4]=1.0f;
            }
                                                  module[2]=0.0f;
                                                  module[5]=0.0f;
          module[6]=0.0f;       module[7]=0.0f;   module[8]=1.0f;
          break;

        case UI_KEY_SHEAR:
          module[0]=1.0f;       module[1]=v1;     module[2]=0.0f;
          module[3]=v2;         module[4]=1.0f;   module[5]=0.0f;
          module[6]=0.0f;       module[7]=0.0f;   module[8]=1.0f;
          break;

        case UI_KEY_TRANSLATE:
          /* Translate cannot be called with '--centeroncorner'. */
          if(p->centeroncorner)
            error(EXIT_FAILURE, 0, "'--translate' and '--centeroncorner' "
                  "(which is a type of translation) cannot be called "
                  "together. To achieve the effect of --centeroncorner, "
                  "start the warp steps with a translation of 0.5 to move "
                  "the coordinate center to the corner of a pixel in each "
                  "dimension");

          /* Build the translation matrix. */
          module[0]=1.0f;       module[1]=0.0f;     module[2]=v1;
          module[3]=0.0f;       module[4]=1.0f;     module[5]=v2;
          module[6]=0.0f;       module[7]=0.0f;     module[8]=1.0f;
          break;

        case UI_KEY_PROJECT:
          module[0]=1.0f;       module[1]=0.0f;     module[2]=0.0f;
          module[3]=0.0f;       module[4]=1.0f;     module[5]=0.0f;
          module[6]=v1;         module[7]=v2;       module[8]=1.0f;
          break;

        default:
          error(EXIT_FAILURE, 0, "a bug! the code %d is not recognized as "
                "a valid modular warp in 'ui_matrix_from_modular', this is "
                "not your fault, something in the programming has gone "
                "wrong. Please contact us at %s so we can correct it",
                pop->status, PACKAGE_BUGREPORT);
        }

      /* Multiply the main matrix with this modular matrix. */
      ui_matrix_inplacw_multiply(p->matrix->array, module);

      /* Clean up. */
      gal_data_free(pop);
    }
}





static void
ui_matrix_center_on_corner(struct warpparams *p)
{
  double *b, *d, *df;
  double before[9]={1,0,0.5,0,1,0.5,0,0,1};
  double after[9]={1,0,-0.5,0,1,-0.5,0,0,1};

  /* Shift the matrix by +0.5 so the coordinate center lies at the bottom
     left corner of the first pixel. Note that the updated values are
     written into the first argument of the function.*/
  ui_matrix_inplacw_multiply(before, p->matrix->array);

  /* Translate them back into the proper FITS center. */
  ui_matrix_inplacw_multiply(before, after);

  /* The final matrix is in 'before', so put its values into the output
     matrix. */
  b = before;
  df = (d=p->matrix->array) + p->matrix->size;
  do *d=*b++; while(++d<df);
}





static void
ui_matrix_finalize(struct warpparams *p)
{
  double *d, *df, *inv;

  /* If a matrix string is not given, the use the modular warpings. */
  if(p->matrix)
    ui_matrix_prepare_raw(p);
  else if (p->modularll)
    ui_matrix_from_modular(p);
  else
    ui_error_no_warps();

  /* If the user has asked for it, set the coordinate center on the corner
     of the first pixel. */
  if(p->centeroncorner) ui_matrix_center_on_corner(p);

  /* Check if there are any non-normal numbers in the matrix: */
  df=(d=p->matrix->array)+p->matrix->size;
  do
    if(!isfinite(*d++))
      {
        ui_matrix_print(p->matrix->array);
        error(EXIT_FAILURE, 0, "%f is not a 'normal' number in the "
              "input matrix shown above", *(d-1));
      }
  while(d<df);

  /* Check if the determinant is not zero: */
  d=p->matrix->array;
  if( d[0]*d[4]*d[8] + d[1]*d[5]*d[6] + d[2]*d[3]*d[7]
      - d[2]*d[4]*d[6] - d[1]*d[3]*d[8] - d[0]*d[5]*d[7] == 0 )
    error(EXIT_FAILURE, 0, "the determinant of the given matrix "
          "is zero");

  /* Not yet implemented: Check if the transformation is spatially
     invariant, in other words, if it differs between differet regions of
     the output. If it doesn't we can use this information for a more
     efficient processing. */

   /* Make the inverse matrix: */
  inv=p->inverse=gal_pointer_allocate(GAL_TYPE_FLOAT64, 9, 0, __func__,
                                      "p->inverse");
  inv[0] = d[4]*d[8] - d[5]*d[7];
  inv[1] = d[2]*d[7] - d[1]*d[8];
  inv[2] = d[1]*d[5] - d[2]*d[4];
  inv[3] = d[5]*d[6] - d[3]*d[8];
  inv[4] = d[0]*d[8] - d[2]*d[6];
  inv[5] = d[2]*d[3] - d[0]*d[5];
  inv[6] = d[3]*d[7] - d[4]*d[6];
  inv[7] = d[1]*d[6] - d[0]*d[7];
  inv[8] = d[0]*d[4] - d[1]*d[3];
  /* Just for a test:
  {
    size_t i;
    printf("\nInput matrix:");
    for(i=0;i<9;++i) { if(i%3==0) printf("\n"); printf("%-10.5f", d[i]); }
    printf("\n-----------\n");
    printf("Inverse matrix:");
    for(i=0;i<9;++i) { if(i%3==0) printf("\n"); printf("%-10.5f", inv[i]); }
    printf("\n\n");
  }
  */
}




















/**************************************************************/
/************        General preparations      ****************/
/**************************************************************/
/* When only one transformation is required, set the suffix for automatic
   output to more meaningful string. */
char *
ui_set_suffix(struct warpparams *p)
{
  /* Return the suffix as soon as nonlinear mode is detected. Just
     ignore the rest. */
  if(p->wcsalign) return "_aligned.fits";

  /* A small independent sanity check: we either need a matrix or at least
     one modular warping. */
  if(p->matrix==NULL && p->modularll==NULL) ui_error_no_warps();

  /* We only want the more meaningful suffix when the list is defined AND
     when its only has one node (the 'next' element is NULL). */
  if(p->matrix==NULL && p->modularll->next==NULL)
    switch(p->modularll->status)
      {
      case UI_KEY_ROTATE:
        return "_rotated.fits";

      case UI_KEY_SCALE:
        return "_scaled.fits";

      case UI_KEY_FLIP:
        return "_flipped.fits";

      case UI_KEY_SHEAR:
        return "_sheared.fits";

      case UI_KEY_TRANSLATE:
        return "_translated.fits";

      case UI_KEY_PROJECT:
        return "_projected.fits";

      default:
        error(EXIT_FAILURE, 0, "a bug! please contact us at %s so we can "
              "fix the problem. The modular warp code %d is not recognized "
              "in 'ui_set_suffix'", PACKAGE_BUGREPORT, p->modularll->status);
        return NULL;
      }
  else
    return "_warped.fits";
}





static void
ui_preparations(struct warpparams *p)
{
  /* Set the output name. This needs to be done before 'ui_finalize_matrix'
     because that function will free the linked list of modular warpings
     which we will need to determine the suffix if no output name is
     specified. */
  if(p->cp.output)
    gal_checkset_writable_remove(p->cp.output, 0, p->cp.dontdelete);
  else
    p->cp.output=gal_checkset_automatic_output(&p->cp, p->inputname,
                                               ui_set_suffix(p));

  /* Prepare the final warping matrix if in linear mode. */
  if(p->wcsalign == 0) ui_matrix_finalize(p);
}



















/**************************************************************/
/************         Set the parameters          *************/
/**************************************************************/
void
ui_read_check_inputs_setup(int argc, char *argv[], struct warpparams *p)
{
  double *matrix;
  uint8_t disttype;
  struct gal_options_common_params *cp=&p->cp;


  /* Include the parameters necessary for argp from this program ('args.h')
     and for the common options to all Gnuastro ('commonopts.h'). We want
     to directly put the pointers to the fields in 'p' and 'cp', so we are
     simply including the header here to not have to use long macros in
     those headers which make them hard to read and modify. This also helps
     in having a clean environment: everything in those headers is only
     available within the scope of this function. */
#include <gnuastro-internal/commonopts.h>
#include "args.h"


  /* Initialize the options and necessary information.  */
  ui_initialize_options(p, program_options, gal_commonopts_options);


  /* Read the command-line options and arguments. */
  errno=0;
  if(argp_parse(&thisargp, argc, argv, 0, 0, p))
    error(EXIT_FAILURE, errno, "parsing arguments");


  /* Read the configuration files and set the common values. */
  gal_options_read_config_set(&p->cp);


  /* Print the option values if asked. Note that this needs to be done
     after the option checks so un-sane values are not printed in the
     output state. */
  gal_options_print_state(&p->cp);


  /* Prepare all the options as FITS keywords to write in output later. */
  gal_options_as_fits_keywords(&p->cp);


  /* Check that the options and arguments fit well with each other. Note
     that arguments don't go in a configuration file. So this test should
     be done after (possibly) printing the option values. */
  ui_check_options_and_arguments(p);


  /* Read/allocate all the necessary starting arrays. */
  ui_preparations(p);


  /* Everything is ready, notify the user of the program starting. */
  if(!p->cp.quiet)
    {
      printf(PROGRAM_NAME" "PACKAGE_VERSION" started on %s",
             ctime(&p->rawtime));
      printf(" Using %zu CPU thread%s\n", p->cp.numthreads,
             p->cp.numthreads==1 ? "." : "s.");
      printf(" Input: %s (hdu: %s)\n", p->inputname, p->cp.hdu);
      if(p->gridfile)
        printf(" Pixel grid: %s (hdu %s)\n", p->gridfile, p->gridhdu);
      if(p->wcsalign)
        {
          disttype=gal_wcs_distortion_identify(p->input->wcs);
          if(disttype!=GAL_WCS_DISTORTION_INVALID)
            printf(" matrix: '%s' distortion from WCS of input.\n",
                   gal_wcs_distortion_to_string(disttype));
        }
      else
        {
          matrix=p->matrix->array;
          printf(" matrix:"
                 "\n\t% .4f   % .4f   % .4f"
                 "\n\t% .4f   % .4f   % .4f"
                 "\n\t% .4f   % .4f   % .4f\n",
                 matrix[0], matrix[1], matrix[2],
                 matrix[3], matrix[4], matrix[5],
                 matrix[6], matrix[7], matrix[8]);
        }
    }
}




















/**************************************************************/
/************      Free allocated, report         *************/
/**************************************************************/
void
ui_free_report(struct warpparams *p, struct timeval *t1)
{
  /* Free the allocated arrays: */
  if(p->inverse) free(p->inverse);
  if(p->gridhdu) free(p->gridhdu);
  if(p->wa.cdelt) p->wa.cdelt=NULL;
  if(p->gridfile) free(p->gridfile);
  if(p->matrix) gal_data_free(p->matrix);
  if(p->inwcsmatrix) free(p->inwcsmatrix);
  if(p->modularll) gal_data_free(p->modularll);

  if(p->wa.input) p->wa.input=NULL;
  if(p->wa.twcs) gal_wcs_free(p->wa.twcs);
  if(p->wa.ctype) gal_data_free(p->wa.ctype);
  if(p->wa.center) gal_data_free(p->wa.center);
  if(p->wa.widthinpix) gal_data_free(p->wa.widthinpix);

  free(p->cp.hdu);
  free(p->cp.output);
  gal_data_free(p->input);
  gal_data_free(p->output);

  /* Report how long the operation took. */
  if(!p->cp.quiet)
    gal_timing_report(t1, PROGRAM_NAME" finished in: ", 0);
}
