import code
import sys
import threading
import time
from SimpleXMLRPCServer import SimpleXMLRPCServer

DEBUG=False

if DEBUG:
    def logmsg(msg,*kargs):
        print >>sys.__stdout__,msg,
        for x in kargs:
            print >>sys.__stdout__,x,
        print >>sys.__stdout__,''
else:
    def logmsg(msg,*kargs):
        return

class datastore:
    def __init__(self):
        self.data=''
        self.lock=threading.Condition(threading.Lock())
        self.inputrequest=False
    def write(self,textstr):
        self.lock.acquire()
        self.data=self.data+textstr
        self.lock.release()
    def flush(self):
        self.lock.acquire()
        self.data=''
        self.lock.release()
    def read(self,size=None):
        self.lock.acquire()
        data=self.data ##TODO: should this be before the lock?
        self.data=''
        self.lock.release()
        return data
        if size:
            self.lock.acquire()
            while 1:
                if(len(self.data)>=size):
                    line=self.data[:size]
                    self.data=self.data[size:]
                    self.lock.release()
                    logmsg('received',size,' chars of text:',line)
                    return line
                self.inputrequest=True
                self.lock.wait()
                self.inputrequest=False
    def readline(self):
        #check data for a full line (terminated by \n or EOF(?))
        #if the line is there, extract it and return
        #if not a complete line, set a request for input from the client
        #  with a hint containing the current data in the line
        #  then wait on a mutex
        ## TODO: Could optionally raise a keyboard interrupt
        self.lock.acquire()
        while 1:
            ind=self.data.find('\n')
            if ind>=0:
                line=self.data[:ind+1]
                self.data=self.data[ind+1:]
                self.lock.release()
                logmsg('received line of text:',line)
                return line
            self.inputrequest=True
            self.lock.wait()
            self.inputrequest=False
    def HasInputRequest(self):
        self.lock.acquire()
        a=self.inputrequest
        self.lock.release()
        return a
    def InputRequestNotify(self):
        self.lock.acquire()
        self.lock.notify()
        self.lock.release()


class PyInterp (code.InteractiveInterpreter):
    def __init__(self,lock):
        code.InteractiveInterpreter.__init__(self)
        self._stdout=datastore()
        self._stdin=datastore()
        self._stderr=datastore()
        self._running=True
        self._runningeval=False
        self.lock=lock
        self.eval_str=''
    def queue_code(self,eval_str):
        if not self._runningeval:
            self.lock.acquire()
            self.eval_str=eval_str
            self._runningeval=True
            self.lock.notify()
            self.lock.release()
            return True
        else:
            return False
    def main_loop(self):
        while self._running: #runs the eval_str queued by the server, then waits for the next eval_str. the loop ends when the server requests exit
            try:
                if self.eval_str!='':
                    logmsg('running code '+self.eval_str)
                    try:
                        self.runsource(self.eval_str+'\n')
                        logmsg('ran code')
                    except KeyboardInterrupt:
                        print 'Keyboard Interrupt'
                    except:
                        print "error in eval_str", sys.exc_info()[0]
                self.lock.acquire() #acquire the lock to reset eval string and running status
                self._runningeval=False
                self.eval_str=''
                self.lock.notify() #notify the server in case it is waiting
                self.lock.wait() #now await the next instruction
                self.lock.release()
            except KeyboardInterrupt:
                logmsg('keyboard interrupt')
                print 'Keyboard Interrupt'

class AsyncServer(threading.Thread):
    def __init__(self,port):
        # Create XMLRPC server
        threading.Thread.__init__(self)
        self.lock=threading.Condition(threading.Lock())
        self.timeout=0.2
        self._quit=False
        self.port=port
        self.interp = PyInterp(self.lock)
        sys.stdout=self.interp._stdout
        sys.stdin=self.interp._stdin
        sys.stderr=self.interp._stderr
    def start_interp(self):
        self.interp.main_loop()
    def run(self):
        self.server = SimpleXMLRPCServer(("localhost", self.port))
        self.server.logRequests=0
        self.server.register_introspection_functions()
        #self.server.socket.settimeout(self.timeout) ##timeouts cause probs
        self.server.register_function(self.end,'end')
        self.server.register_function(self.run_code,'run_code')
        self.server.register_function(self.cont,'cont')
        while not self._quit:
            self.server.handle_request()
    def end(self):
        if self.interp._runningeval:
            raise KeyboardInterrupt
        self.lock.acquire()
        self.interp._running=False
        self._quit=True
        self.lock.notify()
        self.lock.release()
        return "Session Terminated"
    def run_code(self,eval_str,stdin):
        logmsg("compiling code")
        try:
            cobj=code.compile_command(eval_str)
        except:
            logmsg("syntax error")
            return -1,'','',False
        if cobj==None:
            logmsg("statement incomplete")
            return -2,'','',False
        logmsg("running code "+eval_str)
        if self.interp.queue_code(eval_str):
            return self.cont(stdin)
        else:
            return -3,self.interp._stdout.read(),self.interp._stderr.read(),self.interp._stdin.HasInputRequest()
    def cont(self,stdin):
        logmsg('continuing with stdin: '+stdin)
        self.interp._stdin.write(stdin)
        if self.interp._stdin.HasInputRequest():
            self.interp._stdin.InputRequestNotify()
        self.lock.acquire()
        if self.interp._runningeval:
            self.lock.wait(self.timeout)
        self.lock.release()
        result=(int(self.interp._runningeval),self.interp._stdout.read(),self.interp._stderr.read(),self.interp._stdin.HasInputRequest())
        logmsg('returning result ',result)
        return result
        #return status, stdout, stderr

def cmd_err():
    print 'Correct usage: pyinterp.py <port>'
    print '<port> must be a positive integer'
    exit()

if len(sys.argv)!=2:
    cmd_err()
try:
    port = int(sys.argv[1])
except:
    cmd_err()
if port<=0:
    cmd_err()

logmsg('starting server on port', port)

interp_server=AsyncServer(port)
interp_server.start()
interp_server.start_interp()

logmsg('server terminated')
