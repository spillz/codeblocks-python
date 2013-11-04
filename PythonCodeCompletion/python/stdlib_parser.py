import sys
import pkgutil
import imp
import os,os.path
import types
import time
import inspect
import cPickle
import json
del sys.path[0]

def get_builtin_mods(mods):
    builtins = set(sys.builtin_module_names)
    for m in builtins:
        mods[m]=imp.find_module(m)
    return mods

def get_package_mods(mods,mod_list,prefix='',path=None,level=0):
    for m in mod_list:
        try:
            fp, pathname, description = imp.find_module(m,path)
            mod = __import__(prefix+m)
            ##mod = imp.load_module(m, fp, pathname, description)
        except:
            import traceback
            print prefix,m,path
            print traceback.format_exception(*sys.exc_info())
            #traceback.print_exception(*sys.exc_info())
            print 'ERROR: skipping',prefix+m
            continue
        try:
            chpath=mod.__path__
        except:
            chpath=None
        fullname=prefix+m
        mods[fullname]=None
        chmods=pkgutil.iter_modules([pathname])
        mlist=[n[1] for n in chmods if not n[1].startswith('_') and not n[1].startswith('test') and not n[1].startswith('try')]
        get_package_mods(mods,mlist,fullname+'.',chpath,level+1)

chtypes=[types.ClassType, types.ObjectType, types.ModuleType, types.TypeType] #types.InstanceType
obtypes=[types.ClassType, types.ObjectType, types.TypeType] #types.InstanceType

def parse_objs(symbols,obj_list,follow=True,prefix=''):
    if(type(symbols)!=types.DictType):
        print '***NOT DICT',prefix,type(symbols)
    for o in obj_list:
        if o[0].startswith('_'):
            continue
        chsyms=None
        args=None
        path = None
        line = None
        try:
            doc=str(o[1].__doc__)
        except:
            doc=''
        try:
            path = inspect.getsourcefile(o[1])
        except TypeError:
            pass
        try:
            lines,line = inspect.getsourcelines(o[1])
        except:
            pass
        try:
            o[1].__bases__
            has_bases=True
        except:
            has_bases=False
        if type(o[1]) in obtypes or has_bases:
            ch=inspect.getmembers(o[1])
            if follow:
                print '###follow',prefix+o[0],type(o[1])
                chsyms={}
                parse_objs(chsyms,ch,False,prefix+o[0]+'.')
        if type(o[1]) in [types.BuiltinFunctionType, types.BuiltinMethodType, types.FunctionType,
                        types.LambdaType, types.UnboundMethodType, types.MethodType]:
            try:
                a=inspect.getargspec(o[1])
                args=', '.join(a.args)
                if a.varargs is not None:
                    args+= ', *'+a.varargs
                if a.keywords is not None:
                    args+= ', **'+a.keywords
            except:
                pass
        symbols[o[0]]=(str(type(o[1])),args,path,line,doc,chsyms)

def parse_mods(mods):
    symbols={}  ##Symbol is formatted key:value key=symbol, value=tuple(type,args,docstring,children)
    for m in sorted(mods):
        try:
            print m
            a=m.find('.')
            if a>=0:
                mod=__import__(m,fromlist=[m[a+1:]])
            else:
                mod=__import__(m)
        except:
            continue
        msyms=m.split('.')
        psymbols=symbols
        for s in msyms:
            if s not in psymbols:
                psymbols[s]=[str(types.ModuleType),None,mod.__doc__,{}]
            elif type(psymbols[s][-1])!=types.DictType:
                print 'WARNING FOUND',msyms,type(psymbols[s][-1])
                psymbols[s]=[str(types.ModuleType),None,mod.__doc__,{}]
            psymbols=psymbols[s][-1]
        parse_objs(psymbols,inspect.getmembers(mod),True,m+'.')
    return symbols


def create(dest='STDLIB',use_json=True):
    '''
    creates the standard lib from path dest
    '''
    mods={}

    #get_builtin_mods(mods)
    #std_mod_list=[line.strip().split(' ')[0] for line in open("pymods.txt","r").readlines()]
    #for m in std_mod_list:
    #    mods[m]=None
    #pkg_mod_list=['gtk','numpy','scipy','pandas','statsmodels']
    pkg_mod_list=['numpy']
    get_package_mods(mods,pkg_mod_list)

    print 'MODULES'
    print '##########'
    for m in sorted(mods):
        print m

    syms=parse_mods(mods)
    for s in syms:
        print s

    t=time.time()
    f=open(dest,'wb')
    if use_json:
        f.write(json.dumps(syms))
    else:
        cPickle.dump(syms,f)
    f.close()
    print 'write took',time.time()-t
    return syms

def load(src='STDLIB',use_json=True):
    '''
    loads the stdlib from path src
    '''
    try:
        if os.path.exists(src):
            t=time.time()
            f=open(src,'rb')
            if use_json:
                mods=json.loads(f.read())
            else:
                mods=cPickle.load(f)
            f.close()
            print 'read took',time.time()-t
            print 'main scope'
            for s in sorted(mods):
                print s
            return mods
        else:
            return None
    except:
        return None

if __name__=='__main__':
#    symbols=create()
    syms=load()
    print 'numpy' in syms
    api = syms['numpy'][-1]
    for s in sorted(api):
        print s,api[s][:-2]
#    ols = api['OLS'][-1]
#    for s in sorted(ols):
#        print s,ols[s][:-2]
#    datasets = api['datasets'][-1]
#    datasets2 = syms['statsmodels'][-1]['datasets'][-1]
#    for s in sorted(datasets):
#        print s
#    for s in sorted(datasets2):
#        print s
#    print datasets == datasets2
