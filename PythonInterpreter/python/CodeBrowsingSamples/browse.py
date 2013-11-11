import sys
import ast

def walkAST(node,level,parent=""):
    newparent=parent
    if isinstance(node,compiler.ast.Class):
        print " "*level,"C",node.name,":",
        for inode in node.bases:
            try:
                print inode.name,
            except:
                print "***",inode,  #when base class is in another module, inode is a (possible nested) Getattr type
        print
        newparent=node.name
    if isinstance(node,ast.FunctionDef):
        print " "*level,"F",node.name,"(",
        for a in node.argnames:
            print a,
            try:
                defaultval=node.defaults[i].name
                print "=",defaultval,
            except:
                print ",",
        print ")"
    if isinstance(node, ast.Assign):
        try:
            qual=node.attrname
            inode=node.expr
            while isinstance(inode, ast.GetAttr):
                qual=inode.attrname+'.'+qual
                inode=inode.expr
            if inode.name=='self':
                qual=parent+'.'+qual
            else:
                qual=inode.name+'.'+qual
            print " "*level,"AA",qual
        except:
            print 'ERR'
            print " "*level,"AA",node.expr,".",node.attrname
    if isinstance(node, ast.AugAssign):
        print " "*level,"AN",node.name,node
    if isinstance(node, ast.Import):
        print " "*level,"IM",node.names
    if isinstance(node, ast.ImportFrom):
        print " "*level,"IF",node.names
#    if isinstance(node, compiler.ast.If):
#        print " "*level,"IF",node
    try:
        raise
        for inode in node.code:
            walkAST(inode,level+1,newparent)
    except:
        try:
            for inode in node.getChildNodes():
                walkAST(inode,level+1,newparent) #NB: Not incrementing level here...
        except:
            return



class allnames(ast.NodeVisitor):
   def visit_Module(self, node):
     self.names = set()
     self.generic_visit(node)
     print sorted(self.names)
   def visit_Name(self, node):
     self.names.add(node.id)


class SymbolVisitor(ast.NodeVisitor):
    def visit_Module(self, node):
        '''(stmt* body)'''
        self.root={}
        self.pos=self.root
        self.generic_visit(node)
    def visit_FunctionDef(self,node):
        '''(identifier name, arguments args, stmt* body, expr* decorator_list, expr? returns)'''
#        args=None
        pos=self.pos
        self.pos={}
        print  'FN stmts for',node.name
        for nb in node.body:
            try:
                print nb.targets
            except:
                pass
            self.visit(nb)
        pos[node.name]=['F',node.args,ast.get_docstring(node),self.pos]
        self.pos=pos
        '''
        arguments = (arg* args, identifier? vararg, expr? varargannotation,
                     arg* kwonlyargs, identifier? kwarg,
                     expr? kwargannotation, expr* defaults,
                     expr* kw_defaults)'''
    def visit_ClassDef(self,node):
        '''(identifier name,
             expr* bases,
             keyword* keywords,
             expr? starargs,
             expr? kwargs,
             stmt* body,
             expr* decorator_list)
             '''
        args=None
        pos=self.pos
        self.pos={}
        for nb in node.body:
            self.visit(nb)
        pos[node.name]=['C',args,ast.get_docstring(node),self.pos]
        self.pos=pos
    def visit_Assign(self,node):
        '''(expr* targets, expr value)'''
 #       print 'A',node.targets,node.value
        for t in node.targets:
            if isinstance(t,ast.Name):
                self.pos[t.id]=['A',t.ctx,None,None]
            elif isinstance(t,ast.Attribute):
                'Attribute(expr value, identifier attr, expr_context ctx)'
                #probably should only use this to define class attributes that are set in __init__
                if isinstance(t.value,ast.Name):
                    if t.value.id=='self': ##TODO: not always 'self' -- should be whatever the first arg of the class method is
                        self.pos[t.attr]=('CA',None,None,None)
            elif isinstance(t, ast.Tuple):
                for el in t.elts:
                    if isinstance(t,ast.Name):
                        self.pos[t.id]=['A',t.ctx,None,None]
            else :
                print 'UNKOWN ATTR',t
    def visit_Import(self,node):
        '''(alias* names)'''
        names=[]
        for n in node.names:
            if n.asname == None:
                name=n.name
            else:
                name=n.asname
            self.pos[name]=['I',n.name,None,None]
    def visit_ImportFrom(self,node):
        '''(identifier? module, alias* names, int? level)'''
        names=[]
        for n in node.names:
            if n.asname == None:
                name=n.name
            else:
                name=n.asname
            self.pos[name]=['I',n.name,ast.get_docstring(Node),None]

if __name__=='__main__':
    print "PYTHON SOURCE BROWSER v0.0001"
    if len(sys.argv)!=2:
        fname='function_attribute.py'
    #    print "Correct usage: \n\nbrowse.py <python file>\n\nor:\n\npython browse.py <python file>"
    #    exit(1)
    else:
        fname=sys.argv[1]

    mod=ast.parse(open(fname,'r').read())
    print ast.dump(mod)

    allnames().visit(mod)
    sv=SymbolVisitor()
    sv.visit(mod)
    for s in sv.pos:
        print s,sv.pos[s]


    import sys
    sys.exit()
    print "MODULE\n-----------------------------------"
    print fname
    print "\nMODULE DOCSTRING\n-----------------------------------"
    print mod.doc
    print "\n\nMODULE CONTENTS\n-----------------------------------"
    for outernode in mod.node.nodes:
        walkAST(outernode,0)

    #NB: use sys.builtin_module_names to return the list of modules
