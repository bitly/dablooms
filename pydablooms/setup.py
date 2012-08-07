from distutils.core import setup, Extension
import os, sys

def local_path(path):
    local_dir = os.path.dirname(__file__)
    return os.path.normpath(os.path.join(local_dir, path))

def parse_version_from_c():
    cfile = open(local_path('../src/dablooms.c'))
    result = ''
    for line in cfile:
        parts = line.split()
        if len(parts) == 3 and parts[:2] == ['#define', 'DABLOOMS_VERSION']:
            result = parts[2].strip('"')
            break
    cfile.close()
    return result

def paths_from_env(name, default):
    paths_str = os.environ.get(name, local_path(default))
    return paths_str.split()

module1 = Extension('pydablooms', 
                    include_dirs = paths_from_env('INCPATH', '../src'),
                    libraries = ['dablooms'],
                    library_dirs = paths_from_env('LIBPATH', '../build'),
                    sources = [local_path('pydablooms.c')],
                   )

setup (name = 'pydablooms',
    version = parse_version_from_c(),
    description = 'This is a a python extension of the scaling, counting, bloom filter, dablooms.',
    author = 'Justin P. Hines',
    author_email = 'justinhines@bit.ly',
    url = 'http://github.com/bitly/dablooms.git',
    ext_modules = [module1])
