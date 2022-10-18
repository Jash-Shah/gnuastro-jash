/*********************************************************************
Statistics - Statistical analysis on input dataset.
Statistics is part of GNU Astronomy Utilities (Gnuastro) package.

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
#include <config.h>

#include <math.h>
#include <errno.h>
#include <error.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gsl/gsl_fit.h>

#include <gnuastro/fit.h>
#include <gnuastro/wcs.h>
#include <gnuastro/fits.h>
#include <gnuastro/tile.h>
#include <gnuastro/blank.h>
#include <gnuastro/pointer.h>
#include <gnuastro/arithmetic.h>
#include <gnuastro/statistics.h>
#include <gnuastro/interpolate.h>
#include <gnuastro/permutation.h>

#include <gnuastro-internal/timing.h>
#include <gnuastro-internal/checkset.h>

#include "main.h"

#include "ui.h"
#include "sky.h"
#include "contour.h"
#include "statistics.h"





/*******************************************************************/
/**************           Print in one row           ***************/
/*******************************************************************/
static gal_data_t *
statistics_pull_out_element(gal_data_t *input, size_t index)
{
  size_t dsize=1;
  gal_data_t *out=gal_data_alloc(NULL, input->type, 1, &dsize,
                                 NULL, 1, -1, 1, NULL, NULL, NULL);
  memcpy( out->array,
          gal_pointer_increment(input->array, index, input->type),
          gal_type_sizeof(input->type) );
  return out;
}





static double
statistics_read_check_args(struct statisticsparams *p)
{
  double d;
  if(p->tp_args==NULL)
    error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s so we can "
          "address the problem. Not enough arguments for the requested "
          "single measurement options", __func__, PACKAGE_BUGREPORT);
  d=gal_list_f64_pop(&p->tp_args);
  return d;
}





static void
statistics_print_one_row(struct statisticsparams *p)
{
  int mustfree;
  char *toprint;
  double arg, *d;
  gal_list_i32_t *tmp;
  size_t dsize=1, counter;
  gal_data_t *sum=NULL, *med=NULL, *meanstd=NULL, *modearr=NULL;
  gal_data_t *tmpv, *sclip=NULL, *out=NULL, *num=NULL, *min=NULL, *max=NULL;

  /* The user can ask for any of the operators more than once, also some
     operators might return more than one usable value (like mode). So we
     will calculate the desired values once, and then print them any number
     of times. */
  for(tmp=p->singlevalue; tmp!=NULL; tmp=tmp->next)
    switch(tmp->v)
      {
      /* Calculate respective values. Checking with 'if(num==NULL)' gives
         compiler warnings of 'this if clause does not guard ...'. So we
         are using this empty-if and else statement. */
      case UI_KEY_NUMBER:
        num = num ? num : gal_statistics_number(p->input);           break;
      case UI_KEY_MINIMUM:
        min = min ? min : gal_statistics_minimum(p->input);          break;
      case UI_KEY_MAXIMUM:
        max = max ? max : gal_statistics_maximum(p->input);          break;
      case UI_KEY_SUM:
        sum = sum ? sum : gal_statistics_sum(p->input);              break;
      case UI_KEY_MEDIAN:
        med = med ? med : gal_statistics_median(p->sorted, 0); break;
      case UI_KEY_STD:
      case UI_KEY_MEAN:
      case UI_KEY_QUANTOFMEAN:
        meanstd = meanstd ? meanstd : gal_statistics_mean_std(p->input);
        break;
      case UI_KEY_MODE:
      case UI_KEY_MODEQUANT:
      case UI_KEY_MODESYM:
      case UI_KEY_MODESYMVALUE:
        modearr = ( modearr
                    ? modearr
                    : gal_statistics_mode(p->sorted, p->mirrordist, 0) );
        d=modearr->array;
        if(d[2]<GAL_STATISTICS_MODE_GOOD_SYM) d[0]=d[1]=NAN;
        break;
      case UI_KEY_SIGCLIPSTD:
      case UI_KEY_SIGCLIPMEAN:
      case UI_KEY_SIGCLIPNUMBER:
      case UI_KEY_SIGCLIPMEDIAN:
        sclip = ( sclip
                  ? sclip
                  : gal_statistics_sigma_clip(p->sorted, p->sclipparams[0],
                                              p->sclipparams[1], 0, 1) );
        break;

      /* Will be calculated as printed. */
      case UI_KEY_QUANTILE:
      case UI_KEY_QUANTFUNC:
        break;

      /* The option isn't recognized. */
      default:
        error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s so we "
              "can address the problem. Operation code %d not recognized",
              __func__, PACKAGE_BUGREPORT, tmp->v);
      }


  /* Print every requested number. */
  counter=0;
  for(tmp=p->singlevalue; tmp!=NULL; tmp=tmp->next)
    {
      /* By default don't free anything. */
      mustfree=0;

      /* Get the output. */
      switch(tmp->v)
        {
        /* Previously calculated values. */
        case UI_KEY_NUMBER:     out=num;                  break;
        case UI_KEY_MINIMUM:    out=min;                  break;
        case UI_KEY_MAXIMUM:    out=max;                  break;
        case UI_KEY_SUM:        out=sum;                  break;
        case UI_KEY_MEDIAN:     out=med;                  break;
        case UI_KEY_MEAN:
          out=statistics_pull_out_element(meanstd, 0); mustfree=1; break;
        case UI_KEY_STD:
          out=statistics_pull_out_element(meanstd, 1); mustfree=1; break;
        case UI_KEY_MODE:
          out=statistics_pull_out_element(modearr, 0); mustfree=1; break;
        case UI_KEY_MODEQUANT:
          out=statistics_pull_out_element(modearr, 1); mustfree=1; break;
        case UI_KEY_MODESYM:
          out=statistics_pull_out_element(modearr, 2); mustfree=1; break;
        case UI_KEY_MODESYMVALUE:
          out=statistics_pull_out_element(modearr, 3); mustfree=1; break;
        case UI_KEY_SIGCLIPSTD:
          out=statistics_pull_out_element(sclip,   3); mustfree=1; break;
        case UI_KEY_SIGCLIPMEAN:
          out=statistics_pull_out_element(sclip,   2); mustfree=1; break;
        case UI_KEY_SIGCLIPMEDIAN:
          out=statistics_pull_out_element(sclip,   1); mustfree=1; break;
        case UI_KEY_SIGCLIPNUMBER:
          out=statistics_pull_out_element(sclip,   0); mustfree=1; break;

        /* Not previously calculated. */
        case UI_KEY_QUANTILE:
          mustfree=1;
          arg = statistics_read_check_args(p);
          out = gal_statistics_quantile(p->sorted, arg, 0);
          break;

        case UI_KEY_QUANTFUNC:
          mustfree=1;
          arg = statistics_read_check_args(p);
          tmpv = gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                NULL, 1, -1, 1, NULL, NULL, NULL);
          *((double *)(tmpv->array)) = arg;
          tmpv = gal_data_copy_to_new_type_free(tmpv, p->input->type);
          out = gal_statistics_quantile_function(p->sorted, tmpv, 0);
          gal_data_free(tmpv);
          break;

        case UI_KEY_QUANTOFMEAN:
          mustfree=1;
          tmpv=statistics_pull_out_element(meanstd, 0);
          out = gal_statistics_quantile_function(p->sorted, tmpv, 0);
          gal_data_free(tmpv);
          break;

        /* The option isn't recognized. */
        default:
          error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s so we "
                "can address the problem. Operation code %d not recognized",
                __func__, PACKAGE_BUGREPORT, tmp->v);
        }

      /* Print the number. Note that we don't want any extra white space
         characters before or after the printed outputs. So we have defined
         'counter' to add a single white space character before any element
         except the first one. */
      toprint=gal_type_to_string(out->array, out->type, 0);
      printf("%s%s", counter ? " " : "", toprint);
      free(toprint);

      /* Clean up (if necessary). */
      ++counter;
      if(mustfree) gal_data_free(out);
    }


  /* Print a new line. */
  printf("\n");


  /* Clean any of the allocated arrays. */
  if(num)     gal_data_free(num);
  if(min)     gal_data_free(min);
  if(max)     gal_data_free(max);
  if(sum)     gal_data_free(sum);
  if(med)     gal_data_free(med);
  if(sclip)   gal_data_free(sclip);
  if(meanstd) gal_data_free(meanstd);
  if(modearr) gal_data_free(modearr);
}




















