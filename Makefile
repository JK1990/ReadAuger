# The CDAS Library
LIBPATH=$(CDASHOME)/lib
LIBOFFLINEPATH=$(AUGEROFFLINEROOT)/lib
LIBCLHEPPATH=$(CLHEPHOME)/lib
LIBBOOSTPATH=$(BOOSTROOT)/lib
LIB_AUGER=-lEr -lEs -lIoAuger -lIoSd -lSTCoordinates -lMoIO
# Set this to the path where the headers have been installed
INCPATH=$(CDASHOME)/include
INCOFFLINEPATH=$(AUGEROFFLINEROOT)/include
INCCLHEPPATH=$(CLHEPHOME)/include
INCBOOSTPATH=$(BOOSTROOT)/include
FD_INCLUDE=$(FD_ROOT)/include


PROG=readAuger

GPP=g++
CFLAGS=-Wall -c -I$(INCPATH) -I$(INCOFFLINEPATH) -I$(INCCLHEPPATH) -I$(INCBOOSTPATH) -I$(shell root-config --incdir) -I$(FD_INCLUDE) -std=c++11
LDFLAGS=-Wall $(LIB_AUGER) -L$(LIBPATH) -L$(LIBOFFLINEPATH) -L$(LIBCLHEPPATH) -L$(LIBBOOSTPATH) $(shell root-config --libs) -lboost_filesystem -lboost_system

.PHONY:	all clean

all:			$(PROG)
			@echo "done."

$(PROG):	$(PROG).o readMC.o readRaw.o intChecker.o
	@echo -n ">>> Linking   "
	$(GPP) $(PROG).o readMC.o readRaw.o intChecker.o $(shell auger-offline-config --ldflags) -L$(LIBPATH) $(LIB_AUGER) -lboost_filesystem -lboost_system -std=c++11 -o $(PROG)

$(PROG).o:    $(PROG).cc
	@echo -n ">>> Compiling "
	$(GPP) $(shell auger-offline-config --cppflags) --std=c++11 -c $(PROG).cc

readMC.o:	readMC.cc readMC.h
	@echo -n ">>> Compiling "
	$(GPP) $(shell auger-offline-config --cppflags) --std=c++11 -c readMC.cc

readRaw.o:	readRaw.cc readRaw.h
	@echo -n ">>> Compiling "
	$(GPP) $(shell auger-offline-config --cppflags) --std=c++11 -c readRaw.cc

intChecker.o: intChecker.h intChecker.cc
	@echo -n ">>> Compiling "
	$(GPP) --std=c++11 -c readRaw.cc

clean:
	@for progs in $(PROG); do ( rm -f $$progs $$progs.o ); done
