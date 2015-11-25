#!/usr/bin/env python
import os
import sys
import distutils.sysconfig

user = False
system = False

def err_exit(msg):
    sys.stderr.write(msg + "\n")
    exit(1)

for arg in sys.argv[1:]:
    if   arg == "--user":    user   = True
    elif arg == "--system":  system = True
    else:
        err_exit("invalid argument '%s'" % arg)

mod_paths = sys.path
mod_paths.reverse()

if user:
    home = os.environ.get('HOME')
    if not home:
        err_exit("environment variable 'HOME' not set")
    for p in mod_paths:
        if p.startswith(home):
            print(p)
            exit(0)
    err_exit("no user python module path found")

if not system:
    for p in mod_paths:
        if p.startswith("/usr/local/"):
            print(p)
            exit(0)

print(distutils.sysconfig.get_python_lib())
