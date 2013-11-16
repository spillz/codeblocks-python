# makefile written for gnu make
CXX		= g++
SRC		= ./src
INCLUDE		= ./include
CPPFLAGS	= -I$(INCLUDE)
DEBUG		= -g
OPTIMIZE	= -O2
GCCWARN		= 
CXXFLAGS	= $(DEBUG) $(GCCWARN) $(OPTIMIZE) $(INCLUDES)

LIB		= ./libXmlRpc.a

# Add your system-dependent network libs here. These are
# only used to build the tests (your application will need them too).
# Linux: none
# Solaris: -lsocket -lnsl
#SYSTEMLIBS	= -lsocket -lnsl
SYSTEMLIBS	=
LDLIBS		= $(LIB) $(SYSTEMLIBS)

OBJ		= $(SRC)/XmlRpcClient.o $(SRC)/XmlRpcDispatch.o \
		$(SRC)/XmlRpcServer.o $(SRC)/XmlRpcServerConnection.o \
		$(SRC)/XmlRpcServerMethod.o $(SRC)/XmlRpcSocket.o $(SRC)/XmlRpcSource.o \
		$(SRC)/XmlRpcUtil.o $(SRC)/XmlRpcValue.o

all:		$(LIB)

$(LIB):		$(OBJ)
		$(AR) $(ARFLAGS) $(LIB) $(OBJ)


tests:		$(LIB)
		cd test && $(MAKE) CXX=$(CXX) CXXFLAGS="$(CXXFLAGS)" SYSTEMLIBS="$(SYSTEMLIBS)"

doc doxygen:
		cd src && doxygen Doxyfile

clean:
		rm -f $(SRC)/*.o
		rm -f $(SRC)/*~
		rm -f $(LIB)
		cd test && $(MAKE) clean

