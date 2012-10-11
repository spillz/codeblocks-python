import sys
import compiler,compiler.ast

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
    if isinstance(node,compiler.ast.Function):
        print " "*level,"F",node.name,"(",
        for a in node.argnames:
            print a,
            try:
                defaultval=node.defaults[i].name
                print "=",defaultval,
            except:
                print ",",
        print ")"
    if isinstance(node, compiler.ast.AssAttr):
        try:
            if node.expr.name=='self':
                expr=parent
            else:
                expr=node.expr.name
            print " "*level,"AA",expr,".",node.attrname
        except:
            print " "*level,"AA",node.expr,".",node.attrname
    if isinstance(node, compiler.ast.AssName):
        print " "*level,"AN",node.name
    try:
        for inode in node.code:
            walkAST(inode,level+1,newparent)
    except:
        try:
            for inode in node.nodes:
                walkAST(inode,level,newparent) #NB: Not incrementing level here...
        except:
            return


print "PYTHON SOURCE BROWSER v0.0001"
if len(sys.argv)!=2:
    print "Correct usage: \n\nbrowse.py <python file>\n\nor:\n\npython browse.py <python file>"
    exit(1)

mod=compiler.parseFile(sys.argv[1])
print "MODULE\n-----------------------------------"
print sys.argv[1]
print "\nMODULE DOCSTRING\n-----------------------------------"
print mod.doc
print "\n\nMODULE CONTENTS\n-----------------------------------"
for outernode in mod.node.nodes:
    walkAST(outernode,0)

#NB: use sys.builtin_module_names to return the list of modules
