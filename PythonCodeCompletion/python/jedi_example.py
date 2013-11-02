import jedi
script = '''
def f(x,y,z):
    print x,y,z
f()
print 1
import os
os.
'''
script1='''
import numpy
numpy.'''
script2='''
import pandas
pandas.'''

jedi.settings.cache_directory = 'c:\\jedi-cache'
s = jedi.Script(script1,3,6)
comps = s.completions()

def repeated_import_test():
    import time
    for x in range(200):
        s = jedi.Script(script1,3,6)
        comps = s.completions()
        time.sleep(10)
        for c in comps:
            print c.name
            col = 6+len(c.name+'(')
            s=jedi.Script(script1+c.name+'(',3,col)
            time.sleep(10)

#    s = jedi.Script(script2,3,7)
#    comps = s.completions()
#    for c in comps:
#        print c.name
#        col = 7+len(c.name+'(')
#        s=jedi.Script(script2+c.name+'(',3,col)

def get_definition_location(path,source,line,column):
    source=source.replace('\r','')
    script = jedi.Script(source,line=line+1,column=column,source_path=path)
    results = script.goto_definitions()
    for d in results:
        print dir(d)
        module_path = d.module_path
        print d.line,d.line_nr
        if module_path is None:
            module_path = path
        return [module_path,d.line-1]
    return ['',-1]

def complete_tip(path,source,line,column):
    source=source.replace('\r','')
    script = jedi.Script(source,line=line+1,column=column,source_path=path)
    call_def = script.call_signatures()
    calltip = ''
    for c in call_def:
        calltip=c.call_name+'('
        calltip+=', '.join([p.get_name().names[0] for p in c.params])+')'
#            docstr=''
#            for p in c.params:
#                docstr+=p.docstr
#            if docstr!='':
#                calltip+='\n'+docstr
        return calltip
    return ''

#print complete_tip(None,script,3,2)


#s = jedi.Script(script,line=4,column=2)
#cs = s.call_signatures()
#for m in dir(cs[0]):
#    print m
#
#for p in cs[0].params:
#    print p.get_name().names, p.docstr
#

#for m in dir(cs[0].params[0]):
#    print m

#print cs[0].call_name
#print cs[0].module
#for m in dir(cs[0].params[0].get_name()):
#    print m
