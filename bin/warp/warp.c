/*********************************************************************
Warp - Warp images using projective mapping.
Warp is part of GNU Astronomy Utilities (Gnuastro) package.

Corresponding author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
     Pedram Ashofteh Ardakani <pedramardakani@pm.me>
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
#include <config.h>

#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>

#include <gnuastro/wcs.h>
#include <gnuastro/fits.h>
#include <gnuastro/warp.h>
#include <gnuastro/polygon.h>
#include <gnuastro/pointer.h>
#include <gnuastro/threads.h>

#include <gnuastro-internal/timing.h>

#include "main.h"
#include "warp.h"





/***************************************************************/
/**************            MACROS             ******************/
/***************************************************************/
/* Multiply a 2 element vector with a transformation matrix and put the
   result in the 2 element output array. It is assumed that the input is
   from a flat coordinate systemy. */
#define WARP_MAPPOINT(V, T, O)                                  \
  {                                                             \
    (O)[0]=( ( (T)[0]*(V)[0] + (T)[1]*(V)[1] + (T)[2] )         \
             / ( (T)[6]*(V)[0] + (T)[7]*(V)[1] + (T)[8] ) );    \
    (O)[1]=( ( (T)[3]*(V)[0] + (T)[4]*(V)[1] + (T)[5] )         \
             / ( (T)[6]*(V)[0] + (T)[7]*(V)[1] + (T)[8] ) );    \
  }                                                             \





















