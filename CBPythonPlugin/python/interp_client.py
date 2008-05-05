import xmlrpclib

s = xmlrpclib.Server('http://localhost:800')
# Print list of available methods
print s.system.listMethods()
s.end()

