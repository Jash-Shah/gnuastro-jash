/*********************************************************************
Units -- Convert data from one unit to other.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Kartik Ohri <kartikohri13@gmail.com>
Contributing author(s):
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Copyright (C) 2020-2022 Free Software Foundation, Inc.

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
#ifndef __GAL_UNITS_H__
#define __GAL_UNITS_H__

/* When we are within Gnuastro's building process, 'IN_GNUASTRO_BUILD' is
   defined. In the build process, installation information (in particular
   'GAL_CONFIG_SIZEOF_SIZE_T' that we need below) is kept in
   'config.h'. When building a user's programs, this information is kept in
   'gnuastro/config.h'. Note that all '.c' files in Gnuastro's source must
   start with the inclusion of 'config.h' and that 'gnuastro/config.h' is
   only created at installation time (not present during the building of
   Gnuastro). */
#ifndef IN_GNUASTRO_BUILD
#include <gnuastro/config.h>
#endif


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




int
gal_units_extract_decimal(char *convert, const char *delimiter,
                          double *args, size_t n);

double
gal_units_ra_to_degree(char *convert);

double
gal_units_dec_to_degree(char *convert);

char *
gal_units_degree_to_ra(double decimal, int usecolon);

char *
gal_units_degree_to_dec(double decimal, int usecolon);

double
gal_units_counts_to_mag(double counts, double zeropoint);

double
gal_units_mag_to_counts(double mag, double zeropoint);

double
gal_units_counts_to_jy(double counts, double zeropoint_ab);

double
gal_units_jy_to_counts(double jy, double zeropoint_ab);

double
gal_units_jy_to_mag(double jy);

double
gal_units_mag_to_jy(double mag);

double
gal_units_au_to_pc(double au);

double
gal_units_pc_to_au(double pc);

double
gal_units_ly_to_pc(double ly);

double
gal_units_pc_to_ly(double pc);

double
gal_units_ly_to_au(double ly);

double
gal_units_au_to_ly(double au);

__END_C_DECLS    /* From C++ preparations */

#endif           /* __GAL_UNITS_H__ */
