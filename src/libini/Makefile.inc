# src/libini/Makefile.inc

rel := src/libini

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

LIBINI_BAS := expr read scanner section strings value write
LIBINI_SRC := $(foreach f,$(LIBINI_BAS),$(rel)/$(f).c)
LIBINI_OBJ := $(foreach f,$(LIBINI_BAS),$(rel)/$(f).o)
LIBINI_HDR := $(foreach f,libini scanner,$(rel)/$(f).h)
LIBINI_ARC := $(rel)/libini.a

CLN  += $(LIBINI_ARC) $(LIBINI_OBJ)
DIST += $(LIBINI_SRC) $(LIBINI_HDR)

$(rel)/expr.o:		$(rel)/expr.c
$(rel)/read.o:		$(rel)/read.c
$(rel)/scanner.o:	$(rel)/scanner.c
$(rel)/section.o:	$(rel)/section.c
$(rel)/strings.o:	$(rel)/strings.c
$(rel)/value.o:		$(rel)/value.c
$(rel)/write.o:		$(rel)/write.c

$(rel)/libini.a: $(LIBINI_OBJ)
