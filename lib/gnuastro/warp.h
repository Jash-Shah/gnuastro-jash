/*********************************************************************
Warp -- Warp pixels of one dataset to another pixel grid.
This is part of GNU Astronomy Utilities (Gnuastro) package.

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
#ifndef __GAL_WARP_H__
#define __GAL_WARP_H__

/* Include other headers if necessary here. Note that other header files
   must be included before the C++ preparations below */
#include <wcslib/wcs.h>

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


typedef struct
{
  /* Arguments given (and later freed) by the caller. Note that if 'twcs'
     is  given, then the "WCS-Build" elements will be ignored. */
  gal_data_t       *input;  /* Pointer to input image.                   */
  size_t       numthreads;  /* Number of threads to use.                 */
  double      coveredfrac;  /* Acceptable fraction of output covered.    */
  size_t     edgesampling;  /* Order of samplings along each pixel edge. */
  gal_data_t  *widthinpix;  /* Output image width and height in pixels.  */
  struct wcsprm     *twcs;  /* WCS-Predefined: the wcsprm.               */
  gal_data_t       *ctype;  /* WCS-Build: Type of the coordinates.       */
  gal_data_t       *cdelt;  /* WCS-Build: Pixel scale of the output.     */
  gal_data_t      *center;  /* WCS-Build: Center of output in RA and Dec.*/

  /* Output (must be freed by caller) */
  gal_data_t      *output;  /* Pointer to output data structure.         */

  /* Internal variables (allocated and freed internally)  */
  size_t               v0;  /* The first vertical corner in each pixel.  */
  size_t             nhor;  /* No. of vertices on top side of output.    */
  size_t             ncrn;  /* No. of total vertices around each pixel.  */
  size_t             gcrn;  /* Gap between corners of each row.          */
  int               isccw;  /* Rotation orientation of pixel edges.      */
  gal_data_t    *vertices;  /* Stores all vertice coords of output img.  */
} gal_warp_wcsalign_t;





/* Return an empty set of the wcsalign data structure. */
gal_warp_wcsalign_t
gal_warp_wcsalign_template();


/* Create the empty output WCS-ready image. */
void
gal_warp_wcsalign_init(gal_warp_wcsalign_t *wa);


/* Fill nonlinear output by pixel. */
void
gal_warp_wcsalign_onpix(gal_warp_wcsalign_t *wa, size_t ind);


/* Worker function to align per pixel. */
void *
gal_warp_wcsalign_onthread(void *inparam);


/* Spin-off the threads and finalize the output 'gal_data_t' image in
   'wa->output'. */
void
gal_warp_wcsalign(gal_warp_wcsalign_t *wa);


/* Clean up ONLY the internal variables. Caller must 'free' their own
   inputs as well as the output (e.g. the input image). */
void
gal_warp_wcsalign_free(gal_warp_wcsalign_t *wa);


/* Return an image where each pixel shows its own area on the sky. */
void
gal_warp_pixelarea(gal_warp_wcsalign_t *wa);


__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_WARP_H__ */
