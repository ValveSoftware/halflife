
EXES = qbsp light vis bspinfo entmap visx
NTEXES = qbsp.exe light.exe vis.exe bspinfo.exe entmap.exe visx.exe

#==============================================================================

EXT=	.o

all: $(EXES)

clean:
	rm *.o *.obj $(EXES) $(NTEXES)

next:
	make "CFLAGS = -g -Wall -I.."

nextinstall:
	make "CFLAGS = -O4 -g -Wall  -I.. -arch i386 -arch hppa"
	cp $(EXES) /LocalApps

alpha:
	make "CFLAGS = -g  -I.." "LDFLAGS = -lm"

alphainstall:
	make "CFLAGS = -O4  -I.." "LDFLAGS = -lm"
	cp $(EXES) /LocalApps

nt:
	nmake /nologo "CFLAGS =  -nologo -Zi -DWIN32 -I.." "LDFLAGS = " "EXT = .obj"

ntinstall:
	nmake /nologo "CFLAGS =  -nologo -Ox -G5 -DWIN32 -I.." "LDFLAGS = " "EXT = .obj"
	cp $(NTEXES) f:\nt\id

#==============================================================================


QBSPFILES = region$(EXT) map$(EXT) brush$(EXT) cmdlib$(EXT) csg4$(EXT) surfaces$(EXT) mathlib$(EXT)\
         merge$(EXT) outside$(EXT) portals$(EXT) qbsp$(EXT) solidbsp$(EXT) tjunc$(EXT)\
         writebsp$(EXT) bspfile$(EXT) nodraw$(EXT)
qbsp : $(QBSPFILES)
	$(CC) $(CFLAGS) -o qbsp $(QBSPFILES)

light : threads$(EXT) bspfile$(EXT) cmdlib$(EXT) light$(EXT) ltface$(EXT) mathlib$(EXT) trace$(EXT) entities$(EXT)
	$(CC) $(CFLAGS) -o light threads$(EXT) bspfile$(EXT) cmdlib$(EXT) light$(EXT) ltface$(EXT) mathlib$(EXT) trace$(EXT) entities$(EXT)
	
vis : vis$(EXT) flow$(EXT) cmdlib$(EXT) mathlib$(EXT) bspfile$(EXT) soundpvs$(EXT)
	$(CC) $(CFLAGS) -o vis vis$(EXT) flow$(EXT) cmdlib$(EXT) mathlib$(EXT) bspfile$(EXT) soundpvs$(EXT)

visx : visx$(EXT) flowx$(EXT) cmdlib$(EXT) mathlib$(EXT) bspfile$(EXT) soundpvs$(EXT)
	$(CC) $(CFLAGS) -o visx visx$(EXT) flowx$(EXT) cmdlib$(EXT) mathlib$(EXT) bspfile$(EXT) soundpvs$(EXT)

bspinfo : bspinfo$(EXT) bspfile$(EXT) cmdlib$(EXT)
	$(CC) $(CFLAGS) -o bspinfo bspinfo$(EXT) bspfile$(EXT) cmdlib$(EXT)

entmap : entmap$(EXT) cmdlib$(EXT)
	$(CC) $(CFLAGS) -o entmap entmap$(EXT) cmdlib$(EXT)


cmdlib$(EXT) : ../cmdlib.c
	$(CC) $(CFLAGS) -c -o cmdlib$(EXT) ../cmdlib.c
mathlib$(EXT) : ../mathlib.c
	$(CC) $(CFLAGS) -c -o mathlib$(EXT) ../mathlib.c


