import sys
import os
import struct
import xmlrpclib
from SimpleXMLRPCServer import SimpleXMLRPCServer
from rope.contrib import codeassist
import rope.base.project

'''
Rope type/scope combinations
                                            type
                scope
     |                    | instance | class | function | module | None
     |              local |    +     |   +   |    +     |   +    |
     |             global |    +     |   +   |    +     |   +    |
     |            builtin |    +     |   +   |    +     |        |
     |          attribute |    +     |   +   |    +     |   +    |
     |           imported |    +     |   +   |    +     |   +    |
     |            keyword |          |       |          |        |  +
     |  parameter_keyword |          |       |          |        |  +
'''

def type_suffix(o):
    #TODO: Add type/scope combinations here adding more bitmaps to the list in the plugin
    t = o.type
    s = o.scope
    if t=='instance':
        return '?'+str(5)
    elif t=='class':
        return '?'+str(2)
    elif t=='function':
        return '?'+str(6)
    elif t=='module':
        return '?'+str(1)
    elif s=='keyword':
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
            res_xml = bytes(xmlrpclib.dumps(result, methodresponse=True))
        except:
            import traceback
            res_xml='Error running call'+name+'\n'+call_xml+'\n'
            res_xml+='\n'.join(traceback.format_exception(*sys.exc_info()))
        size = len(res_xml)
        self.outpipe.write(struct.pack('I',size))
        self.outpipe.write(res_xml)

    def __call__(self,name,*args):
        return self.fn_dict[name](*args)


class PythonCompletionServer:
    '''
    An XMLRPC server that serves up a rope interface
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

    def notify_active_path(self,path):
        fdir,fname = os.path.split(path)
        if self.project!=None and self.active_path==fdir:
            return True
        if not os.path.exists(path):
            return False
        self.active_path=fdir
        try:
            if os.path.exists(os.path.join(fdir,'.ropeproject')):
                self.project = rope.base.project.Project(fdir)
            else:
                self.project = rope.base.project.Project(fdir, ropefolder=None)
        except:
            self.project=None
            self.active_path=None
            return False
        return True

    def complete_phrase(self,path,source,position):
#        print 'complete context',path,position
        if path!=None:
            if not self.notify_active_path(path):
                return []
        if self.project==None:
            return []
        results = codeassist.code_assist(self.project, source, position)
        completions=sorted([s.name+type_suffix(s) for s in results])
        return completions

    def complete_tip(self,path,source,position):
#        print 'complete tip',symbol
        if path!=None:
            if not self.notify_active_path(path):
                return []
        if self.project==None:
            return []
        calltip = codeassist.get_calltip(self.project, source, position)
        if calltip==None:
            calltip=''
        doc = codeassist.get_doc(self.project, source, position)
        if doc is not None:
            lines=doc.split('\n')
            del lines[0] ##rope inserts its own header in the doc string
            doc=doc.strip(' \t\n')
            fnfull = calltip[:calltip.find('(')]
            fn = calltip[fnfull.find('.'):]
            if not (fnfull and lines[0].startswith(fnfull)):
                if not (fn and lines[0].startswith(fn)):
                    lines.insert(0,calltip)
            if lines[0].strip()[-1]!=':':
                lines[0]=lines[0]+':'
            shortened=False
            if len(lines)>6:
                shortened=True
                doc='\n'.join(lines[:8])
            else:
                doc='\n'.join(lines)
            if len(doc)>250:
                shortened=True
                doc=doc[:250]
            if shortened:
                doc+='...'
            calltip=doc
        if calltip==None:
            calltip='<unknown function>'
        return calltip

    def get_definition_location(self,path,source,position):
        if path!=None:
            if not self.notify_active_path(path):
                return ['',-1]
        resource,lineno = codeassist.get_definition_location(self.project,source,position)
        if lineno == None:
            return ['',-1]
        if resource == None:
            return [path,lineno]
        return [resource.path,lineno]

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