/*******************************************************************/
/**************         Single value on tile         ***************/
/*******************************************************************/
static void
statistics_interpolate_and_write(struct statisticsparams *p,
                                 gal_data_t *values, char *output)
{
  gal_data_t *interpd;
  struct gal_options_common_params *cp=&p->cp;

  /* Do the interpolation (if necessary). */
  if( p->interpolate
      && !(p->cp.interponlyblank && gal_blank_present(values, 1)==0) )
    {
      interpd=gal_interpolate_neighbors(values, &cp->tl,
                                        cp->interpmetric,
                                        cp->interpnumngb,
                                        cp->numthreads,
                                        cp->interponlyblank, 0,
                                        GAL_INTERPOLATE_NEIGHBORS_FUNC_MEDIAN);
      gal_data_free(values);
      values=interpd;
    }

  /* Write the values. */
  gal_tile_full_values_write(values, &cp->tl, !p->ignoreblankintiles,
                             output, NULL, PROGRAM_NAME);
  gal_fits_key_write_filename("input", p->inputname, &p->cp.okeys, 1,
                              p->cp.quiet);
  gal_fits_key_write_config(&p->cp.okeys, "Statistics configuration",
                            "STATISTICS-CONFIG", output, "0");
}





static void
statistics_on_tile(struct statisticsparams *p)
{
  double arg=0;
  gal_list_i32_t *operation;
  gal_data_t *tile, *values;
  size_t tind, dsize=1, mind=-1;
  uint8_t type=GAL_TYPE_INVALID;
  gal_data_t *tmp=NULL, *tmpv=NULL, *ttmp;
  struct gal_options_common_params *cp=&p->cp;
  struct gal_tile_two_layer_params *tl=&p->cp.tl;
  char *output=gal_checkset_automatic_output(cp, cp->output
                                             ? cp->output
                                             : p->inputname,
                                             "_ontile.fits");

  /* Do the operation on each tile. */
  for(operation=p->singlevalue; operation!=NULL; operation=operation->next)
    {
      /* Set the type of the output array. */
      switch(operation->v)
        {
        case UI_KEY_NUMBER:
          type=GAL_TYPE_INT32; break;

        case UI_KEY_MINIMUM:
        case UI_KEY_MAXIMUM:
        case UI_KEY_MEDIAN:
        case UI_KEY_MODE:
        case UI_KEY_QUANTFUNC:
          type=p->input->type; break;

        case UI_KEY_SUM:
        case UI_KEY_MEAN:
        case UI_KEY_STD:
        case UI_KEY_QUANTILE:
        case UI_KEY_MODEQUANT:
        case UI_KEY_MODESYM:
        case UI_KEY_MODESYMVALUE:
          type=GAL_TYPE_FLOAT64; break;

        default:
          error(EXIT_FAILURE, 0, "%s: a bug! %d is not a recognized "
                "operation code", __func__, operation->v);
        }

      /* Allocate the space necessary to keep the value for each tile. */
      values=gal_data_alloc(NULL, type, p->input->ndim, tl->numtiles, NULL,
                            0, p->input->minmapsize, p->cp.quietmmap,
                            NULL, NULL, NULL);

      /* Read the argument for those operations that need it. This is done
         here, because below, the functions are repeated on each tile. */
      switch(operation->v)
        {
        case UI_KEY_QUANTILE:
          arg = statistics_read_check_args(p);
          break;
        case UI_KEY_QUANTFUNC:
          arg = statistics_read_check_args(p);
          tmpv = gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                NULL, 1, -1, 1, NULL, NULL, NULL);
          *((double *)(tmpv->array)) = arg;
          tmpv = gal_data_copy_to_new_type_free(tmpv, p->input->type);
        }

      /* Do the operation on each tile. */
      tind=0;
      for(tile=tl->tiles; tile!=NULL; tile=tile->next)
        {
          /* Do the proper operation. */
          switch(operation->v)
            {
            case UI_KEY_NUMBER:
              tmp=gal_statistics_number(tile);                      break;

            case UI_KEY_MINIMUM:
              tmp=gal_statistics_minimum(tile);                     break;

            case UI_KEY_MAXIMUM:
              tmp=gal_statistics_maximum(tile);                     break;

            case UI_KEY_MEDIAN:
              tmp=gal_statistics_median(tile, 1);                   break;

            case UI_KEY_QUANTFUNC:
              tmp=gal_statistics_quantile_function(tile, tmpv, 1);  break;

            case UI_KEY_SUM:
              tmp=gal_statistics_sum(tile);                         break;

            case UI_KEY_MEAN:
              tmp=gal_statistics_mean(tile);                        break;

            case UI_KEY_STD:
              tmp=gal_statistics_std(tile);                         break;

            case UI_KEY_QUANTILE:
              tmp=gal_statistics_quantile(tile, arg, 1);            break;

            case UI_KEY_MODE:
            case UI_KEY_MODESYM:
            case UI_KEY_MODEQUANT:
            case UI_KEY_MODESYMVALUE:
              switch(operation->v)
                {
                case UI_KEY_MODE:         mind=0;  break;
                case UI_KEY_MODESYM:      mind=2;  break;
                case UI_KEY_MODEQUANT:    mind=1;  break;
                case UI_KEY_MODESYMVALUE: mind=3;  break;
                }
              tmp=gal_statistics_mode(tile, p->mirrordist, 1);
              ttmp=statistics_pull_out_element(tmp, mind);
              gal_data_free(tmp);
              tmp=ttmp;
              break;

            default:
              error(EXIT_FAILURE, 0, "%s: a bug! please contact us at %s to "
                    "fix the problem. The operation code %d is not "
                    "recognized", __func__, PACKAGE_BUGREPORT, operation->v);
            }

          /* Put the output value into the 'values' array and clean up. */
          tmp=gal_data_copy_to_new_type_free(tmp, type);
          memcpy(gal_pointer_increment(values->array, tind++, values->type),
                 tmp->array, gal_type_sizeof(type));
          gal_data_free(tmp);
        }

      /* Do the interpolation (if necessary) and write the array into the
         output. */
      statistics_interpolate_and_write(p, values, output);

      /* Clean up. */
      gal_data_free(values);
      if(operation->v==UI_KEY_QUANTFUNC) gal_data_free(tmpv);
    }

  /* Clean up. */
  free(output);
}



