/***************************************************************/
/**************      Processing function      ******************/
/***************************************************************/
static void *
warp_onthread_linear(void *inparam)
{
  struct gal_threads_params *tprm=(struct gal_threads_params *)inparam;
  struct warpparams *p=(struct warpparams *)tprm->params;

  size_t *extinds=p->extinds, *ordinds=p->ordinds;
  long is0=p->input->dsize[0], is1=p->input->dsize[1];
  double area, filledarea, *input=p->input->array, v=NAN;
  size_t i, j, ind, os1=p->output->dsize[1], numcrn, numinput;
  long x, y, xstart, xend, ystart, yend; /* Might be negative */
  double ocrn[8], icrn_base[8], icrn[8], *output=p->output->array;
  double pcrn[8], *outfpixval=p->outfpixval, ccrn[GAL_POLYGON_MAX_CORNERS];

  for(i=0; tprm->indexs[i] != GAL_BLANK_SIZE_T; ++i)
    {
      ind=tprm->indexs[i];

      /* Initialize the output pixel value: */
      numinput=0;
      output[ind]=filledarea=0.0f;

      /* Set the corners of this output pixel. The ind/os1 and ind%os1
         start from 0. Note that the outfpixval already contains the
         correction for the fact that the FITS standard considers the
         center of first pixel to be at (1.0f, 1.0f).*/
      ocrn[0]=(double)(ind%os1)-0.5f+outfpixval[0];
      ocrn[1]=(double)(ind/os1)-0.5f+outfpixval[1];
      ocrn[2]=(double)(ind%os1)+0.5f+outfpixval[0];
      ocrn[3]=(double)(ind/os1)-0.5f+outfpixval[1];
      ocrn[4]=(double)(ind%os1)-0.5f+outfpixval[0];
      ocrn[5]=(double)(ind/os1)+0.5f+outfpixval[1];
      ocrn[6]=(double)(ind%os1)+0.5f+outfpixval[0];
      ocrn[7]=(double)(ind/os1)+0.5f+outfpixval[1];


      /* Transform the four corners of the output pixel to the input
         image coordinates. */
      for(j=0;j<4;++j) WARP_MAPPOINT(&ocrn[j*2], p->inverse,
                                     &icrn_base[j*2]);

      /* Using the known relationships between the vertice locations,
         put everything in the right place: */
      xstart =
        GAL_DIMENSION_NEARESTINT_HALFHIGHER( icrn_base[extinds[0]] );
      xend   =
        GAL_DIMENSION_NEARESTINT_HALFLOWER(  icrn_base[extinds[1]] ) + 1;
      ystart =
        GAL_DIMENSION_NEARESTINT_HALFHIGHER( icrn_base[extinds[2]] );
      yend   =
        GAL_DIMENSION_NEARESTINT_HALFLOWER(  icrn_base[extinds[3]] ) + 1;
      icrn[0]=icrn_base[ordinds[0]*2]; icrn[1]=icrn_base[ordinds[0]*2+1];
      icrn[2]=icrn_base[ordinds[1]*2]; icrn[3]=icrn_base[ordinds[1]*2+1];
      icrn[4]=icrn_base[ordinds[2]*2]; icrn[5]=icrn_base[ordinds[2]*2+1];
      icrn[6]=icrn_base[ordinds[3]*2]; icrn[7]=icrn_base[ordinds[3]*2+1];

      /* For a check:
      if(ind==9999)
        {
          printf("\n\n\nind: %zu: (%zu, %zu):\n",
                 ind, ind%os1+1, ind/os1+1);
          for(j=0;j<4;++j)
            printf("(%.3f, %.3f) --> (%.3f, %.3f)\n",
                   ocrn[j*2], ocrn[j*2+1], icrn_base[j*2],
                   icrn_base[j*2+1]);
          printf("------- Ordered -------\n");
          for(j=0;j<4;++j) printf("(%.3f, %.3f)\n", icrn[j*2], icrn[j*2+1]);
          printf("------- Start and ending pixels -------\n");
          printf("X: %ld -- %ld\n", xstart, xend);
          printf("Y: %ld -- %ld\n", ystart, yend);
        }
      */

      /* Go over all the input pixels that are covered. Note that x
         and y are the centers of the pixel. */
      for(y=ystart;y<yend;++y)
        {
          /* If the pixel isn't in the image (note that the pixel
             coordinates start from 1), skip this pixel. Note that the
             pixel polygon should be counter clockwise. */
          if( y<1 || y>is0 ) continue;
          pcrn[1]=y-0.5f;      pcrn[3]=y-0.5f;
          pcrn[5]=y+0.5f;      pcrn[7]=y+0.5f;
          for(x=xstart;x<xend;++x)
            {
              if( x<1 || x>is1 ) continue;

              /* Read the value of the input pixel. */
              v=input[(y-1)*is1+x-1];

              pcrn[0]=x-0.5f;          pcrn[2]=x+0.5f;
              pcrn[4]=x+0.5f;          pcrn[6]=x-0.5f;

              /* Find the overlapping (clipped) polygon: */
              gal_polygon_clip(icrn, 4, pcrn, 4, ccrn, &numcrn);
              area=gal_polygon_area(ccrn, numcrn);

              /* Add the fractional value of this pixel. If this
                 output pixel covers a NaN pixel in the input grid,
                 then calculate the area of this NaN pixel to account
                 for it later. */
              if( !isnan(v) )
                {
                  ++numinput;
                  filledarea+=area;
                  output[ind]+=v*area;
                }

              /* For a polygon check:
              if(ind==9999)
                {
                  printf("%zu -- (%zd, %zd):\n", ind, x, y);
                  printf("icrn:\n");
                  for(j=0;j<4;++j)
                    printf("\t%.3f, %.3f\n", icrn[j*2], icrn[j*2+1]);
                  printf("pcrn:\n");
                  for(j=0;j<4;++j)
                    printf("\t%.3f, %.3f\n", pcrn[j*2], pcrn[j*2+1]);
                  printf("ccrn:\n");
                  for(j=0;j<numcrn;++j)
                    printf("\t%.3f, %.3f\n", ccrn[j*2], ccrn[j*2+1]);
                  printf("[%zu]: %.3f of [%ld, %ld]: %f\n", ind,
                         gal_polygon_area(ccrn, numcrn), x, y,
                         input[(y-1)*is1+x-1]);
                }
              */

              /* For a simple pixel value check:
              if(ind==97387)
                printf("%f --> (%zu) %f\n",
                       v*gal_polygon_area(ccrn, numcrn),
                       numinput, output[ind]);
              */
            }
        }

      /* See if the pixel value should be set to NaN or not (because of not
         enough coverage). */
      if(numinput && filledarea/p->opixarea < p->coveredfrac-1e-5)
        numinput=0;

      /* Write the final value to disk: */
      if(numinput==0) output[ind]=NAN;
    }

  /* Wait for all the other threads to finish, then return. */
  if(tprm->b) { pthread_barrier_wait(tprm->b); }
  return NULL;
}

















