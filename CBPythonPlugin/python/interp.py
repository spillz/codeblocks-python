import code
import sys
from SimpleXMLRPCServer import SimpleXMLRPCServer

class datastore:
    def __init__(self):
        self.data=''
    def write(self,textstr):
        #queue stdout string to be sent to client periodically
        print >>sys.__stdout__, 'INTERCEPTED:',textstr
        self.data=self.data+textstr
    def flush(self):
        data=''
    def read(self):
        #ask client for input
        return ''


class PyInterp (code.InteractiveInterpreter):
    def __init__(self):
        code.InteractiveInterpreter.__init__(self)
        self._stdout=datastore()
        self._stdin=datastore()
        self._stderr=datastore()
        self._quit=False
    def end(self):
        self._quit=True


interp = PyInterp()
sys.stdout=interp._stdout
sys.stdin=interp._stdin
sys.stderr=interp._stderr

print 'test'

# Create XMLRPC server
server = SimpleXMLRPCServer(("localhost", 800))
server.register_introspection_functions()
server.socket.settimeout(5)
server.register_instance(interp)


while not interp._quit:
    try:
        server.handle_request()
    except:
        print "Unexpected Error"

print 'success'


## SimpleXMLRPCServer examples

# Register a function under a different name
#def adder_function(x,y):
#    return x + y
#server.register_function(adder_function, 'add')


# Run the server's main loop
#server.serve_forever()
