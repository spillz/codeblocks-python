This file contains source browsing command (browse.py) operating on 
one python file at a time. A couple of sample python files are included.

This routine cannot be used to browse python builtins or python compiled modules. To do this probably requires something like:

#Either construct a database of builtins OR
#alternatively, it might be better to simply search in real time through the known builts-in or loaded modules

#Iterate through builtins
for obj in dir(__builtins__):
   print obj, eval("type("+mod+"."+obj+")")
   #recurse on classes/functions to get methods/attributes etc

#Iterate through compiled modules
import sys
for mod in sys.builtin_module_names:
    import mod
    for obj in dir(mod):
       print obj, eval("type("+mod+"."+obj+")")
       #could do this recursively on classes/functions to get methods/attributes etc
    #close module??



