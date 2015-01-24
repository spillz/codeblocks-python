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
            result = ({'error':'ERROR','desc':result},)
        try:
            res_xml = bytes(xmlrpclib.dumps(result, methodresponse=True))
        except:
            result = ({'error':'ERROR','desc':'Method result of length %i could not be converted to XML'%(len(res_xml))},)
            res_xml = bytes(xmlrpclib.dumps(result), methodresponse=True)
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
        self.server.register_function(self.get_doc,'get_doc')
        self.server.register_function(self.get_definition_location,'get_definition_location')
        while not self._quit:
            self.server.handle_request()

    def complete_phrase(self,path,source,line,column):
        source=source.replace('\r','')
        script = jedi.Script(source,line=line+1,column=column,source_path=path)
        self.completions = script.completions()
        comps=[s.name+type_suffix(s.description) for s in self.completions]
        return comps

    def get_doc(self,index):
        '''
        gets the documentation for the `index`th item of the last completion result in self.completions
        '''
        comp = self.completions[index]
        if comp.type in ['class','import']:
            doclines = comp.follow_definition()[0].raw_doc.splitlines()
        else:
            doclines = comp.doc.splitlines()
        doclines = [l.strip() for l in doclines]
        if len(doclines)>0:
            doclines = ['<br><br>']+[l + ' ' if l!='' else '<br><br>' for l in doclines[:-1]] + [doclines[-1]]
        doclines =['<b>'+comp.type+' '+comp.name+'</b>'] + doclines
        doc = ''.join(doclines)
        return doc

    def complete_tip(self,path,source,line,column):
        source=source.replace('\r','')
        #don't use the source_path arg because it causes buggy caching to be used
        script = jedi.Script(source,line=line+1,column=column)#,source_path=path)
        call_def = script.call_signatures()
        calltip = ''
        for c in call_def:
            calltip=c.call_name+'('
            if jedi.__version__ >= '0.8.0':
                calltip+=', '.join([p.name for p in c.params])+')'
                default_args=[]
                for p in c.params:
                    if '=' in p.description:
                        default_args.append(p.description)
                if default_args:
                    calltip = calltip + '\n\nDefault Arguments: ' + ', '.join(default_args)
#                if c.doc:
#                    calltip = calltip + '\n\nDescription\n' + c.doc
            else:
                calltip+=', '.join([p.get_name().names[0] for p in c.params])+')'
#            calltip = '\n'.join(textwrap.wrap(calltip,70))

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
            line=d.line-1
            if line is None:
                line=0
            return [module_path,line]
        return ['',-1]

    def end(self):
        self._quit=True
        return True

def cmd_err():
    print 'Correct usage: python_completion_server.py <port>'
    print '<port> must be a positive integer or -1 to use stdin/stdout'
    sys.exit()

if __name__=='__main__':
    if len(sys.argv)<2:
        port=8001
    else:
        try:
            port = int(sys.argv[1])
        except:
            cmd_err()
        if port<-1:
            cmd_err()
        if len(sys.argv)>2:
            config_module_path = sys.argv[2]
            config_path, module_name = os.path.split(config_module_path)
            module_name = os.path.splitext(module_name)[0]
            import importlib
            try:
                sys.path.insert(0, config_path)
                importlib.import_module(module_name)
            except:
                #TODO: Do something better than silently failing here
                pass

    server=PythonCompletionServer(port)
    server.run()
