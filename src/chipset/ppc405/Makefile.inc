# src/chipset/ppc405/Makefile.inc

rel := src/chipset/ppc405

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CS_PPC405_BAS := uic
CS_PPC405_SRC := $(foreach f,$(CS_PPC405_BAS),$(rel)/$(f).c)
CS_PPC405_OBJ := $(foreach f,$(CS_PPC405_BAS),$(rel)/$(f).o)
CS_PPC405_HDR := $(foreach f,$(CS_PPC405_BAS),$(rel)/$(f).h)

CLN  += $(CS_PPC405_OBJ)
DIST += $(CS_PPC405_SRC) $(CS_PPC405_HDR)

$(rel)/uic.o:	$(rel)/uic.c $(rel)/uic.h