/*******************************************************************/
/**************             ASCII plots              ***************/
/*******************************************************************/
static void
print_ascii_plot(struct statisticsparams *p, gal_data_t *plot,
                 gal_data_t *bins, int h1_c0, int printinfo)
{
  int i, j;
  size_t *s, *sf, max=0;
  double *b, v, halfbinwidth, correction;

  /* Find the maximum of the plot. */
  sf=(s=plot->array)+plot->size; do max = *s>max ? *s : max; while(++s<sf);

  /* Print the range so the user knows. */
  if(printinfo)
    {
      b=bins->array;
      halfbinwidth = (b[1]-b[0])/2;
      printf("\nASCII %s:\n", ( h1_c0 ? "Histogram" :
                                "Cumulative frequency plot") );
      if(h1_c0) printf("Number: %zu\n", p->input->size);
      printf("Y: (linear: 0 to %zu)\n", max);
      printf("X: (linear: %g -- %g, in %zu bins)\n", b[0]-halfbinwidth,
             b[ bins->size - 1 ] + halfbinwidth, bins->size);
    }

  /* Print the ASCII plot: */
  s=plot->array;
  correction = (double)(p->asciiheight) / (double)max;
  for(i=p->asciiheight;i>=0;--i)
    {
      printf(" |");
      for(j=0;j<plot->size;++j)
        {
          v = (double)s[j] * correction;
          if( v >= ((double)i-0.5f) && v > 0.0f ) printf("*");
          else printf(" ");
        }
      printf("\n");
    }
  printf(" |");
  for(j=0;j<plot->size;++j) printf("-");
  printf("\n\n");
}





/* Data structure that must be fed into 'gal_statistics_regular_bins'.*/
static gal_data_t *
set_bin_range_params(struct statisticsparams *p, size_t dim)
{
  size_t rsize=2;
  gal_data_t *range=NULL;

  if(p->manualbinrange)
    {
      /* Allocate the range data structure. */
      range=gal_data_alloc(NULL, GAL_TYPE_FLOAT32, 1, &rsize, NULL,
                           0, -1, 1, NULL, NULL, NULL);
      switch(dim)
        {
        case 1:
          ((float *)(range->array))[0]=p->greaterequal;
          ((float *)(range->array))[1]=p->lessthan;
          break;
        case 2:
          ((float *)(range->array))[0]=p->greaterequal2;
          ((float *)(range->array))[1]=p->lessthan2;
          break;
        default:
          error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at %s to "
                "address the problem. The value %zu for 'dim' isn't "
                "recogized", __func__, PACKAGE_BUGREPORT, dim);
        }
    }
  return range;
}





static void
ascii_plots(struct statisticsparams *p)
{
  gal_data_t *bins, *hist, *cfp=NULL, *range=NULL;

  /* Make the bins and the respective plot. */
  range=set_bin_range_params(p, 1);
  bins=gal_statistics_regular_bins(p->input, range, p->numasciibins, NAN);
  hist=gal_statistics_histogram(p->input, bins, 0, 0);
  if(p->asciicfp)
    {
      bins->next=hist;
      cfp=gal_statistics_cfp(p->input, bins, 0);
    }

  /* Print the plots. */
  if(p->asciihist)  print_ascii_plot(p, hist, bins, 1, 1);
  if(p->asciicfp)   print_ascii_plot(p, cfp,  bins, 0, 1);

  /* Clean up.*/
  gal_data_free(bins);
  gal_data_free(hist);
  gal_data_free(range);
  if(p->asciicfp) gal_data_free(cfp);
}




















/*******************************************************************/
/*******    Histogram and cumulative frequency tables    ***********/
/*******************************************************************/
static char *
statistics_output_name(struct statisticsparams *p, char *suf, int *isfits)
{
  int use_auto_output=0;
  char *out, *fix, *suffix=NULL;

  /* Automatic output should be used when no output name was specified or
     we have more than one output file. */
  use_auto_output = p->cp.output ? (p->numoutfiles>1 ? 1 : 0) : 1;

  /* Set the 'fix' and 'suffix' strings. Note that 'fix' is necessary in
     every case, even when no automatic output is to be used. Since it is
     used to determine the format of the output. */
  fix = ( *isfits
          ? "fits"
          : ( p->cp.output
              ? gal_fits_name_is_fits(p->cp.output) ? "fits" : "txt"
              : "txt" ) );
  if(use_auto_output)
    if( asprintf(&suffix, "%s.%s", suf, fix)<0 )
      error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);

  /* Make the output name. */
  out = ( use_auto_output
          ? gal_checkset_automatic_output(&p->cp, p->inputname, suffix)
          : p->cp.output );

  /* See if it is a FITS file. */
  *isfits=strcmp(fix, "fits")==0;

  /* Make sure it doesn't already exist. */
  gal_checkset_writable_remove(out, 0, p->cp.dontdelete);

  /* Clean up and return. */
  if(suffix) free(suffix);
  return out;
}





void
write_output_table(struct statisticsparams *p, gal_data_t *table,
                   char *suf, char *contents)
{
  int isfits=0;
  char *tmp, *output;
  gal_list_str_t *comments=NULL;

  /* Set the output. */
  output=statistics_output_name(p, suf, &isfits);

  /* Write the comments, NOTE: we are writing the first two in reverse of
     the order we want them. They will later be freed as part of the list's
     freeing.*/
  tmp=gal_fits_name_save_as_string(p->inputname, p->cp.hdu);
  gal_list_str_add(&comments, tmp, 0);

  if( asprintf(&tmp, "%s created from:", contents)<0 )
    error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
  gal_list_str_add(&comments, tmp, 0);

  if(!isfits)  /* The intro info will be in FITS files anyway.*/
    gal_table_comments_add_intro(&comments, PROGRAM_STRING, &p->rawtime);


  /* Write the table. */
  gal_checkset_writable_remove(output, 0, p->cp.dontdelete);
  gal_table_write(table, NULL, comments, p->cp.tableformat, output,
                  "TABLE", 0);


  /* Write the configuration information if we have a FITS output. */
  if(isfits)
    {
      gal_fits_key_write_filename("input", p->inputname, &p->cp.okeys, 1,
                                  p->cp.quiet);
      gal_fits_key_write_config(&p->cp.okeys, "Statistics configuration",
                                "STATISTICS-CONFIG", output, "0");
    }


  /* Let the user know, if we aren't in quiet mode. */
  if(!p->cp.quiet)
    printf("%s created.\n", output);


  /* Clean up. */
  gal_list_str_free(comments, 1);
  if(output!=p->cp.output) free(output);
}





