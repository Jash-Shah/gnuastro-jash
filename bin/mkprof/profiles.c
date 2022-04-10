/*********************************************************************
MakeProfiles - Create mock astronomical profiles.
MakeProfiles is part of GNU Astronomy Utilities (Gnuastro) package.

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
#include <stdio.h>
#include <stdlib.h>

#include <gsl/gsl_sf_gamma.h>   /* For total Sersic brightness. */

#include "main.h"
#include "mkprof.h"             /* Needs main.h, astrthreads.h */
#include "profiles.h"





/****************************************************************
 *****************         Profiles:         ********************
 ****************************************************************/
/* The distance of this pixel. */
double
profiles_radial_distance(struct mkonthread *mkp)
{
  return mkp->r;
}





/* Azimuthal angle.

   Assuming theta is the azimuthal angle (along a constant radius), then an
   ellipse is defined by:

      x=a*cos(theta)
      y=b*sin(theta)

   Now, let's take 'phi' to be the angle in the normal equi-distant
   (non-elliptical coordinates). Therefore:

      tan(phi)=y/x

   So:
                 b*sin(theta)
      tan(phi) = ------------ = b/a * tan(theta)
                 a*cos(theta)

   Therefore we can find theta like this (where q=b/a):

      theta = atan( a/b * tan(phi) )
            = atan( a/b * y/x )
            = atan( y / (x*q) )

   However, the 'x' and 'y' above are only for the case where the ellipse
   has a position angle of zero (its major axis is aligned with the
   horizontal axis. When the ellipse is rotated, we first need to rotate
   the 'mkp->coord' values, and then use 'x' and 'y' like above. */
double
profiles_azimuth(struct mkonthread *mkp)
{
  /* We are rotating by the inverse (multiplied by -1) position angle. */
  double x =      mkp->coord[0]*mkp->c[0] + mkp->coord[1]*mkp->s[0];
  double y = -1 * mkp->coord[0]*mkp->s[0] + mkp->coord[1]*mkp->c[0];

  /* The normal 'atan' function will only return values between -90 to +90
     degrees, or only two quadrants (because it only takes a single
     value). However, with 'atan2' we can get the full -180 to +180 degrees
     (all four quadrants). */
  double d=atan2(y, x*mkp->q[0])*RADIANSTODEGREES;

  /* 'atan2' returns values between -180 to 180 deg. But we want values
     from 0 to 360, so we'll add the negative ones by 360. */
  return d>0 ? d : d+360.0f;
}





/* Read the values based on the distance from a table. */
double
profiles_custom_table(struct mkonthread *mkp)
{
  long i; /* May become negative. */
  double *reg=mkp->p->customregular;
  double *min=mkp->p->custom->array;
  double *max=mkp->p->custom->next->array;
  double *value=mkp->p->custom->next->next->array;
  double out=0.0f; /* Zero means no value, user may want a NaN value! */

  /* If the table isn't regular ('reg[0]' isn't NaN), then we have to parse
     over the whole table. However, if its regular, we can find the proper
     value much more easily. */
  if( isnan(reg[0]) )
    {
      for(i=0;i<mkp->p->custom->size;++i)
        if( mkp->r >= min[i] && mkp->r < max[i] )
          { out=value[i]; break; }
    }
  else
    {
      i=(mkp->r - reg[0])/reg[1];
      if(i>=0 && i<=mkp->p->custom->size) out=value[i];
    }

  /* Return the output value. */
  return out;
}





/* This is just a place-holder function, but will never be used. */
double
profiles_custom_image(struct mkonthread *mkp)
{
  return NAN;
}





/* The integral of the Gaussian from -inf to +inf equals the square root of
   PI. So from zero to +inf it equals half of that.*/
double
profiles_gaussian_total(double q)
{
  return q*sqrt(M_PI)/2;
}





/* The Gaussian function at a point. */
double
profiles_gaussian(struct mkonthread *mkp)
{
  return exp( mkp->gaussian_c * mkp->r * mkp->r );
}





/* This function will find the moffat function alpha value based on
   the explantions here:

   http://labs.adsabs.harvard.edu/adsabs/abs/2001MNRAS.328..977T/

   alpha=(FWHM/2)/(2^(1.0/beta)-1)^(0.5). Then the moffat
   function at r is: (1.0 + (r/alpha)^2.0)^(-1.0*beta)*/
double
profiles_moffat_alpha(double fwhm, double beta)
{
  return (fwhm/2)/pow((pow(2, 1/beta)-1), 0.5f);
}





/* Find the total value of the Moffat profile. I am using equation 10
 from Pengetal 2010 (Galfit). In finding the profiles, I am assuming
 \Sigma_0=1. So that is what I put here too.*/
double
profiles_moffat_total(double alpha, double beta, double q)
{
  return M_PI*alpha*alpha*q/(beta-1);
}





/* Find the Moffat profile for a certain radius.

   rda=r/alpha     and nb=-1*b.

   This is done before hand to speed up the process. */
double
profiles_moffat(struct mkonthread *mkp)
{
  return pow(1+mkp->r*mkp->r/mkp->moffat_alphasq, mkp->moffat_nb);
}





/* This approximation of b(n) for n>0.35 is taken from McArthur,
   Courteau and Holtzman 2003:
   http://adsabs.harvard.edu/abs/2003ApJ...582..689 */
double
profiles_sersic_b(double n)
{
  if(n<=0.35f)
    error(EXIT_FAILURE, 0, "the Sersic index cannot be smaller "
          "than 0.35. It is %.3f", n);
  return 2*n-(1/3)+(4/(405*n))+(46/(25515*n*n))+
    (131/(1148175*n*n*n)-(2194697/(30690717750*n*n*n*n)));
}





/* Find the total brightness in a Sersic profile. From equation 4 in
   Peng 2010. This assumes the surface brightness at the effective
   radius is 1.*/
double
profiles_sersic_total(double n, double re, double b, double q)
{
  return (2*M_PI*re*re*exp(b)*n*pow(b, -2*n)*q*
          gsl_sf_gamma(2*n));
}





/* Find the Sersic profile for a certain radius. rdre=r/re, inv_n=1/n,
   nb= -1*b.  */
double
profiles_sersic(struct mkonthread *mkp)
{
  return exp( mkp->sersic_nb
              * ( pow(mkp->r/mkp->sersic_re, mkp->sersic_inv_n) -1 ) );
}





/* Make a circumference (inner to the radius). */
double
profiles_circumference(struct mkonthread *mkp)
{
  return ( (mkp->r > mkp->intruncr && mkp->r <= mkp->truncr)
           ? mkp->fixedvalue : 0.0f );
}





/* Always returns a fixed value: */
double
profiles_flat(struct mkonthread *mkp)
{
  return mkp->fixedvalue;
}
