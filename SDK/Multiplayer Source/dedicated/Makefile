#
# hlds (front end for hlds_l) Makefile for Linux i386
#
# May 2000, Leon Hartwig (hartwig@valvesoftware.com)
#

#make sure this is the correct compiler for your system
CC=gcc

SRCDIR=.
OBJDIR=$(SRCDIR)/obj

#safe optimization
CFLAGS=-w -m486 -O1

#full optimization
#CFLAGS=-w -m486 -O2 \
	-ffast-math -funroll-loops \
	-fexpensive-optimizations -malign-loops=2 \
	-malign-jumps=2 -malign-functions=2

#use these when debugging 
#CFLAGS=$(BASE_CFLAGS) -g

LDFLAGS=-lgcc -ldl

AR=ar
RANLIB=ranlib

INCLUDEDIRS=-I. -I../common

DO_CC=$(CC) $(INCLUDEDIRS) $(CFLAGS) -o $@ -c $<

#############################################################################
# HLDS FRONT END
#############################################################################

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(DO_CC)

OBJ = \
	$(OBJDIR)/sys_ded.o \
	$(OBJDIR)/engine.o

hlds : neat $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)
neat:
	-mkdir $(OBJDIR)
clean:
	-rm -f $(OBJ)
	-rm -f hlds
spotless: clean
	-rm -r $(OBJDIR)

