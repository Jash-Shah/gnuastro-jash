/*********************************************************************
fit -- functions for doing fitting to input datasets.
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
#ifndef __GAL_FIT_H__
#define __GAL_FIT_H__


/* Include other headers if necessary here. Note that other header files
   must be included before the C++ preparations below */
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




/* Definitions. */
enum gal_fit_types
{
  GAL_FIT_INVALID,   /* Invalid (=0 by C standard).  */
  GAL_FIT_LINEAR,
  GAL_FIT_LINEAR_WEIGHTED,
  GAL_FIT_LINEAR_NO_CONSTANT,
  GAL_FIT_LINEAR_NO_CONSTANT_WEIGHTED,
  GAL_FIT_POLYNOMIAL,
  GAL_FIT_POLYNOMIAL_ROBUST,
  GAL_FIT_POLYNOMIAL_WEIGHTED,

  /* This will be the total number of shapes (good for scripts). */
  GAL_FIT_NUMBER
};

enum gal_fit_robust_types
{
  GAL_FIT_ROBUST_INVALID,   /* Invalid (=0 by C standard).  */
  GAL_FIT_ROBUST_BISQUARE,
  GAL_FIT_ROBUST_CAUCHY,
  GAL_FIT_ROBUST_FAIR,
  GAL_FIT_ROBUST_HUBER,
  GAL_FIT_ROBUST_OLS,
  GAL_FIT_ROBUST_WELSCH,

  /* This will be the total number of shapes (good for scripts). */
  GAL_FIT_ROBUST_NUMBER
};



/* Functions */
uint8_t
gal_fit_name_to_id(char *name);

char *
gal_fit_name_from_id(uint8_t fitid);

int
gal_fit_name_robust_to_id(char *name);

char *
gal_fit_name_robust_from_id(uint8_t robustid);

gal_data_t *
gal_fit_1d_linear(gal_data_t *xin, gal_data_t *yin,
                  gal_data_t *ywht);

gal_data_t *
gal_fit_1d_linear_no_constant(gal_data_t *xin, gal_data_t *yin,
                              gal_data_t *ywht);

gal_data_t *
gal_fit_1d_linear_estimate(gal_data_t *fit, gal_data_t *xin);

gal_data_t *
gal_fit_1d_polynomial(gal_data_t *xin, gal_data_t *yin,
                      gal_data_t *ywht, size_t maxpower,
                      double *redchisq);

gal_data_t *
gal_fit_1d_polynomial_robust(gal_data_t *xin, gal_data_t *yin,
                             size_t maxpower, uint8_t robustid,
                             double *redchisq);

gal_data_t *
gal_fit_1d_polynomial_estimate(gal_data_t *fit, gal_data_t *xin);

__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_FIT_H__ */
