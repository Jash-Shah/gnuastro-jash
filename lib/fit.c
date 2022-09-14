/*********************************************************************
Functions for parametric fitting.
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
#include <string.h>
#include <stdlib.h>

#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit.h>

#include <gnuastro/fit.h>
#include <gnuastro/blank.h>
#include <gnuastro/pointer.h>

#include <gnuastro-internal/checkset.h>





/**********************************************************************/
/****************              Identifiers             ****************/
/**********************************************************************/

/* Read the desired parameters. */
uint8_t
gal_fit_name_to_id(char *name)
{
  if( !strcmp(name, "linear") )
    return GAL_FIT_LINEAR;
  else if( !strcmp(name, "linear-weighted") )
    return GAL_FIT_LINEAR_WEIGHTED;
  else if( !strcmp(name, "linear-no-constant") )
    return GAL_FIT_LINEAR_NO_CONSTANT;
  else if( !strcmp(name, "linear-no-constant-weighted") )
    return GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED;
  else if( !strcmp(name, "polynomial-weighted") )
    return GAL_FIT_POLYNOMIAL_WEIGHTED;
  else if( !strcmp(name, "polynomial") )
    return GAL_FIT_POLYNOMIAL;
  else if( !strcmp(name, "polynomial-robust") )
    return GAL_FIT_POLYNOMIAL_ROBUST;
  else return GAL_FIT_INVALID;

  /* If control reaches here, there was a bug! */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
        "find a fix it. Control should not have reached here",
        __func__, PACKAGE_BUGREPORT);
  return GAL_FIT_INVALID;
}





char *
gal_fit_name_from_id(uint8_t fitid)
{
  /* Prepare the temporary array. */
  switch(fitid)
    {
    case GAL_FIT_LINEAR:              return "linear";
    case GAL_FIT_LINEAR_WEIGHTED:     return "linear-weighted";
    case GAL_FIT_LINEAR_NO_CONSTANT:  return "linear-no-constant";
    case GAL_FIT_POLYNOMIAL:          return "polynomial";
    case GAL_FIT_POLYNOMIAL_WEIGHTED: return "polynomial-weighted";
    case GAL_FIT_POLYNOMIAL_ROBUST:   return "polynomial-robust";
    case GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED:
      return "linear-no-constant-weighted";
    default: return NULL;
    }

  /* If control reaches here, there was a bug! */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
        "find a fix it. Control should not have reached here",
        __func__, PACKAGE_BUGREPORT);
  return NULL;
}





int
gal_fit_name_robust_to_id(char *name)
{
  /* In case 'name' is NULL, then return the invalid type.  */
  if(name==NULL) return GAL_FIT_ROBUST_INVALID;

  /* Match the name. */
  if(      !strcmp(name, "bisquare") ) return GAL_FIT_ROBUST_BISQUARE;
  else if( !strcmp(name, "cauchy")   ) return GAL_FIT_ROBUST_CAUCHY;
  else if( !strcmp(name, "fair")     ) return GAL_FIT_ROBUST_FAIR;
  else if( !strcmp(name, "huber")    ) return GAL_FIT_ROBUST_HUBER;
  else if( !strcmp(name, "ols")      ) return GAL_FIT_ROBUST_OLS;
  else if( !strcmp(name, "welsch")   ) return GAL_FIT_ROBUST_WELSCH;
  else                                 return GAL_FIT_ROBUST_INVALID;

  /* If control reaches here, there was a bug! */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
        "find a fix it. Control should not have reached this point",
        __func__, PACKAGE_BUGREPORT);
  return GAL_FIT_ROBUST_INVALID;
}





char *
gal_fit_name_robust_from_id(uint8_t robustid)
{
  switch(robustid)
    {
    case GAL_FIT_ROBUST_BISQUARE:   return "bisquare";
    case GAL_FIT_ROBUST_CAUCHY:     return "cauchy";
    case GAL_FIT_ROBUST_FAIR:       return "fair";
    case GAL_FIT_ROBUST_HUBER:      return "huber";
    case GAL_FIT_ROBUST_OLS:        return "ols";
    case GAL_FIT_ROBUST_WELSCH:     return "welsch";
    default:                        return NULL;
    }

  /* If control reaches here, there was a bug! */
  error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' to "
        "find a fix it. Control should not have reached this point",
        __func__, PACKAGE_BUGREPORT);
  return NULL;
}




















