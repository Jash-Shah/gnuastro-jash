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




# Initialization
# ==============
# Exporting these variables so that they can be
# used by setup.py script when building the extension modules
export prefix:=$(prefix)
export srcdir:=$(srcdir)
export top_builddir:=$(top_builddir)

# The options given are:
# 	--user   : installs the package in the home directory
#              of the current user, instead of the root directory.
#   --upgrade: updates the package to the latest version available.
pip_install_cmd = pip install \
                  $(builddir)/dist/Gnuastro*.whl \
                  --verbose --user --upgrade
pip_uninstall_cmd = pip uninstall gnuastro -y



BUILT_SOURCES = setup-done





# Rules
# =====
# This is the main command which builds the
# extension modules and generates the wheel file for installing.
# Since, setup-done is a prereq. of all, it gets called everytime
# the 'make' is invoked.
setup-done: $(srcdir)/setup.py
	$(PYTHON) $(srcdir)/setup.py build_ext bdist_wheel --verbose
	echo "Python extensions have been built" > $@






# Cleans the 'build' 'dist' and
# '.egg-info' folders that are made during building.
clean-local:
	rm -rf $(builddir)/Gnuastro.egg-info
	rm -rf $(builddir)/build
	rm -rf $(builddir)/dist
	rm -f setup-done





# The wheel(.whl) file can be locally installed
# using pip. Although, pip needs to be ran not as root
# so that the package gets installed in the ~/.local folder.
# SUDO_USER(see 'man sudo') is provided if 'sudo make install' is
# invoked, in which case we default pip to the SUDO_USER instead of root.
install-exec-local:
	if [ -z "$(SUDO_USER)" ]; then \
		$(pip_install_cmd); \
	else \
		sudo -u "$(SUDO_USER)" $(pip_install_cmd); \
	fi





uninstall-local:
	if [ -z "$(SUDO_USER)" ]; then \
		$(pip_uninstall_cmd); \
	else \
		sudo -u "$(SUDO_USER)" $(pip_uninstall_cmd); \
	fi