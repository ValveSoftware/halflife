# simple Makefile for Half-Life Model Viewer

CC = g++
CFLAGS = -O2
IFLAGS =
LFLAGS = -s
LIBS = -lmxtk-qt -lqt -lqgl -lGL -lGLU -lXaw

HLMV = ../bin/hlmv
OBJS = \
	ControlPanel.o \
	FileAssociation.o \
	GlWindow.o \
	ViewerSettings.o \
	mathlib.o \
	mdlviewer.o \
	pakviewer.o \
	studio_render.o \
	studio_utils.o \

all: $(HLMV)

$(HLMV): $(OBJS)
	$(CC) $(LFLAGS) -o $(HLMV) $(OBJS) $(LIBS) 

ControlPanel.o: ControlPanel.cpp ControlPanel.h ViewerSettings.h StudioModel.h GlWindow.h
	$(CC) -c $(CFLAGS) $(IFLAGS) ControlPanel.cpp

FileAssociation.o: FileAssociation.cpp FileAssociation.h
	$(CC) -c $(CFLAGS) $(IFLAGS) FileAssociation.cpp

GlWindow.o: GlWindow.cpp GlWindow.h StudioModel.h ViewerSettings.h
	$(CC) -c $(CFLAGS) $(IFLAGS) GlWindow.cpp

ViewerSettings.o: ViewerSettings.cpp ViewerSettings.h
	$(CC) -c $(CFLAGS) $(IFLAGS) ViewerSettings.cpp

mathlib.o: mathlib.c mathlib.h
	$(CC) -c $(CFLAGS) $(IFLAGS) mathlib.c

mdlviewer.o: mdlviewer.cpp mdlviewer.h GlWindow.h ControlPanel.h StudioModel.h pakviewer.h FileAssociation.h
	$(CC) -c $(CFLAGS) $(IFLAGS) mdlviewer.cpp

pakviewer.o: pakviewer.cpp pakviewer.h mdlviewer.h GlWindow.h ControlPanel.h StudioModel.h FileAssociation.h
	$(CC) -c $(CFLAGS) $(IFLAGS) pakviewer.cpp


studio_render.o: studio_render.cpp StudioModel.h ViewerSettings.h
	$(CC) -c $(CFLAGS) $(IFLAGS) studio_render.cpp

studio_utils.o: studio_utils.cpp StudioModel.h
	$(CC) -c $(CFLAGS) $(IFLAGS) studio_utils.cpp



# clean

clean:
	rm *.o
