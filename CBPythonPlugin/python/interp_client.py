# client-side test program for interp.py

import xmlrpclib

s = xmlrpclib.ServerProxy('http://localhost:8000')
# Print list of available methods
for m in s.system.listMethods():
    print m
s.run_code('a=[1,2,3]')
s.run_code('print a')
s.end()

