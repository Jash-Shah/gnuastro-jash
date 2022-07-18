## Process this file with automake to produce Makefile.in
##
## Hand written file: used as input into Autotools.
## This is part of GNU Astronomy Utilities (gnuastro).
##
## Original author:
##     Mohammad Akhlaghi <mohammad@akhlaghi.org>
## Contributing author(s):
## Copyright (C) 2015-2022 Free Software Foundation, Inc.
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


# Builds the extension modules for gnuastro to be used in Python.

# NOTE
# ==== 
# This file is only processed if the user has Python and NumPy setup.






# Exporting these variables so that they can be
# used by setup.py script when building the extension modules
export prefix:=$(prefix)
export srcdir:=$(srcdir)
    


# This is the main command which builds the
# extension modules and generates the wheel file for installing.
all-local: $(srcdir)/setup.py $(srcdir)/src/*.c
	$(PYTHON) $(srcdir)/setup.py build_ext bdist_wheel \
						--verbose





# Cleans the /build /dist and
# egg-info folders that are made during building.
clean-local: clean-build clean-dist
	rm -r $(builddir)/Gnuastro.egg-info

clean-build:
	if [ -d "./build" ]; then \
		rm -r $(builddir)/build; \
	fi

clean-dist:
	if [ -d "./dist" ]; then \
		rm -r $(builddir)/dist; \
	fi





# The wheel(.whl) file can be locally installed
# using pip. Although, pip needs to be ran not as root
# so that the package gets installed in the ~/.local folder.
install-exec-local:
	sudo -u $(SUDO_USER) pip install $(builddir)/dist/G*.whl \
											 --verbose





uninstall-local:
	sudo -u $(SUDO_USER) pip uninstall gnuastro -y