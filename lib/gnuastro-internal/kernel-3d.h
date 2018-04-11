/*********************************************************************
The default 3D kernel to be used in NoiseChisel and Segment.
This is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2018, Free Software Foundation, Inc.

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

/* How to build this kernel.

   Run MakeProfiles with the following commands:

       $ export GSL_RNG_SEED=1
       $ export GSL_RNG_TYPE=ranlxs2
       $ astmkprof --kernel=gaussian-3d,1.5,5,0.5 --oversample=1 --envseed

   The resulting fits file is converted to plain text with the this C
   program to generate the `kernel-3d.h' header file which is then put
   under these comments.

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

   Assuming this C program is in a file named `kernel.c', it can be
   compiled, run and saved into `kernel-3d.h' with the command below:

       $ astbuildprog -q kernel.c > kernel-3d.h

 */

size_t kernel_3d_dsize[3]={5, 9, 9};
float kernel_3d[405]={0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,

0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 5.293481e-07, 1.391654e-06, 5.283476e-07, 0, 0, 0,
0, 0, 5.023404e-06, 0.0001093707, 0.0003028791, 0.0001103754, 5.206822e-06, 0, 0,
0, 5.27038e-07, 0.0001140605, 0.002517939, 0.006957374, 0.002468025, 0.0001131454, 5.31773e-07, 0,
0, 1.481738e-06, 0.0002994444, 0.006708808, 0.01894199, 0.006776441, 0.0003116253, 1.493134e-06, 0,
0, 5.293459e-07, 0.0001081432, 0.002473192, 0.006704316, 0.002578867, 0.0001105525, 4.866729e-07, 0,
0, 0, 4.985141e-06, 0.000110672, 0.0002885369, 0.0001184523, 4.688276e-06, 0, 0,
0, 0, 0, 5.610833e-07, 1.336052e-06, 5.216995e-07, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,

0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 3.462786e-07, 7.853481e-06, 2.153742e-05, 7.564613e-06, 3.611804e-07, 0, 0,
0, 3.52808e-07, 7.161698e-05, 0.001676089, 0.004618002, 0.001656355, 7.414426e-05, 3.43867e-07, 0,
0, 7.990315e-06, 0.001704389, 0.03857445, 0.1031222, 0.03779064, 0.001703231, 7.975499e-06, 0,
0, 2.231775e-05, 0.00473238, 0.1043504, 0.2858114, 0.10241, 0.004618072, 2.193002e-05, 0,
0, 7.621142e-06, 0.001647367, 0.03755038, 0.1036048, 0.03796006, 0.001672143, 7.758124e-06, 0,
0, 3.68421e-07, 7.659229e-05, 0.001695716, 0.004599371, 0.001670764, 7.450273e-05, 3.481919e-07, 0,
0, 0, 3.521386e-07, 7.924394e-06, 2.207155e-05, 7.889852e-06, 3.53245e-07, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,

0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 5.182329e-07, 1.397928e-06, 5.17901e-07, 0, 0, 0,
0, 0, 4.812169e-06, 0.0001135803, 0.0003043477, 0.000109481, 4.974718e-06, 0, 0,
0, 5.377616e-07, 0.0001073098, 0.002424047, 0.006817353, 0.002492242, 0.0001147361, 5.33439e-07, 0,
0, 1.44089e-06, 0.0002933802, 0.006909565, 0.01873765, 0.006651767, 0.0003042257, 1.366647e-06, 0,
0, 5.166781e-07, 0.0001119049, 0.002454791, 0.006940499, 0.002578837, 0.0001061706, 5.329928e-07, 0,
0, 0, 4.656712e-06, 0.0001124511, 0.0003010639, 0.0001131462, 4.786908e-06, 0, 0,
0, 0, 0, 5.303103e-07, 1.363214e-06, 5.122773e-07, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,

0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0};

#endif
