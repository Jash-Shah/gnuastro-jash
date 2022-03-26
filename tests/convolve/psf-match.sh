# Find the matching kernel for two PSFs
#
# See the Tests subsection of the manual for a complete explanation
# (in the Installing gnuastro section).
#
# Original author:
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Contributing author(s):
# Copyright (C) 2022 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.





# Preliminaries
# =============
#
# Set the variables (The executable is in the build tree). Do the
# basic checks to see if the executable is made or if the defaults
# file exists (basicchecks.sh is in the source tree).
prog=convolve
psfwide=psf.fits
psfsharp=psf-sharp.fits
fitsprog=$progbdir/astfits
cropprog=$progbdir/astcrop
execname=$progbdir/ast$prog
arithprog=$progbdir/astarithmetic





# Skip?
# =====
#
# If the dependencies of the test don't exist, then skip it. There are two
# types of dependencies:
#
#   - The executable was not made (for example due to a configure option),
#
#   - The input data was not made (for example the test that created the
#     data file failed).
if [ ! -f $execname  ]; then echo "$execname not created."; exit 77; fi
if [ ! -f $psfwide   ]; then echo "$psfwide does not exist."; exit 77; fi
if [ ! -f $psfsharp  ]; then echo "$psfsharp does not exist."; exit 77; fi
if [ ! -f $fitsprog  ]; then echo "$fitsprog does not exist."; exit 77; fi
if [ ! -f $cropprog  ]; then echo "$cropprog does not exist."; exit 77; fi
if [ ! -f $arithprog ]; then echo "$arithprog does not exist."; exit 77; fi





# Prepare Sharper PSF
# -------------------
#
# It is necessary that the two PSFs be the same width. So before finding
# the matching PSF, it is necessary to make the sharper (smaller) PSF have
# the same pixel size as the wider one.
psfsharpwidth=psf-sharp-width-set.fits
psfsharpnoblank=psf-sharp-no-blank.fits
wwidth=$($fitsprog $psfwide --keyvalue=naxis1 --quiet)
center=$($fitsprog $psfsharp --keyvalue=naxis1 --quiet \
             | $AWK '{print int($1/2)+1}')
$cropprog $psfsharp --center=$center,$center --width=$wwidth \
          --mode=img --output=$psfsharpwidth
$arithprog $psfsharpwidth set-i i i isblank 0 where \
           --output=$psfsharpnoblank





# Actual test script
# ==================
#
# 'check_with_program' can be something like Valgrind or an empty
# string. Such programs will execute the command if present and help in
# debugging when the developer doesn't have access to the user's system.
$check_with_program $execname $psfwide --kernel=$psfsharpnoblank \
                              --makekernel=10   \
                              --output=psf-match.fits \
    && rm $psfsharpwidth $psfsharpnoblank
