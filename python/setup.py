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

# Get the paths to where the gnuastro library
# the source files(.c) for the extension modules
# are from environment variables defined in the Makefile.
lib_dir = os.getenv("prefix") + "/lib"
src_dir = os.getenv("srcdir") + "/src"

# These arguments will be common while initializing
# all Extension modules. Hence, can be defined here only once.
default_ext_args = dict(include_dirs=[get_include()],
                        libraries=["gnuastro"],
                        library_dirs=[lib_dir])





# Utility Functions
# =================
'''
Uses the version specified in the .version file at
the root of the source, to find the version of Gnuastro.
'''
def find_version():
  with open(os.path.join(here,"..",".version")) as f:
    ver = f.read()
  return ver



'''
Gets the lisence info from the
license document in the root of the source.
'''
def get_license():
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