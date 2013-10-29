import sys
import os
import struct
import xmlrpclib
from SimpleXMLRPCServer import SimpleXMLRPCServer
import jedi
import textwrap

def type_suffix(d):
    #TODO: Add type/scope combinations here adding more bitmaps to the list in the plugin
    if d.startswith('statement'):
        return '?'+str(5)
    elif d.startswith('class'):
        return '?'+str(2)
    elif d.startswith('function'):
        return '?'+str(6)
    elif d.startswith('import'):
        return '?'+str(1)
    elif d.startswith('keyword'):
        return ''
    return ''

isz=struct.calcsize('I')

class XmlRpcPipeServer:
    '''
    A simple XMLRPC server implementation that uses the stdin, stdout
    pipes to communicate with the owner of the process. Implements
    most of the features of SimpleXMLRPCServer
    '''
    def __init__(self):
        self.fn_dict={}
        self.inpipe=sys.stdin
        self.outpipe=sys.stdout
        sys.stdout=open(os.devnull,'wb')

    def register_function(self,fn,name):
        self.fn_dict[name]=fn

    def register_introspection_functions(self):
        pass

    def handle_request(self):
        ##TODO: Need more error handling!
        size_buf = self.inpipe.read(isz)
        size = struct.unpack('I',size_buf)[0]
        call_xml = self.inpipe.read(size)
        self.outpipe.write(struct.pack('c','M'))
        name=''
        try:
            args,name = xmlrpclib.loads(call_xml)
            result = self.__call__(name, *args)
            if not isinstance(result,tuple):
                result=(result,)
        except:
            import traceback
            result ='Error running call'+name+'\n'+call_xml+'\n'
            result += '\n'.join(traceback.format_exception(*sys.exc_info()))
            result = (result,)
        try:
            res_xml = bytes(xmlrpclib.dumps(result, methodresponse=True))
        except:
            res_xml = bytes(xmlrpclib.dumps('Method result of length %i could not be converted to XML'%(len(res_xml)), methodresponse=True))
        size = len(res_xml)
        self.outpipe.write(struct.pack('I',size))
        self.outpipe.write(res_xml)

    def __call__(self,name,*args):
        return self.fn_dict[name](*args)


class PythonCompletionServer:
    '''
    An XMLRPC server that serves up a jedi interface
    '''
    def __init__(self,port):
        # Create XMLRPC server
        self.timeout=0.2
        self._quit=False
        self.port=port
        self.project=None
        self.active_path=None

    def run(self):
        if self.port==-1:
            self.server = XmlRpcPipeServer()
        else:
            self.server = SimpleXMLRPCServer(("localhost", self.port))
        self.server.logRequests=0
        self.server.register_introspection_functions()
        #self.server.socket.settimeout(self.timeout) ##timeouts cause probs
        self.server.register_function(self.end,'end')
        self.server.register_function(self.complete_phrase,'complete_phrase')
        self.server.register_function(self.complete_tip,'complete_tip')
        self.server.register_function(self.get_definition_location,'get_definition_location')
        while not self._quit:
            self.server.handle_request()

    def complete_phrase(self,path,source,line,column):
        source=source.replace('\r','')
        script = jedi.Script(source,line=line+1,column=column,source_path=path)
        results = script.completions()
        completions=sorted([s.name+type_suffix(s.description) for s in results])
        return completions

    def complete_tip(self,path,source,line,column):
        source=source.replace('\r','')
        script = jedi.Script(source,line=line+1,column=column,source_path=path)
        call_def = script.call_signatures()
        calltip = ''
        for c in call_def:
            calltip=c.call_name+'('
            calltip+=', '.join([p.get_name().names[0] for p in c.params])+')'
            calltip = '\n'.join(textwrap.wrap(calltip,70))
#            docstr=''
#            for p in c.params:
#                docstr+=p.docstr
#            if docstr!='':
#                calltip+='\n'+docstr
            return calltip
        return ''

    def get_definition_location(self,path,source,line,column):
        source=source.replace('\r','')
        script = jedi.Script(source,line=line+1,column=column,source_path=path)
        results = script.goto_definitions()
        for d in results:
            module_path = d.module_path
            if module_path is None:
                module_path = path
            return [module_path,d.line-1]
        return ['',-1]

    def end(self):
        self._quit=True
        return True

def cmd_err():
    print 'Correct usage: python_completion_server.py <port>'
    print '<port> must be a positive integer or -1 to use stdin/stdout'
    sys.exit()

if __name__=='__main__':
    if len(sys.argv)!=2:
        port=8001
    else:
        try:
            port = int(sys.argv[1])
        except:
            cmd_err()
        if port<-1:
            cmd_err()

    server=PythonCompletionServer(port)
    server.run()