/***************************************************************/
/**************          Preparations         ******************/
/***************************************************************/
/* Do all the preparations.

   Make the output array. We transform the four corners of the image
   into the output space. To find the four sides of the image.

   About fpixel and lpixel. The point is that we don't want to spend
   time, transforming any pixels which we know will not be in the
   input image.

   Find the proper order of transformed pixel corners from the output
   array to the input array. The order is fixed for all the pixels in
   the image altough the scale might change.
*/
static void
warp_linear_init(struct warpparams *p)
{
  double is0=p->input->dsize[0], is1=p->input->dsize[1];

  double output[8], forarea[8];
  double *matrix=p->matrix->array;
  double icrn[8]={0,0,0,0,0,0,0,0};
  size_t i, *extinds=p->extinds, dsize[2];
  double xmin=DBL_MAX, xmax=-DBL_MAX, ymin=DBL_MAX, ymax=-DBL_MAX;
  double ocrn[8]={0.5f,0.5f,  1.5f,0.5f, 0.5f,1.5f,   1.5f, 1.5f};
  double input[8]={ 0.5f, 0.5f,         is1+0.5f, 0.5f,
                    0.5f, is0+0.5f,     is1+0.5f, is0+0.5f };


  /* Find the range of pixels of the input image. All the input positions
     are moved to the negative by half a pixel since the center of the
     pixel is an integer value.*/
  for(i=0;i<4;++i)
    {
      WARP_MAPPOINT(&input[i*2], matrix, &output[i*2]);
      if(output[i*2]<xmin)     xmin = output[i*2];
      if(output[i*2]>xmax)     xmax = output[i*2];
      if(output[i*2+1]<ymin)   ymin = output[i*2+1];
      if(output[i*2+1]>ymax)   ymax = output[i*2+1];
    }


  /* For a check:
  for(i=0;i<4;++i)
      printf("(%.3f, %.3f) --> (%.3f, %.3f)\n",
             input[i*2], input[i*2+1],
             output[i*2], output[i*2+1]);
  printf("xmin: %.3f\nxmax: %.3f\nymin: %.3f\nymax: %.3f\n",
         xmin, xmax, ymin, ymax);
  */


  /* Set the final size of the image. The X axis is horizontal. The reason
     we are using the halflower variation of 'nearestint' for the maximums
     is that these points are the farthest extremes of the input image. If
     they are half a pixel value, they should point to the pixel before. */
  dsize[1] = GAL_DIMENSION_NEARESTINT_HALFLOWER(xmax)         \
             - GAL_DIMENSION_NEARESTINT_HALFHIGHER(xmin)+1;
  dsize[0] = GAL_DIMENSION_NEARESTINT_HALFLOWER(ymax)         \
             - GAL_DIMENSION_NEARESTINT_HALFHIGHER(ymin)+1;
  p->outfpixval[0]=GAL_DIMENSION_NEARESTINT_HALFHIGHER(xmin);
  p->outfpixval[1]=GAL_DIMENSION_NEARESTINT_HALFHIGHER(ymin);


  /* If we have translation, the 'dsize's and 'outfpixval's should be
     corrected. Note that centeroncorner is also a translation operation,
     but in that scenario, we don't want this feature! */
  if( p->centeroncorner==0 && (matrix[2]!=0.0f || matrix[5]!=0.0f) )
    {
      dsize[1] += abs( (int)(matrix[2]) )+1; /* (int): avoid warnings. */
      dsize[0] += abs( (int)(matrix[5]) )+1;
      if(xmin>0) p->outfpixval[0]=0;
      if(ymin>0) p->outfpixval[1]=0;
    }


  /* For a check:
  printf("Wrapped:\n");
  printf("dsize [C]: (%zu, %zu)\n", dsize[0], dsize[1]);
  printf("outfpixval [FITS]: (%.4f, %.4f)\n", p->outfpixval[0],
         p->outfpixval[1]);
  */


  /* We now know the size of the output and the starting and ending
     coordinates in the output image (bottom left corners of pixels)
     for the transformation. */
  p->output=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 2, dsize,
                           p->input->wcs, 0, p->cp.minmapsize,
                           p->cp.quietmmap, "Warped", p->input->unit, NULL);


  /* Order the corners of the inverse-transformed pixel (from the
     output to the input) in an anti-clockwise transformation. In a
     general homographic transform, the scales of the output pixels
     may change, but the relative positions of the corners will
     not.  */
  for(i=0;i<4;++i)
    {
      ocrn[i*2]   += p->outfpixval[0];
      ocrn[i*2+1] += p->outfpixval[1];
      WARP_MAPPOINT(&ocrn[i*2], p->inverse, &icrn[i*2]);
    }


  /* Order the transformed output pixel. */
  gal_polygon_vertices_sort_convex(icrn, 4, p->ordinds);


  /* Find the area of the output pixel in units of the input pixel,
     this is necessary if we are going to have NAN pixels in the image
     to account for the area lost due to a NAN value. */
  for(i=0;i<4;++i)
    {
      forarea[2*i]=icrn[2*p->ordinds[i]];
      forarea[2*i+1]=icrn[2*p->ordinds[i]+1];
    }
  p->opixarea=gal_polygon_area(forarea, 4);


  /* Find which index after transformation will have the minimum and
     maximum positions along the two axises. We can't use the starting
     loop because that is based on the input image which can be
     not-a-square! So we do it here where pixels are squares. */
  xmin=DBL_MAX; xmax=-DBL_MAX; ymin=DBL_MAX; ymax=-DBL_MAX;
  for(i=0;i<4;++i)
    {
      if(icrn[i*2]<xmin)   { xmin=icrn[i*2];   extinds[0]=i*2;   }
      if(icrn[i*2]>xmax)   { xmax=icrn[i*2];   extinds[1]=i*2;   }
      if(icrn[i*2+1]<ymin) { ymin=icrn[i*2+1]; extinds[2]=i*2+1; }
      if(icrn[i*2+1]>ymax) { ymax=icrn[i*2+1]; extinds[3]=i*2+1; }
    }


  /* For a check:
  for(i=0;i<4;++i)
    printf("(%.3f, %.3f) --> (%.3f, %.3f)\n",
           ocrn[i*2], ocrn[i*2+1], icrn[i*2], icrn[i*2+1]);
  printf("xmin: %.3f\nxmax: %.3f\nymin: %.3f\nymax: %.3f\n",
         xmin, xmax, ymin, ymax);
  */
}