static void
save_hist_and_or_cfp(struct statisticsparams *p)
{
  char *suf, *contents;
  gal_data_t *bins, *hist, *cfp=NULL, *range=NULL;

  /* Set the bins and make the histogram, this is necessary for both the
     histogram and CFP (recall that the CFP is built from the
     histogram). */
  range=set_bin_range_params(p, 1);
  bins=gal_statistics_regular_bins(p->input, range, p->numbins,
                                   p->onebinstart);
  hist=gal_statistics_histogram(p->input, bins, p->normalize, p->maxbinone);


  /* Set the histogram as the next pointer of bins. This is again necessary
     in both cases: when only a histogram is requested, it is used for the
     plotting. When only a CFP is desired, it is used as input into
     'gal_statistics_cfp'. */
  bins->next=hist;


  /* Make the cumulative frequency plot if the user wanted it. Make the
     CFP, note that for the CFP, 'maxbinone' and 'normalize' are the same:
     the last bin (largest value) must be one. So if any of them are given,
     then set the last argument to 1.*/
  if(p->cumulative)
    cfp=gal_statistics_cfp(p->input, bins, p->normalize || p->maxbinone);


  /* FITS tables don't accept 'uint64_t', so to be consistent, we'll conver
     the histogram and CFP to 'uint32_t'.*/
  if(hist->type==GAL_TYPE_UINT64)
    hist=gal_data_copy_to_new_type_free(hist, GAL_TYPE_UINT32);
  if(cfp && cfp->type==GAL_TYPE_UINT64)
    cfp=gal_data_copy_to_new_type_free(cfp, GAL_TYPE_UINT32);


  /* Finalize the next pointers. */
  bins->next=hist;
  hist->next=cfp;


  /* Prepare the contents. */
  if(p->histogram && p->cumulative)
    { suf="-hist-cfp"; contents="Histogram and cumulative frequency plot"; }
  else if(p->histogram)
    { suf="-hist";     contents="Histogram"; }
  else
    { suf="-cfp";      contents="Cumulative frequency plot"; }


  /* Set the output file name. */
  write_output_table(p, bins, suf, contents);

  /* Clean up. */
  gal_data_free(range);
}




/* In the WCS standard, '-' is meaningful, so if a column name contains
   '-', it should be changed to '_'. */
static char *
histogram_2d_set_ctype(char *orig, char *backup)
{
  char *c, *out=NULL;

  /* If an original name exists, then check it. Otherwise, just return the
     backup string so 'CTYPE' isn't left empty. */
  if(orig)
    {
      /* Copy the original name into a newly allocated space because we
         later want to free it. */
      gal_checkset_allocate_copy(orig, &out);

      /* Parse the copy and if a dash is present, correct it. */
      for(c=out; *c!='\0'; ++c) if(*c=='-') *c='_';
    }
  else
    gal_checkset_allocate_copy(backup, &out);

  /* Return the final string. */
  return out;
}





static void
histogram_2d(struct statisticsparams *p)
{
  int isfits=1;
  int32_t *imgarr;
  double *d1, *d2;
  uint32_t *histarr;
  gal_data_t *range1, *range2;
  gal_data_t *img, *hist2d, *bins;
  size_t i, j, dsize[2], nb1=p->numbins, nb2=p->numbins2;
  char *output, suf[]="-hist2d", contents[]="2D Histogram";

  /* WCS-related arrays. */
  char *cunit[2], *ctype[2];
  double crpix[2], crval[2], cdelt[2], pc[4]={1,0,0,1};

  /* Set the bins for each dimension */
  range1=set_bin_range_params(p, 1);
  range2=set_bin_range_params(p, 2);
  bins=gal_statistics_regular_bins(p->input, range1, nb1,
                                   p->onebinstart);
  bins->next=gal_statistics_regular_bins(p->input->next, range2,
                                         nb2, p->onebinstart2);

  /* Build the 2D histogram. */
  hist2d=gal_statistics_histogram2d(p->input, bins);

  /* Write the histogram into a 2D FITS image. Note that in the FITS image
     standard, the first axis is the fastest array (unlike the default
     format we have adopted for the tables). */
  if(!strcmp(p->histogram2d,"image"))
    {
      /* Allocate the 2D image array. Note that 'dsize' is in the order of
         C, where the first dimension is the slowest. However, in FITS the
         fastest dimension is the first. */
      dsize[0]=nb2;
      dsize[1]=nb1;
      histarr=hist2d->next->next->array;
      img=gal_data_alloc(NULL, GAL_TYPE_INT32, 2, dsize, NULL, 0,
                         p->cp.minmapsize, p->cp.quietmmap,
                         NULL, NULL, NULL);

      /* Fill the array values. */
      imgarr=img->array;
      for(i=0;i<nb2;++i)
        for(j=0;j<nb1;++j)
          imgarr[i*nb1+j]=histarr[j*nb2+i];

      /* Set the WCS. */
      d1=bins->array;
      d2=bins->next->array;
      crpix[0] = 1;                   crpix[1] = 1;
      crval[0] = d1[0];               crval[1] = d2[0];
      cdelt[0] = d1[1]-d1[0];         cdelt[1] = d2[1]-d2[0];
      cunit[0] = p->input->unit;      cunit[1] = p->input->next->unit;
      ctype[0] = histogram_2d_set_ctype(p->input->name, "X");
      ctype[1] = histogram_2d_set_ctype(p->input->next->name, "Y");
      img->wcs=gal_wcs_create(crpix, crval, cdelt, pc, cunit, ctype, 2,
                              p->cp.wcslinearmatrix);

      /* Write the output. */
      output=statistics_output_name(p, suf, &isfits);
      gal_fits_img_write(img, output, NULL, PROGRAM_STRING);
      gal_fits_key_write_filename("input", p->inputname, &p->cp.okeys, 1,
                                  p->cp.quiet);
      gal_fits_key_write_config(&p->cp.okeys, "Statistics configuration",
                                "STATISTICS-CONFIG", output, "0");

      /* Clean up and let the user know that the histogram is built. */
      free(ctype[0]);
      free(ctype[1]);
      gal_data_free(img);
      if(!p->cp.quiet) printf("%s created.\n", output);
    }
  /* Write 2D histogram as a table. */
  else
    write_output_table(p, hist2d, suf, contents);

  /* Clean up. */
  gal_data_free(range1);
  gal_data_free(range2);
  gal_list_data_free(bins);
  gal_list_data_free(hist2d);
}





void
print_mirror_hist_cfp(struct statisticsparams *p)
{
  size_t dsize=1;
  gal_data_t *table;
  double mirror_val;
  gal_data_t *mirror=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &dsize,
                                    NULL, 1, -1, 1, NULL, NULL, NULL);

  /* Convert the given mirror value into the type of the input dataset. */
  *((double *)(mirror->array)) = p->mirror;
  mirror=gal_data_copy_to_new_type_free(mirror, p->input->type);

  /* Make the table columns. */
  table=gal_statistics_mode_mirror_plots(p->sorted, mirror, p->numbins, 0,
                                         &mirror_val);

  if(p->mirror!=mirror_val)
    {
      fprintf(stderr, "Warning: Mirror value is %f.\n", mirror_val);
      if(!p->cp.quiet)
        fprintf(stderr, "\nNote that the mirror distribution is discrete "
                "and depends on the input data. So the closest point in "
                "the data to your desired mirror at %f was %f.\n\n",
                p->mirror, mirror_val);
    }

  /* If the mirror value was out-of-range, then no table will be made. */
  if(table)
    write_output_table(p, table, "_mirror_hist_cfp",
                       "Histogram and CFP of mirror distribution");
  else
    error(EXIT_FAILURE, 0, "%s: mirror value %g is out of range",
          __func__, p->mirror);
}


















