/*********************************************************************
Arithmetic - Do arithmetic operations on images.
Arithmetic is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2015-2021, Free Software Foundation, Inc.

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

#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>

#include <gnuastro/wcs.h>
#include <gnuastro/fits.h>
#include <gnuastro/tiff.h>
#include <gnuastro/array.h>
#include <gnuastro-internal/checkset.h>
#include <gnuastro-internal/arithmetic-set.h>

#include "main.h"

#include "operands.h"





/**********************************************************************/
/************            General info on operands       ***************/
/**********************************************************************/
size_t
operands_num(struct arithmeticparams *p)
{
  size_t counter=0;
  struct operand *tmp=NULL;
  for(tmp=p->operands;tmp!=NULL;tmp=tmp->next)
    ++counter;
  return counter;
}




















/**********************************************************************/
/************      Adding to and popping from stack     ***************/
/**********************************************************************/
void
operands_add(struct arithmeticparams *p, char *filename, gal_data_t *data)
{
  int readwcs;
  size_t ndim, *dsize;
  struct operand *newnode;

  /* Some operators might not actually return any dataset (data=NULL), in
     such cases filename will also be NULL (since the operand was not added
     from the command-line). So, we shouldn't add anything to the stack. */
  if(data || filename)
    {
      /* Allocate space for the new operand. */
      errno=0;
      newnode=malloc(sizeof *newnode);
      if(newnode==NULL)
        error(EXIT_FAILURE, errno, "%s: allocating %zu bytes for 'newnode'",
              __func__, sizeof *newnode);

      /* If the 'filename' is the name of a dataset, then use a copy of it.
         otherwise, do the basic analysis. */
      if( filename && gal_arithmetic_set_is_name(p->setprm.named, filename) )
        {
          newnode->filename=NULL;
          newnode->data=gal_arithmetic_set_copy_named(&p->setprm, filename);
        }
      else
        {
          /* Set the basic parameters. */
          newnode->data=data;
          newnode->filename=filename;

          /* See if a HDU must be read or not. */
          if(filename != NULL
             && ( gal_fits_name_is_fits(filename)
                  || gal_tiff_name_is_tiff(filename) ) )
            {
              /* Set the HDU for this filename. */
              if(p->globalhdu)
                gal_checkset_allocate_copy(p->globalhdu, &newnode->hdu);
              else
                newnode->hdu=gal_list_str_pop(&p->hdus);

              /* If no WCS is set yet, use the WCS of this image and remove
                 possibly extra dimensions if necessary. */
              readwcs = (p->wcsfile && !strcmp(p->wcsfile,"none")) ? 0 : 1;
              if(readwcs && p->refdata.wcs==NULL)
                {
                  /* If the HDU is an image, read its size. */
                  dsize = ( gal_fits_hdu_format(filename,
                                                newnode->hdu)==IMAGE_HDU
                            ? gal_fits_img_info_dim(filename,
                                                    newnode->hdu, &ndim)
                            : NULL);

                  /* Read the WCS. */
                  p->refdata.wcs=gal_wcs_read(filename, newnode->hdu,
                                              p->cp.wcslinearmatrix,
                                              0, 0, &p->refdata.nwcs);

                  /* Remove extra (length of 1) dimensions (if we had an
                     image HDU). */
                  if(dsize)
                    {
                      ndim=gal_dimension_remove_extra(ndim, dsize,
                                                      p->refdata.wcs);
                      free(dsize);
                    }

                  /* Let the user know that the WCS is read. */
                  if(p->refdata.wcs && !p->cp.quiet)
                    printf(" - WCS: %s (hdu %s).\n", filename,
                           newnode->hdu);
                }
            }
          else newnode->hdu=NULL;
        }

      /* Make the link to the previous list. */
      newnode->next=p->operands;
      p->operands=newnode;
    }
}





gal_data_t *
operands_pop(struct arithmeticparams *p, char *operator)
{
  size_t i;
  gal_data_t *data;
  char *filename, *hdu;
  struct operand *operands=p->operands;

  /* If the operand linked list has finished, then give an error and
     exit. */
  if(operands==NULL)
    error(EXIT_FAILURE, 0, "not enough operands for the '%s' operator",
          operator);

  /* Set the dataset. If filename is present then read the file
     and fill in the array, if not then just set the array. */
  if(operands->filename)
    {
      /* Set the HDU and filename */
      hdu=operands->hdu;
      filename=operands->filename;

      /* Read the dataset and remove possibly extra dimensions. */
      data=gal_array_read_one_ch(filename, hdu, NULL, p->cp.minmapsize,
                                 p->cp.quietmmap);
      data->ndim=gal_dimension_remove_extra(data->ndim, data->dsize, NULL);

      /* Arithmetic changes the contents of a dataset, so the old name and
         metadata (in the FITS 'EXTNAME' keyword for example) must not be
         used beyond this point. Furthermore, in Arithmetic, the 'name'
         element is used to identify variables (with the 'set-'
         operator). */
      if(data->name)    { free(data->name);    data->name=NULL;    }
      if(data->unit)    { free(data->unit);    data->unit=NULL;    }
      if(data->comment) { free(data->comment); data->comment=NULL; }

      /* When the reference data structure's dimensionality is non-zero, it
         means that this is not the first image read. So, write its basic
         information into the reference data structure for future
         checks. */
      if(p->refdata.ndim==0)
        {
          /* Set the dimensionality. */
          p->refdata.ndim=(data)->ndim;

          /* Allocate the dsize array. */
          errno=0;
          p->refdata.dsize=malloc(p->refdata.ndim
                                  * sizeof *p->refdata.dsize);
          if(p->refdata.dsize==NULL)
            error(EXIT_FAILURE, errno, "%s: allocating %zu bytes for "
                  "p->refdata.dsize", __func__,
                  p->refdata.ndim * sizeof *p->refdata.dsize);

          /* Write the values into it. */
          for(i=0;i<p->refdata.ndim;++i)
            p->refdata.dsize[i]=data->dsize[i];
        }

      /* Report the read image if desired: */
      if(!p->cp.quiet) printf(" - Read: %s (hdu %s).\n", filename, hdu);

      /* Free the HDU string: */
      if(hdu) free(hdu);

      /* Add to the number of popped FITS images: */
      ++p->popcounter;
    }
  else
    data=operands->data;


  /* Remove this node from the queue, return the data structure. */
  p->operands=operands->next;
  free(operands);
  return data;
}




/* Wrapper to use the 'operands_pop' function with the 'set-' operator. */
gal_data_t *
operands_pop_wrapper_set(void *in)
{
  struct gal_arithmetic_set_params *tprm
    = (struct gal_arithmetic_set_params *)in;
  struct arithmeticparams *p=(struct arithmeticparams *)tprm->params;
  return operands_pop(p, "set");
}
