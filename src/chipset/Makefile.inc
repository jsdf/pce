# src/chipset/Makefile.inc

rel := src/chipset

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CHIPSET_BAS := e6522 e8530
CHIPSET_SRC := $(foreach f,$(CHIPSET_BAS),$(rel)/$(f).c)
CHIPSET_OBJ := $(foreach f,$(CHIPSET_BAS),$(rel)/$(f).o)
CHIPSET_HDR := $(foreach f,$(CHIPSET_BAS),$(rel)/$(f).h)

CLN  += $(CHIPSET_OBJ)
DIST += $(CHIPSET_SRC) $(CHIPSET_HDR)

$(rel)/e6522.o:	$(rel)/e6522.c $(rel)/e6522.h
$(rel)/e8530.o:	$(rel)/e8530.c $(rel)/e8530.h