/*******************************************************************/
/**************           Basic information          ***************/
/*******************************************************************/
/* To keep things in 'print_basics' clean, we'll define the input data
   here, then only print the values there. */
void
print_input_info(struct statisticsparams *p)
{
  char *str, *name, *col=NULL;

  /* Print the program name and version. */
  printf("%s\n", PROGRAM_STRING);

  /* Print the input information, if the input was a table, we also need to
     give the column information. When the column has a name, it will be
     printed, when it doesn't, we'll use the same string the user gave. */
  printf("-------\n");
  name=gal_fits_name_save_as_string(p->inputname, p->cp.hdu);
  printf("Input: %s\n", name);

  /* If a table was given, print the column. */
  if(p->columns) printf("Column: %s\n",
                        p->input->name ? p->input->name : p->columns->v);

  /* Range. */
  str=NULL;
  if( !isnan(p->greaterequal) && !isnan(p->lessthan) )
    {
      if( asprintf(&str, "from (inclusive) %g, up to (exclusive) %g",
                   p->greaterequal, p->lessthan)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
    }
  else if( !isnan(p->greaterequal) )
    {
      if( asprintf(&str, "from (inclusive) %g", p->greaterequal)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
    }
  else if( !isnan(p->lessthan) )
    {
      if( asprintf(&str, "up to (exclusive) %g", p->lessthan)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
    }
  if(str)
    {
      printf("Range: %s.\n", str);
      free(str);
    }

  /* Units. */
  if(p->input->unit) printf("Unit: %s\n", p->input->unit);

  /* Clean up. */
  if(col) free(col);
  free(name);
  printf("-------\n");
}





/* This function will report the simple immediate statistics of the
   data. For the average and standard deviation, the unsorted data is
   used so we don't suddenly encounter rounding errors. */
void
print_basics(struct statisticsparams *p)
{
  char *str;
  int namewidth=40;
  float mirrdist=1.5;
  double mean, std, *d;
  gal_data_t *tmp, *bins, *hist, *range=NULL;

  /* Define the input dataset. */
  print_input_info(p);

  /* Print the number: */
  printf("  %-*s %zu\n", namewidth, "Number of elements:", p->input->size);

  /* Minimum: */
  tmp=gal_statistics_minimum(p->input);
  str=gal_type_to_string(tmp->array, tmp->type, 0);
  printf("  %-*s %s\n", namewidth, "Minimum:", str);
  gal_data_free(tmp);
  free(str);

  /* Maximum: */
  tmp=gal_statistics_maximum(p->input);
  str=gal_type_to_string(tmp->array, tmp->type, 0);
  printf("  %-*s %s\n", namewidth, "Maximum:", str);
  gal_data_free(tmp);
  free(str);

  /* Find the mean and standard deviation, but don't print them, see
     explanations under median. */
  tmp=gal_statistics_mean_std(p->input);
  mean = ((double *)(tmp->array))[0];
  std  = ((double *)(tmp->array))[1];
  gal_data_free(tmp);

  /* Mode of the distribution (if it is valid). we want the mode and median
     to be found in place to save time/memory. But having a sorted array
     can decrease the floating point accuracy of the standard deviation. So
     we'll do the median calculation in the end.*/
  tmp=gal_statistics_mode(p->input, mirrdist, 1);
  d=tmp->array;
  if(d[2]>GAL_STATISTICS_MODE_GOOD_SYM)
    {        /* Same format as 'gal_data_write_to_string' */
      printf("  %-*s %.10g\n", namewidth, "Mode:", d[0]);
      printf("  %-*s %.10g\n", namewidth, "Mode quantile:", d[1]);
    }
  gal_data_free(tmp);

  /* Find and print the median:  */
  tmp=gal_statistics_median(p->input, 0);
  str=gal_type_to_string(tmp->array, tmp->type, 0);
  printf("  %-*s %s\n", namewidth, "Median:", str);
  gal_data_free(tmp);
  free(str);

  /* Print the mean and standard deviation. Same format as
     'gal_data_write_to_string' */
  printf("  %-*s %.10g\n", namewidth, "Mean:", mean);
  printf("  %-*s %.10g\n", namewidth, "Standard deviation:", std);

  /* Ascii histogram. Note that we don't want to force the user to have the
     plotting parameters. Also, when a reference column is defined, the
     range shown in the basic information section applies to that, not the
     range of the histogram. In that case, we want to print the histogram
     information. */
  printf("-------");
  range=set_bin_range_params(p, 1);
  p->asciiheight = p->asciiheight ? p->asciiheight : 10;
  p->numasciibins = p->numasciibins ? p->numasciibins : 70;
  bins=gal_statistics_regular_bins(p->input, range, p->numasciibins, NAN);
  hist=gal_statistics_histogram(p->input, bins, 0, 0);
  printf("\nHistogram:\n");
  print_ascii_plot(p, hist, bins, 1, 0);
  gal_data_free(bins);
  gal_data_free(hist);
  gal_data_free(range);
}




















/*******************************************************************/
/**************            Sigma clipping            ***************/
/*******************************************************************/
void
print_sigma_clip(struct statisticsparams *p)
{
  float *a;
  char *mode;
  int namewidth=40;
  gal_data_t *sigclip;

  /* Set the mode for printing: */
  if( p->sclipparams[1]>=1.0f )
    {
      if( asprintf(&mode, "for %g clips", p->sclipparams[1])<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
    }
  else
    {
      if( asprintf(&mode, "until relative change in STD is less than %g",
                   p->sclipparams[1])<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
    }

  /* Report the status */
  if(!p->cp.quiet)
    {
      print_input_info(p);
      printf("%g-sigma clipping steps %s:\n\n", p->sclipparams[0], mode);
    }

  /* Do the Sigma clipping: */
  sigclip=gal_statistics_sigma_clip(p->sorted, p->sclipparams[0],
                                    p->sclipparams[1], 0, p->cp.quiet);
  a=sigclip->array;

  /* Finish the introduction. */
  if(!p->cp.quiet)
    printf("-------\nSummary:\n");
  else
    printf("%g-sigma clipped %s:\n", p->sclipparams[0], mode);

  /* Print the final results: */
  printf("  %-*s %zu\n", namewidth, "Number of input elements:",
         p->input->size);
  if( p->sclipparams[1] < 1.0f )
    printf("  %-*s %d\n", namewidth, "Number of clips:",     sigclip->status);
  printf("  %-*s %.0f\n", namewidth, "Final number of elements:", a[0]);
  printf("  %-*s %g\n", namewidth, "Median:",                a[1]);
  printf("  %-*s %g\n", namewidth, "Mean:",                  a[2]);
  printf("  %-*s %g\n", namewidth, "Standard deviation:",    a[3]);

  /* Clean up. */
  free(mode);
}




















/*******************************************************************/
/**************                Fitting               ***************/
/*******************************************************************/
static struct gal_fits_list_key_t *
statistics_fit_params_to_keys(struct statisticsparams *p, gal_data_t *fit,
                              char *whtnat, double *redchisq)
{
  size_t i, j;
  char *kname, *kcomm;
  struct gal_fits_list_key_t *out=NULL;
  double *c=fit->array, *cov=fit->next?fit->next->array:NULL;

  /* Set the title and basic info (independent of the type of fit). */
  gal_fits_key_list_title_add(&out, "Fit results", 0);
  gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITTYPE", 0,
                        gal_fit_name_from_id(p->fitid), 0,
                        "Functional form of the fitting.", 0, NULL, 0);
  if( p->fitid==GAL_FIT_POLYNOMIAL
      || p->fitid==GAL_FIT_POLYNOMIAL_ROBUST
      || p->fitid==GAL_FIT_POLYNOMIAL_WEIGHTED )
    gal_fits_key_list_add(&out, GAL_TYPE_SIZE_T, "FITMAXP", 0,
                          &p->fitmaxpower, 0,
                          "Maximum power of polynomial.", 0, NULL, 0);
  if( p->fitid==GAL_FIT_POLYNOMIAL_ROBUST )
    gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITRTYP", 0,
                          p->fitrobustname, 0,
                          "Function for removing outliers", 0, NULL, 0);
  gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITIN", 0,
                        p->inputname, 0,"Name of file with input columns.",
                        0, NULL, 0);
  if(p->isfits)
    gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITINHDU", 0,
                          p->cp.hdu, 0,"Name or Number of HDU with input "
                          "columns.", 0, NULL, 0);
  gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITXCOL", 0,
                        p->columns->v, 0,"Name or Number of independent "
                        "(X) column.", 0, NULL, 0);
  gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITYCOL", 0,
                        p->columns->next->v, 0,"Name or Number of "
                        "measured (Y) column.", 0, NULL, 0);
  if(p->columns->next->next)
    {
      gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITWCOL", 0,
                            p->columns->next->next->v, 0,
                            "Name or Number of weight column.", 0, NULL, 0);
      gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITWNAT", 0,
                            whtnat, 0, "Nature of weight column.",
                            0, NULL, 0);
    }
  if(p->fitid==GAL_FIT_POLYNOMIAL_ROBUST)
    gal_fits_key_list_add(&out, GAL_TYPE_STRING, "FITROBST", 0,
                          p->fitrobustname, 0,
                          "Robust fitting (rejecting outliers) function.",
                          0, NULL, 0);
  gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FRDCHISQ", 0,
                        redchisq, 0, "Reduced chi^2 of fit.", 0, NULL, 0);

  /* Add the Fitting results. */
  switch(p->fitid)
    {
    /* Linear with no constant */
    case GAL_FIT_LINEAR_NO_CONSTANT:
    case GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED:
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FITC1", 0,
                            c, 0, "C1: Multiple of X in linear fit "
                            "(y=C1*x).", 0, NULL, 0);
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FCOV11", 0,
                            c+1, 0, "Variance of C1 (only element of "
                            "cov. matrix).", 0, NULL, 0);
      break;

    /* Basic linear. */
    case GAL_FIT_LINEAR:
    case GAL_FIT_LINEAR_WEIGHTED:
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FITC0", 0,
                            c, 0, "C0: Constant in linear fit "
                            "(y=C0+C1*x).", 0, NULL, 0);
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FITC1", 0,
                            c+1, 0, "C1: Multiple of X in linear fit "
                            "(y=C0+C1*x).", 0, NULL, 0);
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FCOV11", 0,
                            c+2, 0, "Covariance matrix element (1,1).",
                            0, NULL, 0);
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FCOV12", 0,
                            c+3, 0, "Covariance matrix element "
                            "(1,2)=(2,1).", 0, NULL, 0);
      gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, "FCOV22", 0,
                            c+4, 0, "Covariance matrix element (2,2).",
                            0, NULL, 0);
      break;

    /* Polynomial fit. */
    case GAL_FIT_POLYNOMIAL:
    case GAL_FIT_POLYNOMIAL_ROBUST:
    case GAL_FIT_POLYNOMIAL_WEIGHTED:
      for(i=0;i<fit->size;++i)
        {
          if( asprintf(&kname, "FITC%zu", i)<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf in FITCxx name",
                  __func__);
          if( asprintf(&kcomm, "C%zu: multiple of x^%zu in polynomial",
                       i, i)<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf in FITCxx comment",
                  __func__);
          gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, kname, 1,
                                c+i, 0, kcomm, 1, NULL, 0);
        }
      for(i=0;i<fit->size;++i)
        for(j=0;j<fit->size;++j)
          {
            if( asprintf(&kname, "FCOV%zu%zu", i+1, j+1)<0 )
              error(EXIT_FAILURE, 0, "%s: asprintf in FCOVxx name",
                    __func__);
            if( asprintf(&kcomm, "Covariance matrix element (%zu,%zu).",
                         i+1, j+1)<0 )
              error(EXIT_FAILURE, 0, "%s: asprintf in FCOVxx comment",
                    __func__);
            gal_fits_key_list_add(&out, GAL_TYPE_FLOAT64, kname, 1,
                                  cov+(i*fit->size+j), 0, kcomm, 1,
                                  NULL, 0);
          }
      break;

    /* Unrecognized FIT ID. */
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "fix the problem. The code '%d' isn't recognized for 'fitid'",
            __func__, PACKAGE_BUGREPORT, p->fitid);
    }

  /* Reverse the last-in-first-out list to be in the same logical order we
     inserted the items here. */
  gal_fits_key_list_reverse(&out);

  /* Return the key list. */
  return out;
}





