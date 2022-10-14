/*********************************************************************
Fits - View and manipulate FITS extensions and/or headers.
Fits is part of GNU Astronomy Utilities (Gnuastro) package.

Corresponding author:
     Pedram Ashofteh-Ardakani <pedramardakani@pm.me>
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

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gnuastro/wcs.h>
#include <gnuastro/fits.h>
#include <gnuastro/warp.h>
#include <gnuastro/array.h>

#include <gnuastro-internal/timing.h>
#include <gnuastro-internal/checkset.h>

#include "main.h"

#include "meta.h"





/* Prepare the 'wcsalign' structure based on given options. */
static void
meta_initialize(struct fitsparams *p, gal_warp_wcsalign_t *wa)
{
  /* Low-level variables. */
  gal_data_t *input=NULL;
  struct gal_options_common_params *cp=&p->cp;

  /* High-level variables. */
  char *hdu=cp->hdu;
  char *inputname=p->input->v;
  char *suffix="_pixarea.fits";

  /* Set automatic output filename if nothing is given. */
  if(!cp->output)
    cp->output=gal_checkset_automatic_output(cp, inputname, suffix);

  /* Check if we're allowed to delete the output image. If not, fail FAST
     before any CPU-intensive process. */
  gal_checkset_writable_remove(cp->output, 0, cp->dontdelete);

  /* Read the input image and its WCS, must free it when done. */
  input=gal_array_read_one_ch_to_type(inputname, hdu, NULL,
                                      GAL_TYPE_FLOAT64, -1,  0);
  input->wcs=gal_wcs_read(inputname, hdu, 0, 0, 0, &input->nwcs);

  /* Prepare the essential warping variables. */
  wa->input=input;
  wa->numthreads=cp->numthreads;
  wa->edgesampling=p->edgesampling;

  /* We're using the same grid so only 'coveredfrac=1.0f' is sensible. In
     other words, there are no overlaps between the two grids where one
     pixel might encounter neighboring NaN values which then 'coveredfrac'
     takes care of. */
  wa->coveredfrac=1;

  /* Done with initializations, notify the user and start. */
  if(!p->cp.quiet)
    {
      printf(PROGRAM_NAME" "PACKAGE_VERSION" started on %s",
             ctime(&p->rawtime));
      printf(" Using %zu CPU thread%s\n", p->cp.numthreads,
             p->cp.numthreads==1 ? "." : "s.");
      printf(" Input: %s (hdu: %s)\n", inputname, hdu);
      printf(" Output: %s (size: %zux%zu, type: %s)\n", cp->output,
             input->dsize[0], input->dsize[1],
             gal_type_name(p->cp.type, 1));
    }
}





/* Write the configuration keywords and the created image to output. */
static void
meta_write_to_file(struct fitsparams *p, gal_warp_wcsalign_t *wa)
{
  gal_data_t *output=wa->output;
  gal_fits_list_key_t *headers=NULL;

  /* Add configuration headers. */
  gal_fits_key_list_add_end(&headers, GAL_TYPE_STRING,
                            "input", 0, p->input->v, 0,
                            "File given to astfits", 0, NULL, 0);
  gal_fits_key_list_add_end(&headers, GAL_TYPE_SIZE_T,
                            "edgesampling", 0, &p->edgesampling, 0,
                            "Extra sampling along pixel edges.", 0, NULL, 0);
  gal_fits_key_list_add_end(&headers, GAL_TYPE_FLOAT64,
                            "Coveredfrac", 0, &wa->coveredfrac, 0,
                            "Fraction of pixel that is covered by input",
                            0, NULL, 0);

  /* Convert to type and write to file. */
  if(p->cp.type!=output->type)
    output=gal_data_copy_to_new_type_free(output, p->cp.type);
  gal_fits_img_write(output, p->cp.output, headers, PROGRAM_NAME);

  /* Clean up. */
  wa->output=NULL; /* Must be here to prevent double freeing. */
  gal_data_free(output);
  gal_data_free(wa->input);
}





/* Calculate input pixel area on WCS and write as pixel values on the a
   copy of the input. */
void
meta_pixelareaonwcs(struct fitsparams *p)
{
  struct timeval t1;

  /* Store program start time, and the epoch for further timing reports. */
  time(&p->rawtime);
  gettimeofday(&t1, NULL);

  /* Call for an empty wcsalign structure. */
  gal_warp_wcsalign_t wa=gal_warp_wcsalign_template();

  /* Initialize the warping variables based on commandline arguments. */
  meta_initialize(p, &wa);

  /* Execute the warping and fill the data-structure with results. */
  gal_warp_pixelarea(&wa);

  /* Done with calculations, write to file and finish. */
  meta_write_to_file(p, &wa);

  /* Report how long the operation took. */
  if(!p->cp.quiet)
    gal_timing_report(&t1, PROGRAM_NAME" finished in: ", 0);
}
