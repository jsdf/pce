# src/devices/clock/Makefile.inc

rel := src/devices/clock

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DEV_CLK_BAS := ds1743
DEV_CLK_SRC := $(foreach f,$(DEV_CLK_BAS),$(rel)/$(f).c)
DEV_CLK_OBJ := $(foreach f,$(DEV_CLK_BAS),$(rel)/$(f).o)
DEV_CLK_HDR := $(foreach f,$(DEV_CLK_BAS),$(rel)/$(f).h)

CLN  += $(DEV_CLK_OBJ)
DIST += $(DEV_CLK_SRC) $(DEV_CLK_HDR)

$(rel)/ds1743.o:	$(rel)/ds1743.c
