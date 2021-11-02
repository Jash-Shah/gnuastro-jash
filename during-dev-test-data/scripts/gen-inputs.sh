#!/bin/bash

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
