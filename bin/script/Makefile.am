## Process this file with automake to produce Makefile.inx
##
## Original author:
##     Mohammad Akhlaghi <mohammad@akhlaghi.org>
## Contributing author(s):
## Copyright (C) 2019-2022 Free Software Foundation, Inc.
##
## Gnuastro is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Gnuastro is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Gnuastro. If not, see <http://www.gnu.org/licenses/>.





# Desktop files (for graphic environments)
pkgdata_DATA = astscript-fits-view.desktop
astscript-fits-view.desktop: $(srcdir)/fits-view.desktop.in Makefile
	sed -e 's|@PREFIX[@]|$(exec_prefix)|g' \
	    $(srcdir)/fits-view.desktop.in > $@





## List of programs (scripts in this directory) to install under the
## 'prefix/bin' directory ('bin_SCRIPTS'), files necessary to distribute
## with the tarball ('EXTRA_DIST') and output files (to be cleaned with
## 'make clean').
bin_SCRIPTS = astscript-fits-view \
              astscript-psf-unite \
              astscript-psf-stamp \
              astscript-ds9-region \
              astscript-psf-subtract \
              astscript-sort-by-night \
              astscript-radial-profile \
              astscript-psf-scale-factor \
              astscript-psf-select-stars

EXTRA_DIST = fits-view.in \
             psf-unite.in \
             psf-stamp.in \
             ds9-region.in \
             psf-subtract.in \
             sort-by-night.in \
             radial-profile.in \
             psf-select-stars.in \
             psf-scale-factor.in \
             fits-view.desktop.in

CLEANFILES = $(bin_SCRIPTS) \
             astscript-fits-view.desktop





## Command to do basic substitutions (anything surrounded by an '@').
do_subst = sed -e 's,[@]VERSION[@],$(VERSION),g' \
               -e 's,[@]SCRIPT_NAME[@],$@,g'





## Rules to install the scripts.
astscript-fits-view: fits-view.in Makefile
	$(do_subst) < $(srcdir)/fits-view.in > $@
	chmod +x $@

astscript-ds9-region: ds9-region.in Makefile
	$(do_subst) < $(srcdir)/ds9-region.in > $@
	chmod +x $@

astscript-radial-profile: radial-profile.in Makefile
	$(do_subst) < $(srcdir)/radial-profile.in > $@
	chmod +x $@

astscript-sort-by-night: sort-by-night.in Makefile
	$(do_subst) < $(srcdir)/sort-by-night.in > $@
	chmod +x $@

astscript-psf-unite: psf-unite.in Makefile
	$(do_subst) < $(srcdir)/psf-unite.in > $@
	chmod +x $@

astscript-psf-stamp: psf-stamp.in Makefile
	$(do_subst) < $(srcdir)/psf-stamp.in > $@
	chmod +x $@

astscript-psf-scale-factor: psf-scale-factor.in Makefile
	$(do_subst) < $(srcdir)/psf-scale-factor.in > $@
	chmod +x $@

astscript-psf-select-stars: psf-select-stars.in Makefile
	$(do_subst) < $(srcdir)/psf-select-stars.in > $@
	chmod +x $@

astscript-psf-subtract: psf-subtract.in Makefile
	$(do_subst) < $(srcdir)/psf-subtract.in > $@
	chmod +x $@