static void
statistics_fit_estimate(struct statisticsparams *p, gal_data_t *fit,
                        char *whtnat, double *redchisq)
{
  gal_data_t *est=NULL;
  double *x, *y, *yerr;
  struct gal_fits_list_key_t *keys=NULL;

  /* If the input had no metadata, add them. */
  if(p->fitestval->name==NULL)
    gal_checkset_allocate_copy("X-INPUT", &p->fitestval->name);
  if(p->fitestval->comment==NULL)
    gal_checkset_allocate_copy("Requested values to estimate fit.",
                               &p->fitestval->comment);

  /* Estimations are done on a per-row level. */
  switch(p->fitid)
    {
    case GAL_FIT_LINEAR:
    case GAL_FIT_LINEAR_WEIGHTED:
    case GAL_FIT_LINEAR_NO_CONSTANT:
    case GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED:
      est=gal_fit_1d_linear_estimate(fit, p->fitestval);
      break;

    case GAL_FIT_POLYNOMIAL:
    case GAL_FIT_POLYNOMIAL_ROBUST:
    case GAL_FIT_POLYNOMIAL_WEIGHTED:
      est=gal_fit_1d_polynomial_estimate(fit, p->fitestval);
      break;

    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "fix the problem. The code '%d' isn't recognized for 'fitid'",
            __func__, PACKAGE_BUGREPORT, p->fitid);
    }

  /* Set the estimated columns to be after the input's columns. */
  p->fitestval->next=est;

  /* Non-quiet title. */
  if(p->cp.quiet==0)
    printf("\nRequested estimation:\n");

  /* If only one value was estimated and no column was given, then the
     user wanted the value on the command-line. */
  if(p->fitestimatecol)
    {
      if(p->cp.output)
        {
          if(p->cp.quiet==0)
            printf("  Written to: %s\n", p->cp.output);
        }
      keys=statistics_fit_params_to_keys(p, fit, whtnat, redchisq);
      gal_table_write(p->fitestval, &keys, NULL, p->cp.tableformat,
                      p->cp.output, "FIT_ESTIMATE", 0);
    }

  /* Print estimated value on the commandline. */
  else
    {
      x=p->fitestval->array;
      y=p->fitestval->next->array;
      yerr=p->fitestval->next->next->array;
      if(p->cp.quiet)
        printf("%f %f %f\n", x[0], y[0], yerr[0]);
      else
        printf("  X:         %f       (given on command-line)\n"
               "  Y:         %f\n"
               "  Y_error:   %f\n", x[0], y[0], yerr[0]);
    }

  /* Clean up. */
  gal_list_data_free(p->fitestval);
  p->fitestval=NULL; /* Was freed with 'out', avoid generic freeing later. */
}





