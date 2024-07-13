from distutils.core import setup, Extension

module = Extension('RFCommRAW', sources=['rfcommraw/rfcommraw.c'], libraries=['bluetooth'])

setup(name='RFCommRAW',
      version='1.9',
      description='RFCOMM low level communication',
      ext_modules=[module])

