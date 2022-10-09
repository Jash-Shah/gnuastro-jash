/*********************************************************************
Warp - Warp images using projective mapping.
Warp is part of GNU Astronomy Utilities (Gnuastro) package.

Original author:
     Mohammad Akhlaghi <mohammad@akhlaghi.org>
Contributing author(s):
Copyright (C) 2017-2022 Free Software Foundation, Inc.

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
#ifndef AUTHORS_CITE_H
#define AUTHORS_CITE_H

/* When any specific citation is necessary, please add its BibTeX (from ADS
   hopefully) to this variable along with a title decribing what this
   paper/book does for the progarm in a short line. In the following line
   put a row of '-' with the same length and then put the BibTeX.

   This macro will be used in 'gal_options_print_citation' function of
   'lib/options.c' (from the top Gnuastro source code directory). */

#define PROGRAM_BIBTEX ""                                               \
  "Paper describing Warp's resampling\n"                                \
  "----------------------------------\n"                                \
  "@ARTICLE{gnuastro_warp,\n"                                           \
  "        author = {{Ashofteh-Ardakani}, Pedram and {Akhlaghi}, Mohammad},\n" \
  "         title = \"{Area-based resampling and effects on deep image}\",\n" \
  "       journal = {In preparation},\n"                                \
  "      keywords = {Astrophysics - Instrumentation and Methods for Astrophysics,\n" \
  "                  Astrophysics - Astrophysics of Galaxies,\n"        \
  "                  Computer Science - Computer Vision and Pattern Recognition},\n" \
  "          year = \"2023\",\n"                                        \
  "         month = \"Month\",\n"                                         \
  "           eid = {arXiv:XXXX.XXXXX},\n"                              \
  "         pages = {arXiv:XXXX.XXXXX},\n"                              \
  " archivePrefix = {arXiv},\n"                                         \
  "        eprint = {XXXX.XXXXX},\n"                                    \
  "  primaryClass = {astro-ph.IM},\n"                                   \
  "        adsurl = {https://ui.adsabs.harvard.edu},\n" \
  "}\n"

#define PROGRAM_AUTHORS "Mohammad Akhlaghi and Pedram Ashofteh-Ardakani"

#endif