/**********************************************************************/
/****************            Common to all             ****************/
/**********************************************************************/
static gal_data_t *
fit_1d_sanity_check(gal_data_t *in, gal_data_t *ref, const char *func)
{
  gal_data_t *out;

  /* Make sure the input is 1-dimensional. */
  if(in->ndim!=1)
    error(EXIT_FAILURE, 0, "%s: inputs must have one dimension", func);

  /* Make sure the input has the same size as the reference. */
  if(in->size != ref->size)
    error(EXIT_FAILURE, 0, "%s: all inputs must have the same size",
          func);

  /* Make sure output has a double type. */
  out = ( in->type==GAL_TYPE_FLOAT64
          ? in
          : gal_data_copy_to_new_type(in, GAL_TYPE_FLOAT64) );

  /* If there are blank values, print a warning, then return. */
  if(gal_blank_present(out, 1))
    error(EXIT_SUCCESS, 0, "%s: at least one of the input columns "
          "have a blank value; the fit will become NaN. Within the "
          "Gnuastro, you can use 'gal_blank_remove_rows' to remove "
          "all rows that have at least one blank value in any column",
          func);
  return out;
}




















/**********************************************************************/
/****************              Linear fit              ****************/
/**********************************************************************/
static gal_data_t *
fit_1d_linear_base(gal_data_t *xin, gal_data_t *yin,
                   gal_data_t *ywht, int fitid)
{
  double *o, nparam=NAN;
  size_t osize, chisqind=GAL_BLANK_SIZE_T;
  gal_data_t *x=NULL, *y=NULL, *w=NULL, *out;

  /* Basic sanity checks. */
  x=fit_1d_sanity_check(xin, xin, __func__);
  y=fit_1d_sanity_check(yin, xin, __func__);
  if(ywht) w=fit_1d_sanity_check(ywht, xin, __func__);

  /* Allocate the output dataset. */
  osize = ( fitid==GAL_FIT_LINEAR || fitid==GAL_FIT_LINEAR_WEIGHTED
            ? 6 : 3 );
  out=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &osize, NULL, 0,
                     -1, 1, NULL, NULL, NULL);

  /* For a check.
  {
    size_t i;
    double *xa=x->array, *ya=y->array;
    for(i=0;i<x->size;++i)
      printf("%-15f %-15f\n", xa[i], ya[i]);
  } */

  /* Do the fitting. */
  o=out->array;
  switch(fitid)
    {
    case GAL_FIT_LINEAR:
      nparam=2;
      chisqind=5;
      gsl_fit_linear(x->array, 1, y->array, 1, x->size, o, o+1,
                     o+2, o+3, o+4, o+5);
      break;
    case GAL_FIT_LINEAR_WEIGHTED:
      nparam=2;
      chisqind=5;
      gsl_fit_wlinear(x->array, 1, w->array, 1, y->array, 1, x->size,
                      o, o+1, o+2, o+3, o+4, o+5);
      break;
    case GAL_FIT_LINEAR_NO_CONSTANT:
      nparam=1;
      chisqind=2;
      gsl_fit_mul(x->array, 1, y->array, 1, x->size, o, o+1, o+2);
      break;
    case GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED:
      nparam=1;
      chisqind=2;
      gsl_fit_wmul(x->array, 1, w->array, 1, y->array, 1, x->size,
                   o, o+1, o+2);
      break;
    default:
      error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at '%s' "
            "to fix the problem. The fitting id '%d' isn't recognized",
            __func__, PACKAGE_BUGREPORT, fitid);
    }

  /* For a check.
  {
    printf("c0: %f\nc1: %f\n"
           "cov00: %f\ncov01: %f\ncov11: %f\nsumsq: %f\n",
           o[0], o[1], o[2], o[3], o[4], o[5]);
  } */

  /* Calculate the reduced chi^2: As mentioned in [1], in case we have the
     chi^2, then it is simply the chi^2 divided by the degrees of
     freedom. But GSL only returns the chi^2 for weighted fits. Therefore,
     according to [1], we can also use the residual sum of squares instead.

     The number of degrees of freedom is defined by the number of
     observations subtracted from the number of fitted parameters.

     [1] https://en.wikipedia.org/wiki/Reduced_chi-squared_statistic */
  o[chisqind] /= (x->size - nparam);

  /* Clean up and return. */
  if(x!=xin) gal_data_free(x);
  if(y!=yin) gal_data_free(y);
  if(ywht && w!=ywht) gal_data_free(w);
  return out;
}





