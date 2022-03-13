# Generate a catalog of stars to test the select-stars PSF script
#
# See the Tests subsection of the manual for a complete explanation
# (in the Installing gnuastro section).
#
# Original author:
#     Raul Infante-Sainz <infantesainz@gmail.com>
# Contributing author(s):
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Copyright (C) 2021, Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.





# Preliminaries
# =============
#
# Set the variables (the executable is in the build tree). Do the
# basic checks to see if the executable is made or if the defaults
# file exists (basicchecks.sh is in the source tree).
prog=psf-select-stars
execname=../bin/script/astscript-$prog

dep1name=$progbdir/astmatch
dep2name=$progbdir/astmkcatalog
fits1name=convolve_spatial_noised.fits
fits2name=convolve_spatial_noised_detected_segmented.fits
fits3name=convolve_spatial_noised_detected_segmented_catalog.fits





# Skip?
# =====
#
# If the dependencies of the test don't exist, then skip it. There are two
# types of dependencies:
#
#   - The executable script was not made.
#   - The programs it use weren't made.
#   - The input data weren't made.
if [ ! -f $execname ]; then echo "$execname doesn't exist."; exit 77; fi
if [ ! -f $dep1name ]; then echo "$dep1name doesn't exist."; exit 77; fi
if [ ! -f $dep2name ]; then echo "$dep2name doesn't exist."; exit 77; fi
if [ ! -f $fits1name ]; then echo "$fits1name doesn't exist."; exit 77; fi
if [ ! -f $fits2name ]; then echo "$fits2name doesn't exist."; exit 77; fi





# Actual test script
# ==================
#
# 'check_with_program' can be something like Valgrind or an empty
# string. Such programs will execute the command if present and help in
# debugging when the developer doesn't have access to the user's system.
#
# Since we want the script to recognize the programs that it will use from
# this same build of Gnuastro, we'll add the current directory to PATH.
export PATH="$progbdir:$PATH"

# Create a catalog with appropiate parameters
$check_with_program astmkcatalog $fits2name \
                    --ra --dec --magnitude \
                    --axisratio --output=$fits3name

# Test the script: selecting good stars
$check_with_program $execname $fits1name --hdu=1 \
                    --segmented=$fits2name \
                    --catalog=$fits3name \
                    --field=magnitude \
                    --magnituderange=-17,-10 \
                    --minaxisratio=0.85 \
                    --mindistdeg=0.05 \
                    --matchaperturedeg=0.5