static void
warp_write_to_file(struct warpparams *p, int hasmatrix)
{
  size_t i;
  char keyword[9*FLEN_KEYWORD];
  gal_fits_list_key_t *headers=NULL;
  double *m= hasmatrix ? p->matrix->array : NULL;

  /* Add the appropriate headers: */
  gal_fits_key_write_filename("INF", p->inputname, &headers,
                              0, p->cp.quiet);
  if(hasmatrix)
    for(i=0;i<9;++i)
      {
        sprintf(&keyword[i*FLEN_KEYWORD], "WMTX%zu_%zu", i/3+1, i%3+1);
        gal_fits_key_list_add_end(&headers, GAL_TYPE_FLOAT64,
                                  &keyword[i*FLEN_KEYWORD], 0, &m[i], 0,
                                  "Warp matrix element value", 0, NULL, 0);
      }

  /* Save the output into the proper type and write it. */
  if(p->cp.type && p->cp.type!=p->output->type)
    p->output=gal_data_copy_to_new_type_free(p->output, p->cp.type);
  gal_fits_img_write(p->output, p->cp.output, headers, PROGRAM_NAME);

  /* Write the configuration keywords. */
  gal_fits_key_write_filename("input", p->inputname, &p->cp.okeys,
                              1, p->cp.quiet);
  gal_fits_key_write_config(&p->cp.okeys, "Warp configuration",
                            "WARP-CONFIG", p->cp.output, "0");
}





/* Correct the WCS coordinates (Multiply the 2x2 PC matrix of the WCS
   structure by the INVERSE of the transform in 2x2). Then Multiply the
   crpix array with the ACTUAL transformation matrix. */
