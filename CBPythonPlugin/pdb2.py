"""An extension to the Python debugger for parsing debugger output in Code::Blocks"""
#TO DO: figure out a way to separate stdin for program vs debugger
#Maybe take debugger input from a stream


#TO DO: eliminate the imports that aren't used
import sys
import linecache
import cmd
import bdb
import os
import re
import pprint
import traceback
from pdb import *

class prettyout(object):
    def write(fd,str):
        decoratoropen="<&CB_PDB"
        decoratorclose="CB_PDB&>"
#        strout=str.replace("\n",decorator+"\n")
        sys.stdout.write(decoratoropen+str+decoratorclose)
    def flush(fd):
        sys.stdout.flush()


def main():
    pout=prettyout() #TODO: Derive a class from sys.stdout and handle its write method to add decoration for easy parsing in C::B
    if not sys.argv[1:]:
        print >>pout, "usage: pdb.py scriptfile [arg] ..."
        sys.exit(2)

    mainpyfile =  sys.argv[1]     # Get script filename
    if not os.path.exists(mainpyfile):
        print >>pout, 'Error:', mainpyfile, 'does not exist'
        sys.exit(1)

    del sys.argv[0]         # Hide "pdb.py" from argument list

    # Replace pdb's dir with script's dir in front of module search path.
    sys.path[0] = os.path.dirname(mainpyfile)

    # Note on saving/restoring sys.argv: it's a good idea when sys.argv was
    # modified by the script being debugged. It's a bad idea when it was
    # changed by the user from the command line. The best approach would be to
    # have a "restart" command which would allow explicit specification of
    # command line arguments.
    pdb = Pdb(stdout=pout)
    while 1:
        try:
            pdb._runscript(mainpyfile)
            if pdb._user_requested_quit:
                break
            print >>pout, "The program finished and will be restarted"
        except SystemExit:
            # In most cases SystemExit does not warrant a post-mortem session.
            print >>pout,  "The program exited via sys.exit(). Exit status: ",
            print >>pout,  sys.exc_info()[1]
	except KeyboardInterrupt:
            print >>pout, "Keyboard Interrupt: The program finished and will be restarted"
            #ideally would want program to continue here...
        except:
            traceback.print_exc(pout)
            print >>pout,  "Uncaught exception. Entering post mortem debugging"
            print >>pout,  "Running 'cont' or 'step' will restart the program"
            t = sys.exc_info()[2]
            while t.tb_next is not None:
                t = t.tb_next
            pdb.interaction(t.tb_frame,t)
            print >>pout,  "Post mortem debugger finished. The "+mainpyfile+" will be restarted"


# When invoked as main program, invoke the debugger on a script
if __name__=='__main__':
    main()