static char *
statistics_fit_whtnat(struct statisticsparams *p)
{
  switch(p->fitwhtid)
    {
    case STATISTICS_FIT_WHT_STD:    return "Standard deviation";
    case STATISTICS_FIT_WHT_VAR:    return "Variance";
    case STATISTICS_FIT_WHT_INVVAR: return "Inverse variance";
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' "
            "to find and fix the problem. The value '%d' isn't a "
            "recognized weight type identifier", __func__,
            PACKAGE_BUGREPORT, p->fitwhtid);
    }

  /* Control should not reach here. */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' "
        "to find and fix the problem. Control should not reach the "
        "end of this function", __func__, PACKAGE_BUGREPORT);
  return NULL;
}





static char *
statistics_fit_print_intro(struct statisticsparams *p, char **whtnat)
{
  char *colspace;
  char *filename, *intro, *wcolstr=NULL;
  gal_list_str_t *xn=p->columns, *yn=xn->next?xn->next:NULL,
                 *wn=yn->next?yn->next:NULL;

  /* Set the full file name (for easy reading later!).*/
  filename=gal_fits_name_save_as_string(p->inputname, p->cp.hdu);

  /* Prepare string for nature of weight (if any weight was given!). */
  *whtnat = p->input->next->next ? statistics_fit_whtnat(p) : NULL;

  /* Set the Weight column string(s). */
  if(   p->fitid==GAL_FIT_LINEAR_WEIGHTED
     || p->fitid==GAL_FIT_POLYNOMIAL_WEIGHTED
     || p->fitid==GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED )
    {
      colspace="      ";
      if( asprintf(&wcolstr, "  Weight column: %s    "
                   "[%s of Y in each row]\n", wn->v, *whtnat)<0 )
        error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
    }
  else colspace=" ";

  /* Put everything into one string. */
  if( asprintf(&intro,
               "%s\n"
               "-------\n"
               "Fitting results (remove extra info with '--quiet' "
               "or '-q)\n"
               "  Input file:    %s with %zu non-blank rows.\n"
               "  X%scolumn: %s\n"
               "  Y%scolumn: %s\n"
               "%s",
               PROGRAM_STRING, filename, p->input->size,
               colspace, xn->v, colspace, yn->v,
               wcolstr ? wcolstr : "")<0 )
    error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);

  /* Clean up and return. */
  free(filename);
  free(wcolstr);
  return intro;
}





static int
statistics_fit_linear(struct statisticsparams *p)
{
  double *f, redchisq;
  gal_data_t *fit=NULL;
  char *intro, *funcvals, *whtnat=NULL;

  /* These have to be defined after each other. */
  gal_data_t *x=p->input;
  gal_data_t *y=x->next?x->next:NULL, *w=y->next?y->next:NULL;

  /* If the required number of columns aren't given,  */
  switch(p->fitid)
    {

    /* Linear fit. */
    case GAL_FIT_LINEAR:
      if(gal_list_data_number(p->input)!=2) return 2;
      fit=gal_fit_1d_linear(x, y, NULL);
      break;

    /* Linear (weighted). */
    case GAL_FIT_LINEAR_WEIGHTED:
      if(gal_list_data_number(p->input)!=3) return 3;
      fit=gal_fit_1d_linear(x, y, w);
      break;

    /* Linear (no constant) */
    case GAL_FIT_LINEAR_NO_CONSTANT:
      if(gal_list_data_number(p->input)!=2) return 2;
      fit=gal_fit_1d_linear_no_constant(x, y, NULL);
      break;

    /* Linear (no constant, weighted) */
    case GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED:
      if(gal_list_data_number(p->input)!=3) return 3;
      fit=gal_fit_1d_linear_no_constant(x, y, w);
      break;

    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "fix the problem. '%d' isn't recognized as a fitting ID",
            __func__, PACKAGE_BUGREPORT, p->fitid);
    }

  /* Print the output. */
  f=fit->array;
  if(p->cp.quiet)
    {
      if(p->fitestval) whtnat=statistics_fit_whtnat(p);
      else
        {
          /* The covariance matrix will only be present if a constant was
             present. After it, just print the residual (chi-squared or sum
             of squares of residuals). */
          if(   p->fitid==GAL_FIT_LINEAR
                || p->fitid==GAL_FIT_LINEAR_WEIGHTED )
            printf("%+-.10f %+-.10f\n"    /* The Two coefficients. */
                   "%+-20.10f %+-20.10f\n"/* First row of cov. matrix. */
                   "%+-20.10f %+-20.10f\n"/* Second row of cov. matrix. */
                   "%+-.10f\n",           /* Residual. */
                   f[0], f[1], f[2], f[3], f[3], f[4], f[5]);
          else
            printf("%+-.10f\n%+-.10f\n%+-.10f\n", f[0], f[1], f[2]);
        }
    }
  else
    {
      /* With or without constant. */
      if(    p->fitid==GAL_FIT_LINEAR
          || p->fitid==GAL_FIT_LINEAR_WEIGHTED )
        {
          redchisq=f[5];
          if( asprintf(&funcvals,
                       "Fit function: Y = c0 + (c1 * X)\n"
                       "  c0:  %+-.10f\n"
                       "  c1:  %+-.10f\n\n"
                       "Covariance matrix (off-diagonal are identical "
                       "same):\n"
                       "  %+-20.10f %+-20.10f\n"
                       "  %+-20.10f %+-20.10f\n", f[0], f[1], f[2], f[3],
                       f[3], f[4])<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
        }
      else
        {
          redchisq=f[2];
          if( asprintf(&funcvals,
                       "Fit function: Y = c1 * X\n"
                       "  c1: %+-.10f\n\n"
                       "Variance of 'c1':\n"
                       "  %+-.10f\n", f[0], f[1])<0 )
            error(EXIT_FAILURE, 0, "%s: asprintf allocation", __func__);
        }

      /* Statistics version, and input filenames. */
      intro=statistics_fit_print_intro(p, &whtnat);

      /* Final printed report.*/
      printf("%s\n%s\nReduced chi^2 of fit:\n  %+f\n", intro,
             funcvals, redchisq);

      /* Clean up. */
      free(intro);
      free(funcvals);
    }

  /* Estimate values (if requested), note that it involves writing the
     fitted parameters in the header. */
  if(p->fitestval)
    statistics_fit_estimate(p, fit, whtnat, fit->size==6?f+5:f+2);

  /* Clean up. */
  gal_data_free(fit);
  return 0; /* Means everything is good! */
}





