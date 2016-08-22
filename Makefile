# The CDAS Library
LIBPATH=$(CDASHOME)/lib
LIB_AUGER=-lEr -lEs -lIoAuger -lIoSd -lSTCoordinates -lMoIO
# Set this to the path where the headers have been installed
INCPATH=$(CDASHOME)/include
FD_INCLUDE=$(FD_ROOT)/include


PROG=readAuger

GPP=g++
CFLAGS=-Wall -c -I$(INCPATH) -I$(shell root-config --incdir) -I$(FD_INCLUDE) -std=c++11
LDFLAGS=-Wall $(LIB_AUGER) -L$(LIBPATH) $(shell root-config --libs) -lboost_filesystem -lboost_system

.PHONY:	all clean

all:			$(PROG)
			@echo "done."

$(PROG):	$(PROG).o readMC.o readRaw.o intChecker.o
	@echo -n ">>> Linking   "
	$(GPP) $(PROG).o readMC.o readRaw.o intChecker.o -o $(PROG) $(LDFLAGS)

$(PROG).o:    $(PROG).cc
	@echo -n ">>> Compiling "
	$(GPP) $(CFLAGS) $<

%.o:	%.cc %.h
	@echo -n ">>> Compiling "
	$(GPP) $(CFLAGS) $<

clean:
	@for progs in $(PROG); do ( rm -f $$progs $$progs.o ); done
