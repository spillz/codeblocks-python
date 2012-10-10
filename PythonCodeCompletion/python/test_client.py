# client-side test program for interp.py

import xmlrpclib
import os
import subprocess
import sys
import struct
xml=xmlrpclib.dumps(('os',),'complete_phrase')
print xml
p=subprocess.Popen(['python','python_completion_server.py'],stdin=subprocess.PIPE,stdout=subprocess.PIPE)
p.communicate(struct.pack('@I',len(xml)))
p.communicate(xml)
print sys.stdin.read()


import sys
sys.exit()

s = xmlrpclib.ServerProxy('http://localhost:8000')
# Print list of available methods
for m in s.system.listMethods():
    print m
import numpy
numpy.array
import os.path
os.path.abspath
fullPath=os.path.abspath('./cbpycomp_client.py')
##origSource=open(fullPath,'r').read()
lineNo=13
origCol=9

s.load_stdlib('STDLIB')
print 'os#######################'
for c in s.complete_phrase('os'):
    print c
print 'os.path.#################'
for c in s.complete_phrase('os.path.'):
    print c
print 'call tip for os.path.exists('
print s.complete_tip('os.path.exists')
print 'call tip for numpy.array('
print s.complete_tip('numpy.array')
##s.get_completions(fullPath, origSource, lineNo, origCol)