static void
statistics_fit_polynomial_print(struct statisticsparams *p, gal_data_t *fit,
                                double redchisq, char **whtnat)
{
  size_t i, j;
  char *intro;
  size_t nconst=p->fitmaxpower+1;
  double *farr=fit->array, *carr=fit->next->array;

  /* Print fitted constants */
  if(p->cp.quiet)
    {
      if(p->fitestval==NULL)
        {
          for(i=0;i<nconst;++i) printf("%+-.10f ", farr[i]);
          printf("\b\n");
        }
    }
  else
    {
      /* Statistics version, and input filenames. */
      intro=statistics_fit_print_intro(p, whtnat);

      /* Final printed report.*/
      printf("%s\n"
             "Fit function: Y = c0 + (c1 * X^1) + (c2 * X^2) "
             "+ ... (cN * X^N)\n", intro);

      /* Notice for the (possible) robust function and maximum power of
         polynomial. */
      if(p->fitid==GAL_FIT_POLYNOMIAL_ROBUST)
        printf("  Robust function: %s\n", p->fitrobustname);
      printf("  N:  %zu\n", p->fitmaxpower);

      /* Print the fitted values. */
      for(i=0;i<nconst;++i)
        printf("  c%zu: %s%+-.10f\n", i, i<10?" ":"", farr[i]);

      /* Print the information on the covariance matrix. */
      printf("\nCovariance matrix:\n");

      /* Clean up. */
      free(intro);
    }

  /* Print the covariance matrix (when no estimation is requested, this is
     the same for quiet or non-quiet mode). But when estimation is
     requested, they will be written in the FITS keywords of the output so
     there is no more need to have them on the command-line.*/
  if(p->cp.quiet==0 || p->fitestval==NULL)
    {
      for(i=0;i<nconst;++i)
        {
          if(p->cp.quiet==0) printf("  ");
          for(j=0;j<nconst;++j)
            printf("%+-20.10f ", carr[i*nconst+j]);
          printf("\b\n");
        }

      /* Print the chi^2. */
      if(p->cp.quiet==0)
        printf("\nReduced chi^2 of fit:\n");
      printf("%s%+-.10f\n", p->cp.quiet?"":"  ", redchisq);
    }
}





static int
statistics_fit_polynomial(struct statisticsparams *p)
{
  char *whtnat;
  gal_data_t *fit=NULL;
  double redchisq=NAN; /* Important to initialize. */
  gal_data_t *x=p->input, *y=x->next?x->next:NULL, *w=y->next?y->next:NULL;

  /* Sanity check and call the fitting functions. */
  switch(p->fitid)
    {
    case GAL_FIT_POLYNOMIAL:
      if(gal_list_data_number(p->input)!=2) return 2; /* Sanity check. */
      fit=gal_fit_1d_polynomial(x, y, NULL, p->fitmaxpower, &redchisq);
      break;

    case GAL_FIT_POLYNOMIAL_ROBUST:
      if(gal_list_data_number(p->input)!=2) return 2; /* Sanity check. */
      fit=gal_fit_1d_polynomial_robust(x, y, p->fitmaxpower,
                                       p->fitrobustid, &redchisq);
      break;

    case GAL_FIT_POLYNOMIAL_WEIGHTED:
      if(gal_list_data_number(p->input)!=3) return 3; /* Sanity check. */
      fit=gal_fit_1d_polynomial(x, y, w, p->fitmaxpower, &redchisq);
      break;

    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "fix the problem. '%d' isn't recognized as a fitting ID",
            __func__, PACKAGE_BUGREPORT, p->fitid);
    }

  /* Print the output. */
  statistics_fit_polynomial_print(p, fit, redchisq, &whtnat);

  /* Estimate values (if requested), note that it involves writing the
     fitted parameters in the header. */
  if(p->fitestval)
    statistics_fit_estimate(p, fit, whtnat, &redchisq);

  /* Clean up. */
  gal_data_free(fit);
  gal_list_data_free(p->fitestval); p->fitestval=NULL;
  return 0; /* Means everything is good! */
}





static void
statistics_fit(struct statisticsparams *p)
{
  size_t neededcols=0;

  /* Make sure that at least two columns are provided. */
  if(p->input->next==NULL)
    error(EXIT_FAILURE, 0, "at least two columns are necessary for "
          "the fitting operations");

  /* Do the fitting. */
  switch(p->fitid)
    {
    case GAL_FIT_LINEAR:
    case GAL_FIT_LINEAR_WEIGHTED:
    case GAL_FIT_LINEAR_NO_CONSTANT:
    case GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED:
      neededcols=statistics_fit_linear(p);
      break;

    case GAL_FIT_POLYNOMIAL:
    case GAL_FIT_POLYNOMIAL_ROBUST:
    case GAL_FIT_POLYNOMIAL_WEIGHTED:
      neededcols=statistics_fit_polynomial(p);
      break;

    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
            "fix the problem. '%s' is not a recognized as a fit type",
            __func__, PACKAGE_BUGREPORT, p->fitname);
    }

  /* If the number of columns is not sufficient, then 'neededcols' will be
     non-zero. In this case, we should abort and inform the user. */
  if(neededcols)
    error(EXIT_FAILURE, 0, "'%s' fitting requires %zu columns as input, "
          "but %zu columns have been given", p->fitname, neededcols,
          gal_list_data_number(p->input));
}
















/*******************************************************************/
/**************             Main function            ***************/
/*******************************************************************/
void
statistics(struct statisticsparams *p)
{
  int print_basic_info=1;

  /* Print the one-row numbers if the user asked for them. */
  if(p->singlevalue)
    {
      print_basic_info=0;
      if(p->ontile) statistics_on_tile(p);
      else          statistics_print_one_row(p);
    }

  /* Find the Sky value if called. */
  if(p->sky)
    {
      sky(p);
      print_basic_info=0;
    }

  if(p->contour)
    {
      contour(p);
      print_basic_info=0;
    }

  /* Print the ASCII plots if requested. */
  if(p->asciihist || p->asciicfp)
    {
      ascii_plots(p);
      print_basic_info=0;
    }

  /* Save the histogram and CFP as tables if requested. */
  if(p->histogram || p->cumulative)
    {
      print_basic_info=0;
      save_hist_and_or_cfp(p);
    }

  /* 2D histogram. */
  if(p->histogram2d)
    {
      print_basic_info=0;
      histogram_2d(p);
    }

  /* Print the sigma-clipped results. */
  if( p->sigmaclip )
    {
      print_basic_info=0;
      print_sigma_clip(p);
    }

  /* Make the mirror table. */
  if( !isnan(p->mirror) )
    {
      print_basic_info=0;
      print_mirror_hist_cfp(p);
    }

  /* Fitting. */
  if( p->fitname )
    {
      print_basic_info=0;
      statistics_fit(p);
    }

  /* If nothing was requested print the simple statistics. */
  if(print_basic_info)
    print_basics(p);
}
