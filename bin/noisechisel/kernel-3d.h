/*********************************************************************
The default 3D kernel to be used in NoiseChisel and Segment.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2018-2019, Free Software Foundation, Inc.

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
#ifndef __GAL_KERNEL3D_H__
#define __GAL_KERNEL3D_H__

/* How to produce this kernel
   ==========================

   Below, the steps necessary to easily create the C contents of this file
   are described. The first step can be modified to change the default
   kernel properties and put the new contents into this file.

   Make the kernel
   ---------------

   We'll first make the kernel using MakeProfiles with the following
   commands. IMPORTANT NOTE: because the kernel is so sharp, random
   sampling is going to be used for all the pixels (the center won't be
   used). So it is important to have a large number of random points to
   make the very slight differences between symmetric parts of the profile
   even less significant.

     export GSL_RNG_SEED=1
     export GSL_RNG_TYPE=ranlxs2
     astmkprof --kernel=gaussian-3d,1.5,5,0.5 --oversample=1 --envseed \
               --numrandom=100000

   Convert it to C code
   --------------------

   Put the following C program into a file called `kernel.c'.

       #include <stdio.h>
       #include <stdlib.h>
       #include <gnuastro/fits.h>

       int
       main(void)
       {
         size_t i;
         float *arr;
         gal_data_t *img=gal_fits_img_read_to_type("kernel.fits", "1",
                                                   GAL_TYPE_FLOAT32, -1);

         arr=img->array;

         printf("size_t kernel_3d_dsize[3]={%zu, %zu, %zu};\n",
                img->dsize[0], img->dsize[1], img->dsize[2]);
         printf("float kernel_3d[%zu]={", img->size);
         for(i=0;i<img->size;++i)
           {
             if(i>0)
               {
                 if(i % img->dsize[2]                 == 0 ) printf("\n");
	         if(i % (img->dsize[2]*img->dsize[1]) == 0 ) printf("\n");
	       }

	     // We cannot use `\b' here, since we are writing directly
	     // to the command-line, so we'll first write the number,
	     // then decide if any subsequent character (a comma)
	     // should be written.
	     printf("%.7g", arr[i]);

	     // The last element doesn't need a comma. In each line,
	     // the last character must not be a space, but for easy
	     // readability, the elements in between need a space.
	     if( i!=(img->size-1) )
	       printf("%s", ((i+1)%img->dsize[2]) ? ", " : ",");
           }
         printf("};\n");

         return EXIT_SUCCESS;
       }

   Run the C program
   -----------------

   We can now compile and run that C program and put the outputs in
   `kernel.c'. Once its created, copy the contents of `kernel-3d.h' after
   these comments.

     $ astbuildprog -q kernel.c > kernel-3d.h
 */

size_t kernel_3d_dsize[3]={3, 7, 7};
float kernel_3d[147]={0, 0, 5.219212e-07, 1.43663e-06, 5.288961e-07, 0, 0,
0, 4.890969e-06, 0.0001122713, 0.0003051736, 0.0001101779, 4.908836e-06, 0,
5.281005e-07, 0.0001115758, 0.002482525, 0.006792462, 0.002476586, 0.0001099998, 5.217535e-07,
1.451268e-06, 0.0003033727, 0.006850741, 0.01871265, 0.006743792, 0.0003061892, 1.43298e-06,
5.209756e-07, 0.000110545, 0.002490561, 0.006798376, 0.002503618, 0.0001109581, 5.295083e-07,
0, 4.884327e-06, 0.0001122016, 0.0003074409, 0.0001103678, 4.984748e-06, 0,
0, 0, 5.224761e-07, 1.425723e-06, 5.33545e-07, 0, 0,

0, 3.538044e-07, 7.891201e-06, 2.166429e-05, 7.979216e-06, 3.570717e-07, 0,
3.536664e-07, 7.511148e-05, 0.001693119, 0.004623666, 0.001691994, 7.563465e-05, 3.541075e-07,
7.921932e-06, 0.001687588, 0.03788469, 0.1038283, 0.03776859, 0.001679332, 7.979274e-06,
2.200398e-05, 0.004617298, 0.1038528, 0.2849551, 0.1039089, 0.004635326, 2.165271e-05,
7.955934e-06, 0.001697289, 0.0379024, 0.1037254, 0.03768558, 0.001666544, 7.878737e-06,
3.523428e-07, 7.45342e-05, 0.001689582, 0.00461954, 0.001684659, 7.568914e-05, 3.499421e-07,
0, 3.56209e-07, 7.915472e-06, 2.17837e-05, 7.854093e-06, 3.555503e-07, 0,

0, 0, 5.263017e-07, 1.431613e-06, 5.12058e-07, 0, 0,
0, 4.907304e-06, 0.0001125078, 0.00030509, 0.000111018, 5.084412e-06, 0,
5.256628e-07, 0.0001103357, 0.002493239, 0.006835639, 0.002478384, 0.0001131122, 5.260213e-07,
1.442301e-06, 0.0003005426, 0.006833205, 0.01862562, 0.006816466, 0.000302246, 1.436046e-06,
5.298979e-07, 0.0001107397, 0.002476231, 0.006837885, 0.00252707, 0.0001124105, 5.305631e-07,
0, 5.012419e-06, 0.0001110889, 0.0003017522, 0.0001124517, 4.790862e-06, 0,
0, 0, 5.104474e-07, 1.45196e-06, 5.191671e-07, 0, 0};

#endif
