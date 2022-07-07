from distutils.core import Extension, setup

module_cosmo = Extension("cosmology",
                          sources=["cosmology.c"],
                          include_dirs = ['/usr/local/include'],
                          libraries=["gnuastro"],
                          library_dirs=["/usr/local/lib"])

setup(name="Cosmology",
      version="1.0",
      description="This library does the main cosmological calculations that are commonly necessary in extra-galactic astronomical studies. The main variable in this context is the redshift (z). The cosmological input parameters in the functions below are H0, o_lambda_0, o_matter_0, o_radiation_0 which respectively represent the current (at redshift 0) expansion rate (Hubble constant in units of km/sec/Mpc), cosmological constant (Λ), matter and radiation densities.",
      ext_modules=[module_cosmo]
      )