gal_data_t *
gal_fit_1d_linear(gal_data_t *xin, gal_data_t *yin, gal_data_t *ywht)
{
  return fit_1d_linear_base(xin, yin, ywht,
                            ( ywht
                              ? GAL_FIT_LINEAR_WEIGHTED
                              : GAL_FIT_LINEAR));
}





gal_data_t *
gal_fit_1d_linear_no_constant(gal_data_t *xin, gal_data_t *yin,
                              gal_data_t *ywht)
{
  return fit_1d_linear_base(xin, yin, ywht,
                            ( ywht
                              ? GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED
                              : GAL_FIT_LINEAR_NO_CONSTANT) );
}





gal_data_t *
fit_1d_estimate_prepare(gal_data_t *xin, gal_data_t *fit, gal_data_t **xd,
                        const char *func)
{
  gal_data_t *out=NULL;

  /* The Fit arrays should be double precision. */
  if(fit->type!=GAL_TYPE_FLOAT64
     || (fit->next && fit->next->type!=GAL_TYPE_FLOAT64) )
    error(EXIT_FAILURE, 0, "%s: the 'fit' argument should only "
          "contain double precision floating point types", func);
  if(fit->ndim!=1 || (fit->next && fit->next->ndim!=2) )
    error(EXIT_FAILURE, 0, "%s: the 'fit' argument should only "
          "contain single-dimensional outputs", func);
  if(fit->next && (fit->next->dsize[0]!=fit->next->dsize[1]))
    error(EXIT_FAILURE, 0, "%s: the secont dataset of the 'fit' "
          "argument should be square (same size in both "
          "dimensions)", func);

  /* Make sure the input X values are in double precision. */
  *xd = ( xin->type==GAL_TYPE_FLOAT64
          ? xin
          : gal_data_copy_to_new_type(xin, GAL_TYPE_FLOAT64) );

  /* Allocate the output datasets. */
  gal_list_data_add_alloc(&out, NULL, GAL_TYPE_FLOAT64, 1, xin->dsize,
                          NULL, 1, xin->minmapsize, xin->quietmmap,
                          "Y-ESTIMATED", xin->unit,
                          "Estimated value after fitting.");
  gal_list_data_add_alloc(&out, NULL, GAL_TYPE_FLOAT64, 1, xin->dsize,
                          NULL, 1, xin->minmapsize, xin->quietmmap,
                          "Y-ESTIMATED-ERR", xin->unit,
                          "Estimated error on value after fitting.");
  gal_list_data_reverse(&out);

  /* Return the output. */
  return out;
}





gal_data_t *
gal_fit_1d_linear_estimate(gal_data_t *fit, gal_data_t *xin)
{
  size_t i;
  gal_data_t *out=NULL, *xd;
  double *x, *y, *yerr, *f=fit->array;

  /* Do the basic preparations. */
  out=fit_1d_estimate_prepare(xin, fit, &xd, __func__);

  /* Set the pointers. */
  x    = xd->array;
  y    = out->array;
  yerr = out->next->array;

  /* Estimate the values. */
  switch(fit->size)
    {
    case 6:                     /* Linear with constant. */
      for(i=0;i<out->size;++i)
        gsl_fit_linear_est(x[i], f[0], f[1], f[2], f[3], f[4],
                           y+i, yerr+i);
      break;

    case 3:                     /* Linear WITHOUT constant. */
      for(i=0;i<out->size;++i)
        gsl_fit_mul_est(x[i], f[0], f[1], y+i, yerr+i);
      break;

    default:                    /* Un-recognized situation! */
      error(EXIT_FAILURE, 0, "%s: the 'fit' argument should "
            "either have 6 or 3 elements (be an output of "
            "'gal_fit_1d_linear' or 'gal_fit_1d_linear_no_constant'"
            "respectively), but it has %zu elements", __func__,
            fit->size);
    }

  /* Clean up. */
  if(xd!=xin) gal_data_free(xd);
  return out;
}





