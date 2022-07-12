import os
from numpy import get_include
from setuptools import setup, Extension

here = os.path.abspath(os.path.dirname(__file__))
descp = '''
         A package of programs and library functions for 
         astronomical data manipulation and analysis
        '''
default_ext_args = dict(include_dirs=[get_include()],
                        libraries=["gnuastro"],
                        library_dirs=["/usr/local/lib"])


def find_version():
  with open(os.path.join(here,"..",".version")) as f:
    ver = f.read()
  return ver

def get_license():
  with open(os.path.join(here,"..","COPYING")) as f:
    lic = f.read()
  return lic

# define the extension module
cosmology = Extension(name='cosmology',
                      sources=['src/cosmology.c'],
                      **default_ext_args)

fits = Extension(name='fits',
                 sources=['src/fits.c'],
                 **default_ext_args)

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
