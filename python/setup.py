import os
from numpy import get_include
from setuptools import setup, Extension





# Initialization
# ==============
# A short description of the Gnuastro Python package.
descp = '''A package of programs and library functions for \
          astronomical data manipulation and analysis.'''

# Get the absolute path for the current directory
here = os.path.abspath(os.path.dirname(__file__))

# Get the paths to where the gnuastro library(libgnuastro),
# the source files(.h) and the extension modules(.c)
# are from environment variables defined in the Makefile.

# For Debugging:
# src_dir = "./src"
# include_dir_src = here + "/../lib"
# include_dir_bld = "./../lib"
# lib_dir = "/usr/local/lib"
# tmp_lib_dir = include_dir_bld + "/.libs"



# Path to the source files for the extension
# modules where the wrapper functions and NumPy
# converters are written. These will be in the source tree.
src_dir = os.getenv("srcdir") + "/src"

# Include path for the gnuastro library
# header files used in the extension modules.
include_dir_src = os.getenv("srcdir") + "/../lib"

# The heaader files themselves requre config.h
# which is built in the 'lib' directory of build tree.
include_dir_bld = os.getenv("top_builddir") + "/lib"

# Since setup.py is called in the 'make' step, the
# gnuastro library has not currently been installed but the
# library files are stored in the 'libs/.libs' dir. in the build tree.
tmp_lib_dir = os.getenv("top_builddir") + "/lib/.libs"

# After the user has installed the gnuastro Library
# we want to link to this installed library instead of
# the tmp_lib_dir, which maybe deleted later on.
lib_dir = os.getenv("prefix") + "/lib"

# These arguments will be common while initializing
# all Extension modules. Hence, can be defined here only once.
default_ext_args = dict(include_dirs=[include_dir_src,
                                      include_dir_bld,
                                      get_include()],
                        libraries=["gnuastro"],
                        library_dirs=[tmp_lib_dir, lib_dir])





# Utility Functions
# =================
def find_version():
  '''
  Uses the version specified in the .version file at
  the root of the source, to find the version of Gnuastro.
  '''
  with open(os.path.join(here,"..",".version")) as f:
    ver = f.read()
  return ver



def get_license():
  '''
  Gets the lisence info from the
  license document in the root of the source.
  '''
  with open(os.path.join(here,"..","COPYING")) as f:
    lic = f.read()
  return lic





# Extension Modules
# =================
# Each module is defined using its name, source
# file and the default arguments dictionary defined above.
cosmology = Extension(name='cosmology',
                      sources=[f'{src_dir}/cosmology.c'],
                      **default_ext_args)



fits = Extension(name='fits',
                 sources=[f'{src_dir}/fits.c'],
                 **default_ext_args)





# Setup
# =====
# This is the main funciton which builds the module.
# It uses the metadata passed as arguments to describe the Python Package
setup(name="Gnuastro",
      version=find_version(),
      description=descp,
      author="Mohammad Akhlaghi",
      author_email="mohammad@akhlaghi.org",
      url="http://www.gnu.org/software/gnuastro/manual/",
      project_urls={
        "Manual": "http://www.gnu.org/software/gnuastro/manual/",
        "Bug Tracker": "https://savannah.gnu.org/bugs/?group=gnuastro",},
      license=get_license(),
      ext_package="gnuastro", # This will be used as base package name.
      ext_modules=[fits,cosmology])