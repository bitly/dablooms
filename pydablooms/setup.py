from distutils.core import setup, Extension
import sys, string

from_dir = sys.argv[0][0:string.find(sys.argv[0], 'setup.py')]

module1 = Extension('pydablooms', 
                    include_dirs = ['/usr/local/include'],
                    libraries= ['dablooms'],
                    library_dirs = ['/usr/local/lib'],
                    sources = [from_dir + 'pydablooms.c'])

setup (name = 'pydablooms',
    version = '0.5',
    description = 'This is a a python extension of the scaling, counting, bloom filter, dablooms.',
    author = 'Justin P. Hines',
    author_email = 'justinhines@bit.ly',
    url = 'http://github.com/bitly/dablooms.git',
    ext_modules = [module1])
