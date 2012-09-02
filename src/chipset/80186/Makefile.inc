# src/chipset/80186/Makefile.inc

rel := src/chipset/80186

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CS_80186_BAS := dma icu tcu
CS_80186_SRC := $(foreach f,$(CS_80186_BAS),$(rel)/$(f).c)
CS_80186_OBJ := $(foreach f,$(CS_80186_BAS),$(rel)/$(f).o)
CS_80186_HDR := $(foreach f,$(CS_80186_BAS),$(rel)/$(f).h)

CLN  += $(CS_80186_OBJ)
DIST += $(CS_80186_SRC) $(CS_80186_HDR)

$(rel)/dma.o:	$(rel)/dma.c $(rel)/dma.h
$(rel)/icu.o:	$(rel)/icu.c $(rel)/icu.h
$(rel)/tcu.o:	$(rel)/tcu.c $(rel)/tcu.h