static void
fit_1d_polynomial_prepare(gal_data_t *xin,  gal_data_t *yin,
                          gal_data_t *ywht, int nconst,
                          gsl_matrix **x,   gsl_vector **c,
                          gsl_matrix **cov, gsl_vector *y,
                          gsl_vector *w)
{
  size_t i, j;
  double *xo, *xi;

  /* Use GSL's own matrix allocation functions for the structures that need
     allocation and we can't use the same allocated space of the inputs. */
  *c   = gsl_vector_alloc(nconst);
  *cov = gsl_matrix_alloc(nconst, nconst);
  *x   = gsl_matrix_alloc(xin->size, nconst);

  /* Fill in the X matrix. */
  xi=xin->array;
  xo=(*x)->data;
  for(i=0;i<xin->size;++i)
    {
      /* The first column (constant) doesn't depend on X. So we'll give it
         a value of 1.0. */
      xo[ i*nconst ] = 1.0f;

      /* Column i is the multiplication of column i-1 with the input
         horizontal value. This will make it a polynomial. */
      for(j=1;j<nconst;++j)
        xo[ i*nconst + j ] = xo[ i*nconst + j-1 ] * xi[i];
    }

  /* For a check.
  {
    size_t checki=5;
    printf("Row %zu: ", checki);
    for(j=0;j<maxpower;++j)
      printf("%.3f ", xo[ checki*maxpower + j ]);
    printf("\n");
    exit(0);
  } */

  /* Set the pointers of the 'y' and 'w' GSL vectors. */
  y->data=yin->array;
  if(ywht) w->data=ywht->array;
}





gal_data_t *
gal_fit_1d_polynomial_base(gal_data_t *xin, gal_data_t *yin,
                           gal_data_t *ywht, size_t maxpower,
                           uint8_t robustid, double *redchisq)
{
  /* Low-level variable. */
  size_t nconst=maxpower+1;

  /* Other variables */
  gsl_vector *c=NULL;
  double chisq=NAN, sse=NAN;
  gsl_matrix *x=NULL, *cov=NULL;
  size_t covsize[2]={nconst, nconst};
  gsl_multifit_linear_workspace *work_n;
  gsl_multifit_robust_workspace *work_r;
  const gsl_multifit_robust_type *rtype=NULL;
  gal_data_t *xdata, *ydata, *wdata, *tmp, *out=NULL;

  /* For the 'y' and 'w' GSL vectors, we don't actually need to allocate
     any space, we can just use the allocated space within the
     'gal_data_t'. We can't set the pointers now because we aren't sure
     they have 'double' type yet. */
  gsl_vector yvec={yin->size, 1, NULL, NULL, 0}; /* Both have same size, */
  gsl_vector wvec={yin->size, 1, NULL, NULL, 0}; /* but ywht may be NULL!*/
  gsl_vector *y=&yvec, *w=&wvec; /* These have to be after the two above.*/

  /* Basic sanity checks. */
  xdata =        fit_1d_sanity_check(xin,  xin, __func__);
  ydata =        fit_1d_sanity_check(yin,  xin, __func__);
  wdata = ywht ? fit_1d_sanity_check(ywht, xin, __func__) : NULL;

  /* Fill all the GSL structures. */
  fit_1d_polynomial_prepare(xdata, ydata, wdata, nconst,
                            &x, &c, &cov, y, w);

  /* Do the fit (depending on if it is robust or not. */
  if(robustid==GAL_FIT_ROBUST_INVALID)
    {
      work_n = gsl_multifit_linear_alloc(xin->size, nconst);
      if(ywht) gsl_multifit_wlinear(x, w, y, c, cov, &chisq, work_n);
      else     gsl_multifit_linear( x,    y, c, cov, &sse,   work_n);
      gsl_multifit_linear_free(work_n);
    }
  else
    {
      /* Select the robust function type. */
      switch(robustid)
        {
        case GAL_FIT_ROBUST_BISQUARE: rtype=gsl_multifit_robust_bisquare;
          break;
        case GAL_FIT_ROBUST_CAUCHY:   rtype=gsl_multifit_robust_cauchy;
          break;
        case GAL_FIT_ROBUST_FAIR:     rtype=gsl_multifit_robust_fair;
          break;
        case GAL_FIT_ROBUST_HUBER:    rtype=gsl_multifit_robust_huber;
          break;
        case GAL_FIT_ROBUST_OLS:      rtype=gsl_multifit_robust_ols;
          break;
        case GAL_FIT_ROBUST_WELSCH:   rtype=gsl_multifit_robust_welsch;
          break;
        default:
          error(EXIT_FAILURE, 0, "%s: a bug! Please contact us at "
                "'%s' to fix the problem. the 'robustid' value '%d' "
                "isn't recognize", __func__, PACKAGE_BUGREPORT, robustid);
        }

      /* Initialize the worker and do the fit (depending on if a weight
         image was provided). */
      work_r=gsl_multifit_robust_alloc(rtype, x->size1, x->size2);
      gsl_multifit_robust(x, y, c, cov, work_r);

      /* Get the residual sum of squares and free the worker. */
      sse=gsl_multifit_robust_statistics(work_r).sse;
      gsl_multifit_robust_free(work_r);
    }

  /* For a check:
  {
    size_t i;
    double *ca=c->data;
    for(i=0;i<=nconst;++i) { printf("%f ", ca[i]); } printf("\n");
  } */

  /* Allocate the output dataset containing the fit results as first
     'gal_data_t'. */
  tmp=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 1, &nconst, NULL, 0,
                     xin->minmapsize, xin->quietmmap, NULL, NULL,
                     NULL);
  memcpy(tmp->array, c->data, nconst*sizeof c->data);
  out=tmp;

  /* Allocate the second element of the output (the covariance matrix). */
  tmp=gal_data_alloc(NULL, GAL_TYPE_FLOAT64, 2, covsize, NULL, 0,
                     xin->minmapsize, xin->quietmmap, NULL, NULL,
                     NULL);
  memcpy(tmp->array, cov->data, nconst*nconst*sizeof cov->data);
  out->next=tmp;

  /* Calculate the reduced chi^2, see the description of same step in
     'fit_1d_linear_base'. */
  *redchisq = (isnan(chisq) ? sse : chisq) / (xdata->size-nconst);

  /* Clean up and return. */
  gsl_matrix_free(x);
  gsl_vector_free(c);
  gsl_matrix_free(cov);
  if(xdata!=xin) gal_data_free(xdata);
  if(ydata!=yin) gal_data_free(ydata);
  if(ywht && wdata!=ywht) gal_data_free(wdata);
  return out;
}





