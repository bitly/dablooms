from distutils.core import setup, Extension
import os, sys, string

def local_path(path):
    local_dir = os.path.dirname(__file__)
    return os.path.normpath(os.path.join(local_dir, path))

def parse_version_from_c():
    cfile = open(local_path('pydablooms.c'))
    result = ''
    for line in cfile:
        parts = line.split()
        if len(parts) == 3 and parts[:2] == ['#define', 'PYDABLOOMS_VERSION']:
            result = parts[2].strip('"')
            break
    cfile.close()
    return result

module1 = Extension('pydablooms', 
                    include_dirs = [local_path('../src')],
                    libraries = ['dablooms'],
                    library_dirs = [local_path('../build')],
                    sources = [local_path('pydablooms.c')],
                   )

setup (name = 'pydablooms',
    version = parse_version_from_c(),
    description = 'This is a a python extension of the scaling, counting, bloom filter, dablooms.',
    author = 'Justin P. Hines',
    author_email = 'justinhines@bit.ly',
    url = 'http://github.com/bitly/dablooms.git',
    ext_modules = [module1])