void
warp_write_wcs_linear(struct warpparams *p)
{
  double *m=p->matrix->array, diff;
  struct wcsprm *wcs=p->output->wcs;
  double tcrpix[3], *ps=p->cdelt->array;
  double *crpix=wcs?wcs->crpix:NULL, *w=p->inwcsmatrix;

  /* 'tinv' is the 2 by 2 inverse matrix. Recall that 'p->inverse' is 3 by
     3 to account for homogeneous coordinates. */
  double tinv[4]={p->inverse[0]/p->inverse[8], p->inverse[1]/p->inverse[8],
                  p->inverse[3]/p->inverse[8], p->inverse[4]/p->inverse[8]};

  /* Make the WCS corrections if necessary. */
  if(wcs)
    {
      if(p->keepwcs==0)
        {
          /* Correct the input WCS matrix. Since we are re-writing the PC
             matrix from the full rotation matrix (including pixel scale),
             we'll also have to set the CDELT fields to 1. Just to be sure
             that the PC matrix is used in the end by WCSLIB, we'll also
             set altlin to 1.*/
          wcs->altlin=1;
          wcs->cdelt[0] = wcs->cdelt[1] = 1.0f;
          wcs->pc[0] = w[0]*tinv[0] + w[1]*tinv[2];
          wcs->pc[1] = w[0]*tinv[1] + w[1]*tinv[3];
          wcs->pc[2] = w[2]*tinv[0] + w[3]*tinv[2];
          wcs->pc[3] = w[2]*tinv[1] + w[3]*tinv[3];

          /* Correct the CRPIX point. The +1 in the end of the last two
             lines is because FITS counts from 1. */
          tcrpix[0] = m[0]*crpix[0]+m[1]*crpix[1]+m[2];
          tcrpix[1] = m[3]*crpix[0]+m[4]*crpix[1]+m[5];
          tcrpix[2] = m[6]*crpix[0]+m[7]*crpix[1]+m[8];

          crpix[0] = tcrpix[0]/tcrpix[2] - p->outfpixval[0] + 1;
          crpix[1] = tcrpix[1]/tcrpix[2] - p->outfpixval[1] + 1;
        }

      /* Due to floating point errors extremely small values of PC matrix
         can be set to zero and extremely small differences between PC1_1
         and PC2_2 can be ignored. The reason for all the 'fabs' functions
         is because the signs are usually different.*/
      if( fabs(wcs->pc[1])<ABSOLUTEFLTERROR ) wcs->pc[1]=0.0f;
      if( fabs(wcs->pc[2])<ABSOLUTEFLTERROR ) wcs->pc[2]=0.0f;
      diff=fabs(wcs->pc[0])-fabs(wcs->pc[3]);
      if( fabs(diff/ps[0])<RELATIVEFLTERROR )
        wcs->pc[3]=( (wcs->pc[3] < 0.0f ? -1.0f : 1.0f)
                     * fabs(wcs->pc[0]) );
    }

  /* Write the final keywords and the file. */
  warp_write_to_file(p, 1);
}

























/***************************************************************/
/**************       Outside function        ******************/
/***************************************************************/
void
warp(struct warpparams *p)
{
  struct timeval t0;
  gal_warp_wcsalign_t *wa=&p->wa;

  /* Do the preparations and set the pointers to the functions to use. */
  if( p->nonlinearmode )
    {
      /* Calculate and allocate the output image size and WCS */
      if(!p->cp.quiet)
        {
          gal_timing_report(NULL, "Initializing the output image...", 1);
          gettimeofday(&t0, NULL);
        }
      gal_warp_wcsalign_init(wa);

      /* Fill the output image */
      if(!p->cp.quiet)
        {
          gal_timing_report(&t0, "Done", 2);
          gal_timing_report(NULL, "Warping the input image...", 1);
          gettimeofday(&t0, NULL);
        }
      gal_threads_spin_off(gal_warp_wcsalign_onthread, wa,
                           wa->output->size, wa->numthreads,
                           wa->input->minmapsize, wa->input->quietmmap);
      if(!p->cp.quiet) gal_timing_report(&t0, "Done", 2);
      p->output=wa->output;
      wa->output=NULL; /* must be here! */
      gal_warp_wcsalign_free(wa);

      /* Write the final keywords and the file. */
      warp_write_to_file(p, 0);
    }
  else
    {
      warp_linear_init(p);

      /* Fill the output image */
      gal_threads_spin_off(warp_onthread_linear, p, p->output->size,
                           p->cp.numthreads, p->cp.minmapsize,
                           p->cp.quietmmap);

      /* Fix the linear matrix before saving the output image to disk */
      warp_write_wcs_linear(p);
    }


  if(!p->cp.quiet) { printf(" Output: %s\n", p->cp.output); }
}
