import jedi

script = '''
def f(x,y,z):
    print x,y,z
f(
'''


def complete_tip(path,source,line,column):
    source=source.replace('\r','')
    script = jedi.Script(source,line=line+1,column=column,source_path=path)
    call_def = script.call_signatures()
    calltip = ''
    print call_def
    for c in call_def:
        docstr=''
        calltip=c.call_name+'('
        for p in c.params:
            calltip+=p.get_name()+', '
            docstr+=p.docstr
        calltip+=')'
        if docstr!='':
            calltip+='\n'+docstr
        return calltip
    return ''

#print complete_tip(None,script,3,2)


s = jedi.Script(script,line=4,column=2)
cs = s.call_signatures()
for m in dir(cs[0]):
    print m

for p in cs[0].params:
    print p.get_name().names, p.docstr


#for m in dir(cs[0].params[0]):
#    print m

#print cs[0].call_name
#print cs[0].module
#for m in dir(cs[0].params[0].get_name()):
#    print m
