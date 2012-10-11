
def list2str(list,sep='\t'):
    '''Takes a list of numbers and returns a single string conversion
    of those numbers separated by sep.'''
    return sep.join([str(x) for x in list])

# tolerance for floating point comparison
tol = 1e-5


def equals(val1, val2, matchint=False, matchfloat=False):
    ''' compares two values for equality
    forces integer comparison if matchint is true
    forces floating point comparison if matchfloat is true (using tolerance tol)
    '''
    if matchint:
        return int(val1)==int(val2)
    elif matchfloat:
        tolmul=abs((float(val1)+float(val2))*tol)
        return float(val1)-tolmul<float(val2)<float(val1)+tolmul
    else:
        return val1==val2 #Could compare str(val1) with str(val2) instead


def greater(val1, val2, matchint=False, matchfloat=False):
    ''' returns val1 > val2
    forces integer comparison if matchint is true
    forces floating point comparison if matchfloat is true (using tolerance tol)
    '''
    if matchint:
        return int(val1)>int(val2)
    elif matchfloat:
        v1=float(val1)
        v2=float(val2)
        return v1>v2+tol*abs(v1+v2)
    else:
        return val1>val2 #Could compare str(val1) with str(val2) instead


def convert(val,vtype):
    if vtype=='s':
        return str(val)
    if vtype=='i':
        return int(val)
    if vtype=='f':
        return float(val)
    return val


SEEK_SET = 0 #for compatibility of previous python versions with python 2.5 naming convention for file start position


# TODO: Let user specify autobind on import
#delimrow=rowfactory()

#TODO: type conversion of fields using a mask
#TODO:

class delimfile_iter:
    '''
    an iterator for the delimfile class, reads one row at a time sequentially from
    current start position. Limits rows read to numrows (unlimited if numrows is set to -1)
    '''
    file=None
    rowsread=0
    numrows=-1
    rowincrement=1
    def __init__(self,delimfile,numrows=-1):
        self.file=delimfile
        self.numrows=numrows
        if numrows<0:
            self.rowincrement=0
            self.rowsread=-2
    def __iter__(self):
        return self
    def next(self):
        if self.rowsread>=self.numrows:
            raise StopIteration
        row=self.file.readrow()
        if row:
            self.rowsread+=self.rowincrement
            return row
        else:
            raise StopIteration

