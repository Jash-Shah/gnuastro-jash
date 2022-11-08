/*********************************************************************
Cosmological calculations.
This is part of GNU Astronomy Utilities (Gnuastro) package.

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
#ifndef __GAL_COSMOLOGY_H__
#define __GAL_COSMOLOGY_H__

/* Include other headers if necessary here. Note that other header files
   must be included before the C++ preparations below */
#include <gnuastro/error.h>

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





/* Error codes. */
/* olambda not between 0 and 1. */
#define GAL_COSMOLOGY_ERROR_LAMBDA_OUT_OF_BOUNDS    ERROR_BITSET(1, 0)
/* omatter not between 0 and 1. */
#define GAL_COSMOLOGY_ERROR_MATTER_OUT_OF_BOUNDS    ERROR_BITSET(2, 0)
/* oradiation not between 0 and 1. */
#define GAL_COSMOLOGY_ERROR_RADIATION_OUT_OF_BOUNDS ERROR_BITSET(3, 0)
/* Sum of fractional densities not 1. */
#define GAL_COSMOLOGY_ERROR_SUM_LIMIT               ERROR_BITSET(4, 0)
   
/* Given the error `code` and the `is_warning` flag, returns the value
   whose first 16 bits represents the `is_warning` flag and last 16 bits
   represent the `code`. */
#define ERROR_BITSET(code, is_warning) ((code << 16) | is_warning)





/* Age of the universe (in Gyrs). */
double
gal_cosmology_age(double z, double H0, double o_lambda_0,
                  double o_matter_0, double o_radiation_0,
                  gal_error_t **err);

/* Proper distance to z (Mpc). */
double
gal_cosmology_proper_distance(double z, double H0, double o_lambda_0,
                              double o_matter_0, double o_radiation_0,
                              gal_error_t **err);

/* Comoving volume over 4pi stradian to z (Mpc^3). */
double
gal_cosmology_comoving_volume(double z, double H0, double o_lambda_0,
                              double o_matter_0, double o_radiation_0,
                              gal_error_t **err);

/* Critical density at redshift z in units of g/cm^3. */
double
gal_cosmology_critical_density(double z, double H0, double o_lambda_0,
                               double o_matter_0, double o_radiation_0,
                               gal_error_t **err);

/* Angular diameter distance to z (Mpc). */
double
gal_cosmology_angular_distance(double z, double H0, double o_lambda_0,
                               double o_matter_0, double o_radiation_0,
                               gal_error_t **err);

/* Luminosity distance to z (Mpc). */
double
gal_cosmology_luminosity_distance(double z, double H0, double o_lambda_0,
                                  double o_matter_0, double o_radiation_0,
                                  gal_error_t **err);

/* Distance modulus at z (no units). */
double
gal_cosmology_distance_modulus(double z, double H0, double o_lambda_0,
                               double o_matter_0, double o_radiation_0,
                               gal_error_t **err);

/* Convert apparent to absolute magnitude. */
double
gal_cosmology_to_absolute_mag(double z, double H0, double o_lambda_0,
                              double o_matter_0, double o_radiation_0,
                              gal_error_t **err);

double
gal_cosmology_velocity_from_z(double z);

double
gal_cosmology_z_from_velocity(double v);

__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_COSMOLOGY_H__ */
