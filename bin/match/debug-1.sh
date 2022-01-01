#!/bin/bash
# Script to create inputs to debug match. It can either take one catalog,
# randomly select a sub-set, and add noise to its coordinates to create a
# second catalog, or just build two catalogs from scratch.
#
# Original author:
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Contributing author(s):
# Copyright (C) 2021-2022 Free Software Foundation, Inc.
#
# Gnuastro is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# Gnuastro is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with Gnuastro. If not, see <http://www.gnu.org/licenses/>.





# If you want to use a real catalog as reference (and manually add noise to
# randomly selected rows, give a value to the 'real' variable). If this is
# empty, a mock table with two columns of integers as input will be
# created.
real=~/tmp/gaia.fits
real_c1=ra1
real_c2=dec1

# Number of points (number of inputs in mock, and number of rows in noised
# catalog for real).
num=300000

# Scatter around each coordinate to add and aperture size for mock and real
# tables.
sigma_mock=1
sigma_real=0.000555556 # 2 arcsecond (in degrees)

# Build the base catalog input
if [ x"$real" = x ]; then
    sigma=$sigma_mock
    for i in $(seq 1 $num); do
        echo "$i $i"
    done | asttable ../base.fits
else
    sigma=$sigma_real
    asttable $real -c$real_c1,$real_c2 --output=../base.fits
fi

# Add noise to both columns to build the new dataset.
asttable ../base.fits -c'arith $1 '$sigma' mknoise-sigma' \
         -c'arith $2 '$sigma' mknoise-sigma' --rowrandom=$num \
         -o../base-noised.fits