gal_data_t *
gal_fit_1d_polynomial(gal_data_t *xin, gal_data_t *yin,
                      gal_data_t *ywht, size_t maxpower,
                      double *redchisq)
{
  return gal_fit_1d_polynomial_base(xin, yin, ywht, maxpower,
                                    GAL_FIT_ROBUST_INVALID,
                                    redchisq);
}





gal_data_t *
gal_fit_1d_polynomial_robust(gal_data_t *xin, gal_data_t *yin,
                             size_t maxpower, uint8_t robustid,
                             double *redchisq)
{
  /* Robust fitting doesn't use weights (the functions are effectively the
     weight). */
  return gal_fit_1d_polynomial_base(xin, yin, NULL, maxpower,
                                    robustid, redchisq);
}





/* Estimate values from a polynomial fit.*/
gal_data_t *
gal_fit_1d_polynomial_estimate(gal_data_t *fit, gal_data_t *xin)
{
  size_t i, j;
  size_t nconst=fit->size;
  gal_data_t *xd, *out=NULL;
  double *y, *xi, *xo, *yerr;

  /* We don't need to allocate space for the GSL vectors and matrices, we
     can just use the allocated space within the 'gal_data_t'. We can't set
     the pointers now because we aren't sure they have 'double' type
     yet. */
  gsl_vector xvec={nconst, 1, NULL, NULL, 0};
  gsl_vector cvec={nconst, 1, NULL, NULL, 0};
  gsl_matrix cmat={nconst, nconst, nconst, NULL, NULL, 0};

  /* Do the basic preparations. */
  out=fit_1d_estimate_prepare(xin, fit, &xd, __func__);

  /* Set the pointers. */
  xo = xvec.data = gal_pointer_allocate(GAL_TYPE_FLOAT64, nconst,
                                        0, __func__, "xvec.data");
  xi        = xd->array;
  cvec.data = fit->array;
  y         = out->array;
  yerr      = out->next->array;
  cmat.data = fit->next->array;

  /* Do the estimation. */
  for(i=0;i<xd->size;++i)
    {
      xo[0]=1.0f; for(j=1;j<nconst;++j) xo[j] = xo[j-1] * xi[i];
      gsl_multifit_linear_est(&xvec, &cvec, &cmat, y+i, yerr+i);
    }

  /* Clean up and return. */
  if(xd!=xin) gal_data_free(xd);
  free(xvec.data);
  return out;
}
