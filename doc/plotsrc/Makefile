# Prepare all the figures for the GNU Astronomy Utilities manual.
#
# Original author:
#     Mohammad Akhlaghi <mohammad@akhlaghi.org>
# Contributing author(s):
# Copyright (C) 2015-2022 Free Software Foundation, Inc.
#
# GNU Astronomy Utilities is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# GNU Astronomy Utilities is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Astronomy Utilities. If not, see
# <http://www.gnu.org/licenses/>.

# Definitions of which rules to apply:
finaltarget=../gnuastro-figures/done.txt
all: $(finaltarget)
.PHONY: clean





# Directory to keep TiKZ products.
tikz:
	mkdir $@





# Make all the plots with LaTeX, then convert all the figures to the
# necessary formats using a separate script and finally write the final
# target.
#
# Explanation for putting all the commands in one line (by ending them with
# a '\' and starting the next one with '&&'). We do this because we can't
# assume GNU Make, or GNU Bash here (to use the '.SHELLFLAGS = -ec' and
# '.ONESHELL:' features in the Makefile). The user's Make or shell can be
# any implementation. If any of these commands fails, the final target
# should not be built and the Makefile should crash.
$(finaltarget): all.tex ./tex/*.tex ./conversions.sh | tikz

	latex -shell-escape all.tex \
	&& dvipdf all.dvi \
	&& rm -f *.bcf *.xml *.aux *.auxlock *.log *.out *.dvi \
	&& cp tikz/all-figure0.eps ../gnuastro-figures/iandtime.eps \
	&& cp tikz/all-figure1.eps ../gnuastro-figures/samplingfreq.eps \
	&& cp tikz/all-figure2.eps ../gnuastro-figures/flatplane.eps \
	&& cp tikz/all-figure3.eps ../gnuastro-figures/sphereandplane.eps \
	&& cp tikz/all-figure4.eps ../gnuastro-figures/color-names.eps \
	&& ./conversions.sh ../gnuastro-figures/ \
	&& echo "All necessary images created." > $(finaltarget)





# Clean all generated plots and conversions:
#   1. Copy all the actual PNG files into this directory.
#        NOTE I:  All non-plot figures should be saved in PNG.
#        NOTE II: All non-plot PNG figures should be listed here.
#   2. Remove everything in the gnuastro-figures directory
#   3. Put the actual images back in that directory.
tokeep = epicycles.png gnuastro.svg README
clean:
	rm -rf all.pdf tikz \
	&& for f in $(tokeep); do \
	     cp ../gnuastro-figures/$$f ./$$f-tmp; \
	   done \
	&& rm ../gnuastro-figures/* \
	&& for f in $(tokeep); do \
	     mv ./$$f-tmp ../gnuastro-figures/$$f; \
	   done
