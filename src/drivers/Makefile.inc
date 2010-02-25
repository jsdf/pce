# src/drivers/Makefile.inc

rel := src/drivers

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CLN  += $(rel)/options.o
DIST += $(rel)/options.c $(rel)/options.h

$(rel)/options.o:	$(rel)/options.c
