import sys
import StringIO

output = StringIO.StringIO()
sys.stdout= output
print "written secretly"
sys.stdout=sys.__stdout__
outval=output.getvalue()
print "the output was",outval

