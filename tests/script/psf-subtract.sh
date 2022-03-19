# Generate a simple scattered light field with one single object
#
# See the Tests subsection of the manual for a complete explanation
# (in the Installing gnuastro section).
#
# Original author:
#     Raul Infante-Sainz <infantesainz@gmail.com>
# Contributing author(s):
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Copyright (C) 2015-2021, Free Software Foundation, Inc.
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
prog=psf-subtract
execname=../bin/script/astscript-$prog

fits1name=mkprofcat1.fits
dep1name=$progbdir/astfits
dep2name=$progbdir/astcrop
dep3name=$progbdir/asttable
dep4name=$progbdir/astarithmetic
dep5name=$progbdir/aststatistics
dep6name=$progbdir/astscript-radial-profile





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
if [ ! -f $dep3name ]; then echo "$dep3name doesn't exist."; exit 77; fi
if [ ! -f $dep4name ]; then echo "$dep4name doesn't exist."; exit 77; fi
if [ ! -f $dep5name ]; then echo "$dep5name doesn't exist."; exit 77; fi
if [ ! -f $dep6name ]; then echo "$dep6name doesn't exist."; exit 77; fi
if [ ! -f $fits1name ]; then echo "$fits1name doesn't exist."; exit 77; fi





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
x=$($dep1name $fits1name -h1 | awk '/^NAXIS1/{print $3/2}')
y=$($dep1name $fits1name -h1 | awk '/^NAXIS2/{print $3/2}')
$check_with_program $execname $fits1name --center=$x,$y --mode=img \
                              --psf=$fits1name --scale=3.3 \
                              --output=$prog.fits \
                              --tmpdir=tmpdir-$prog
