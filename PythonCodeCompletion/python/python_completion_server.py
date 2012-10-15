import code
import sys
import os.path
import time
import types
import struct
import xmlrpclib
from SimpleXMLRPCServer import SimpleXMLRPCServer
#from pysmell import idehelper
#from pysmell import tags
from re import split

from rope.contrib import codeassist
import rope.base.project

'''
     |                    | instance | class | function | module | None
     |              local |    +     |   +   |    +     |   +    |
     |             global |    +     |   +   |    +     |   +    |
     |            builtin |    +     |   +   |    +     |        |
     |          attribute |    +     |   +   |    +     |   +    |
     |           imported |    +     |   +   |    +     |   +    |
     |            keyword |          |       |          |        |  +
     |  parameter_keyword |          |       |          |        |  +
'''

'''
type_map = {
str(types.ModuleType) : 1,
str(types.ClassType) : 2,
str(types.ObjectType) : 3,
str(types.TypeType) : 4,
str(types.InstanceType) : 5,
str(types.FunctionType) : 6,
str(types.MethodType) : 7,
str(types.UnboundMethodType) : 8,
str(types.BuiltinFunctionType) : 6,
str(types.BuiltinMethodType) : 7,
str(types.LambdaType) : 9,
str(types.GeneratorType) : 10 }
'''

def type_suffix(o):
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

def _uniquify(l):
    found = set()
    for item in l:
        if item not in found:
            yield item
        found.add(item)

isz=struct.calcsize('@L')

class XmlRpcPipeServer:
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
        size = struct.unpack('@L',size_buf)[0]
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
            res_xml='Error running call'+name+call_xml+'\n'
            res_xml+='\n'.join(traceback.format_exception(*sys.exc_info()))
        size = len(res_xml)
        self.outpipe.write(struct.pack('@L',size))
        self.outpipe.write(res_xml)
    def __call__(self,name,*args):
        return self.fn_dict[name](*args)


class PyCompletionServer:
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
        while not self._quit:
            self.server.handle_request()
    def notify_active_path(self,path):
        if self.active_path==os.path.split(path)[0]:
            return True
        if not os.path.exists(path):
            return False
        fdir,fname = os.path.split(path)
        self.active_path=fdir
        try:
            self.project = rope.base.project.Project(fdir)
        except:
            return False
        return True
    def complete_phrase(self,path,source,position):
#        print 'complete context',path,position
#        if '\r' in source:
#            print 'replacing CRs'
#            source=source.replace('\r\n',' \n').replace('\r','\n')
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
#        source=source.replace('\r\n',' \n').replace('\r','\n')
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
    def end(self):
        self._quit=True
        return True

def cmd_err():
    print 'Correct usage: python_completion_server.py <port>'
    print '<port> must be a positive integer or -1 to use stdin/stdout'
    exit()

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

    server=PyCompletionServer(port)
    server.run()
