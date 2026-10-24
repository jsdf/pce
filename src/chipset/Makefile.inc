# src/chipset/Makefile.inc

rel := src/chipset

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CHIPSET_BAS := \
	e6522 \
	e6560 \
	e6845 \
	e6850 \
	e68901 \
	e8530 \
	wd179x

CHIPSET_SRC := $(foreach f,$(CHIPSET_BAS),$(rel)/$(f).c)
CHIPSET_OBJ := $(foreach f,$(CHIPSET_BAS),$(rel)/$(f).o)
CHIPSET_HDR := $(foreach f,$(CHIPSET_BAS),$(rel)/$(f).h)

CLN  += $(CHIPSET_OBJ)
DIST += $(CHIPSET_SRC) $(CHIPSET_HDR)

$(rel)/e6522.o:  $(rel)/e6522.c $(rel)/e6522.h
$(rel)/e6560.o:  $(rel)/e6560.c $(rel)/e6560.h
$(rel)/e6845.o:  $(rel)/e6845.c $(rel)/e6845.h
$(rel)/e6850.o:  $(rel)/e6850.c $(rel)/e6850.h
$(rel)/e68901.o: $(rel)/e68901.c $(rel)/e68901.h
$(rel)/e8530.o:  $(rel)/e8530.c $(rel)/e8530.h
$(rel)/wd179x.o: $(rel)/wd179x.c $(rel)/wd179x.h