class delimfile(file):
    '''
    class to read and write fields from text delimited files
    inherits from the standard file class but allows cute things like binding field names of read rows
    to attributes of delimrow
    '''
    currentrow=None #buffers the current row of fields - for use in iterative reads
    delim='\t' #text delimiter character separating fields
    names=[] #list of field names - usually taken from the header. Names are assigned to any returned delimrows
    rowloc=0 #position in the file - the next record read or written will have this index, -1 == EOF
    headloc=0 # stores the position of the first row of data in the file (0 if no header row)
    maxiterrows=-1 #maximum number of rows to read with the iterator, -1 = no maximum
    delimrow=None #a class representing a delimited row object (set of fields from a row)

    def __init__(self, name, writemode=False, header=True, delim='\t', autobind=True):
        if writemode:
            file.__init__(self, name, 'w')
        else:
            file.__init__(self, name, 'r')
            if header:
                self.names=self.readline().strip('\n').split(delim)
                self.headloc=self.tell()
            else:
                self.names=[]
                self.headloc=0
        self.delim=delim
        self.delimrow=self.rowfactory(autobind)

    def __iter__(self):
        ''' defines an iterable to allow user to call:
            df=delimfile('filename.txt')
            for rows in df:
                print rows
            the iterable limits the number of rows read to self.maxiterrows (-1 = no limit)
        '''
        return delimfile_iter(self,self.maxiterrows)

    def reset(self):
        '''
        set file position to start of data (i.e. after the header)
        '''
        self.seek(self.headloc,SEEK_SET)

    def fieldindex(self, name):
        '''
        returns the index position of a field named name, or None if not found
        '''
        if name in self.names:
            return self.names.index(name)

    def __getitem__(self, index):
        '''
        allows user to call delimfile[rownum] to read a particular row number or delimfile[start:stop:step] to read a slice of rows
        CAUTION: highly inefficient algorithm since it always seeks from the beginnning
        '''
        # TODO: come up with a fast means of reading lines in reverse
        if isinstance(index,slice):
            self.seek(self.headloc,SEEK_SET)
            start=index.start
            if not start:
                start=0
            if start>0:
                for i in range(index):
                    self.readline()
            l=[]
            count=start
            for row in self:
                l.append(row)
                count+=1
                if index.stop and count>=index.stop:
                    break
                if index.step:
                    for skips in xrange(index.step-1):
                        self.readrow()
                    count+=index.step-1
                    if index.stop and count>=index.stop:
                        break
            if index.stop:
                self.rowloc=index.stop+1
            else:
                self.rowloc=-1
            return l
        if isinstance(index,int):
            self.seek(self.headloc,SEEK_SET)
            if index>0:
                for i in range(index):
                    self.readline()
            self.rowloc=index+1
            return self.readrow()

    def readrow(self):
        '''
        reads one row of data and returns a delimrow containing named fields
        note that the named fields in the returned delimrow is merely a reference
        (not a copy) to the names list in the delimfile object. Thus if you change the
        names in the returned delimrow, they will alse be changed in the delimfile object
        '''
        if self.currentrow: #if there is a buffered row, just return it and empty the buffer
            row=self.currentrow
            self.currentrow=None
            return row
        rowstr=self.readline().strip('\n')
        if rowstr!='':
            self.rowloc+=1
            return self.delimrow(None,rowstr.split(self.delim),self.names)
        else:
            self.rowloc=-1
            return self.delimrow()

    def writerow(self, row):
        self.write(list2str(row,self.delim)+'\n')
        self.rowloc+=1

    def seekrow(self, matchind, matchval, matchint=False, matchfloat=False):
        '''
        seeks the first row from the current position forward where the field
        indexed by matchind matches matchval. Will seek to end of file if match not found
        '''
        if self.currentrow!=None:
            row=self.currentrow
            self.currentrow=None
        else:
            row=self.readrow()
        id=matchval
        while row and not equals(row[matchind],id,matchint,matchfloat):
            row=self.readrow()
        return row

    def seekrow_sorted(self, matchind, matchval, matchint=False, matchfloat=False):
        '''
        seeks the first row from the current position forward where the field
        indexed by matchind matches matchval. returns the first row where a match occurs,
        or if no match, returns None and buffers the next row of fields at the logical position
        of the missing row. Assumes file is sorted on field matchind
        '''
        if self.currentrow!=None:
            row=self.currentrow
            self.currentrow=None
        else:
            row=self.readrow()

        id=matchval
        while row and greater(id,row[matchind],matchint,matchfloat):
            row=self.readrow()
        if row:
            if greater(row[matchind],id,matchint,matchfloat):
                self.currentrow=row
                return self.delimrow()
        return row

    def readrows(self, matchind, matchint=False, matchfloat=False):
        '''
        reads contiguous rows of data from current file position
        where contents of the field at location matchind is
        identical to the coresponding field in current row - returns a compound list
        of the fields and rows.
        '''
        if self.currentrow!=None:
            row=self.currentrow
            self.currentrow=None
        else:
            row=self.readrow()
        if row==None:
            return

        l=[]
        id=row[matchind]
        while row!=None and equals(row[matchind],id,matchint,matchfloat):
            l.append(row)
            row=self.readrow()
        self.currentrow=row
        return l

    def readrows_seek(self, matchind, matchval, matchint=False, matchfloat=False):
        '''
        reads contiguous rows from current file position where value of field
        indexed by matchind matches matchval and returns a list of delimrows
        containing the names field data. This routine will first seek through
        the file until the first occurence of matchval, returning None if not found.

        matchind = index position of field to match
        matchval = value to match
        matchint=True specifies conversion of string field type to integer before
           comparison (matchval should be integer type in this case)
        matchfloat=True specifies conversion of string field type to float before
           comparison (matchval should be integer type in this case). Comparison uses tolerance
           in global variable tol
        '''
        row=self.seekrow(matchind, matchval, matchint, matchfloat)
        if not row:
            return row
        id=row[matchind]
        l=[row]
        row=self.readrow()
        while row and equals(row[matchind],id,matchint,matchfloat):
            l.append(row)
            row=self.readrow()
        self.currentrow=row
        return l

    def readrows_seeksorted(self, matchind, matchval, matchint=False, matchfloat=False):
        '''
        reads rows where value of field indexed by matchind matches matchval
         and returns a array of delimrows containing the named field data. This routine will seek
         through the file until the first occurence of matchval, returning None
         if not found. If no matching rows found, the file is positioned at the nearest row
         to the missing value. Assumes file is sorted on field matchind. See readrows_seek
        '''
        row=self.seekrow_sorted(matchind, matchval, matchint, matchfloat)
        if not row:
            return row
        id=row[matchind]
        l=[row]
        row=self.readrow()
        while row and equals(row[matchind],id,matchint,matchfloat):
            l.append(row)
            row=self.readrow()
        self.currentrow=row
        return l

    def rowfactory(self,autobind=True):
        '''
        This function returns on of two class definitions for a delimited row that allows user to invoke field names
        as class attributes. Derived from a standard list class where each item in the list represents field data for
        a given row. Each field can have a name and a type (in the string lists names and types). When instancing new delimrows
        users have the option to reference (rather than copy) existing field names and types data by
        calling delimrow(None,values,names,types).
        If autobind is false, the user must bind the field names in the names list using bindnames before they can be invoked
            this copies the data in the list, so changes won't affect the underlying values in the list
        If autobind is true, an extended version of the delimited row class definition is returned that inherits
            the simple class but adds overloads to __getattribute__ and __setattr__ to automatically allow the called
            to invoke field names as attributes of the class (as contained in names). Tradeoff = speed
        '''
        class delimrowi(list):
            def __init__(self,copy=None,fieldvals=None,fieldnames=None,fieldtypes=None):
                '''
                called after construction of a new instance of delimrowi
                if copy contains a delimrow object, a new row that is a distinct copy will be created
                if copy is none, and the user can pass a list of fieldnames and/or fieldtypes that will
                be referenced (not copied)
                '''
                if copy and isinstance(copy,delimrow):
                    list.__init__(self,fieldvals)
                    self[:]=copy[:] # this will call the __getitem__ method of copy
                    if names:
                        self.names[:]=copy.names[:]
                    if types:
                        self.types[:]=copy.types[:]
                    return
                if fieldnames:
                    self.names=fieldnames
                else:
                    self.names=[]
                if fieldtypes:
                    self.types=fieldtypes
                else:
                    self.types=[]

                if not fieldvals:
                    fieldvals=[]
                list.__init__(self,fieldvals)
        #    def __init__(self,fieldvals=None):
        #        self.names=[]
        #        list.__init__(self,fieldvals)
            def __repr__(self):
                if self.names:
                    return list2str(['('+self.names[i]+': '+str(self[i])+')' for i in xrange(len(self))],', ')
                else:
                    return list.__repr__(self)
            def __getslice__(self,i,j): # this is supposed to be deprecated, but apparently it is still implemented for list objects, so we need to override it
                return self.__getitem__(slice(i,j,None))
            def __getitem__(self,key):
                if isinstance(key,str):
                    return list.__getitem__(self,self.names.index(key))
                if isinstance(key,slice):
                    subrow=delimrow()
                    subrow[:]=list.__getitem__(self,key)
                    if self.names:
                        subrow.names[:]=self.names[key]
                    if self.types:
                        subrow.types[:]=self.types[key]
                    return subrow
                if isinstance(key,list): # the list can be either a string list of field names or a list of integer index positions (or a mix)
                    subrow=delimrow()
                    for l in key: # unpredictable results if l is not string or int
                        if isinstance(l,str):
                            ind=self.names.index(l)
                        else:
                            ind=l
                        subrow.append(self[ind])
                        if self.names:
                            subrow.names.append(self.names[ind])
                        if self.types:
                            subrow.types.append(self.types[ind])
                    return subrow
                return list.__getitem__(self,key)
            def __setitem__(self,key,value):
                if isinstance(key,str):
                    list.__setitem__(self,self.names.index(key),value)
                list.__setitem__(self,key,value)
            def setnames(self,namelist,delimchar=','):
                if isinstance(namelist,list):
                    self.names=namelist
                    return
                if isinstance(namelist,str):
                    self.names=namestr.strip(' \n').split(delimchar)
                    return
            def bindnames(self): #this is ridiculously inefficient and redundant. already have getattribute overloaded for autobind True AND dictionary style lookup (i.e. row['fieldname']
                i=0
                for n in self.names:
                    self.__dict__[n]=self[i]
                    i+=1
            def settypes(self,typestr,delimchar=','):
                if isinstance(typestr,str):
                    self.types=typestr.strip(' \n').split(delimchar)
            def coercetypes(self):
                for i in range(len(self)):
                    self[i]=convert(self[i],self.types[i])
            names=list()
            types=list()
        if not autobind:
            return delimrowi
        class delimrow(delimrowi):
            def __getattribute__(self,name):
                try:
                    return delimrowi.__getattribute__(self,name)
                except AttributeError:
                    if name in self.names:
                        return list.__getitem__(self,delimrowi.__getattribute__(self,'names').index(name))
#                        return delimrowi.__getitem__(self,delimrowi.__getattribute__(self,'names').index(name))
                    else:
                        raise AttributeError
            def __setattr__(self,name,value):
                try:
                    delimrowi.__setattr__(self,name,value)
                    return
                except AttributeError:
                    if name in self.names:
                        delimrowi.__setitem__(self,delimrowi.__getattribute__(self,'names').index(name),value)
                        return
                    else:
                        raise AttributeError
        return delimrow
