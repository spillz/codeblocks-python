import sys
import pkgutil
import imp
import os,os.path
import types
import time
import inspect
import cPickle
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

chtypes=[types.ClassType, types.ObjectType, types.ModuleType] #types.InstanceType
obtypes=[types.ClassType, types.ObjectType] #types.InstanceType

def parse_objs(objs,obj_list,follow=True,prefix=''):
    for o in obj_list:
        if o[0].startswith('_'):
            continue
        args=None
        try:
            doc=str(o[1].__doc__)
        except:
            doc=''
        if type(o[1]) in obtypes:
            ch=inspect.getmembers(o[1])
            if follow:
                print '###follow',prefix+o[0],type(o[1])
                parse_objs(objs,ch,False,prefix+o[0]+'.')
        if type(o[1]) in [types.BuiltinFunctionType, types.BuiltinMethodType, types.FunctionType,
                        types.LambdaType, types.UnboundMethodType, types.MethodType]:
            try:
                args=tuple(inspect.getargspec(o[1]))[:-1]
            except:
                pass
        objs[prefix+o[0]]=[str(type(o[1])),args,doc]

def parse_mods(mods):
    for m in sorted(mods):
        objs={}
        try:
            print m
            a=m.find('.')
            if a>=0:
                mod=__import__(m,fromlist=[m[a+1:]])
            else:
                mod=__import__(m)
        except:
            continue
        parse_objs(objs,inspect.getmembers(mod),True,m+'.')
        mods[m]=objs
#        print '***********'
#        for o in sorted(objs):
#            print o,objs[o][:2]
#        print '***********'
#        print '***********'

def create(dest='STDLIB'):
    '''
    creates the standard lib from path dest
    '''
    mods={}

    get_builtin_mods(mods)
    std_mod_list=[line.strip().split(' ')[0] for line in open("pymods.txt","r").readlines()]
    for m in std_mod_list:
        mods[m]=None
    pkg_mod_list=['gtk','numpy','scipy','pandas','statsmodels']
    get_package_mods(mods,pkg_mod_list)

    print 'MODULES'
    print '##########'
    for m in sorted(mods):
        print m

    parse_mods(mods)

    t=time.time()
    f=open(dest,'wb')
    cPickle.dump(mods,f)
    f.close()
    print 'write took',time.time()-t
    return mods

def load(src='STDLIB'):
    '''
    loads the stdlib from path src
    '''
    try:
        if os.path.exists(src):
            t=time.time()
            f=open(src,'rb')
            mods=cPickle.load(f)
            f.close()
            print 'read took',time.time()-t
            return mods
        else:
            return None
    except:
        return None

if __name__=='__main__':

    create()
#    mods={}
#    mods['sys']={}
#    parse_mods(mods)
#    print 'OS symbols'
#    for s in sorted(mods['sys']):
#        print s#,mods['os.path'][s][-1]

