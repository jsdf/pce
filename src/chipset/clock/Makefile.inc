# src/chipset/clock/Makefile.inc

rel := src/chipset/clock

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CS_CLK_BAS := ds1743
CS_CLK_SRC := $(foreach f,$(CS_CLK_BAS),$(rel)/$(f).c)
CS_CLK_OBJ := $(foreach f,$(CS_CLK_BAS),$(rel)/$(f).o)
CS_CLK_HDR := $(foreach f,$(CS_CLK_BAS),$(rel)/$(f).h)

CLN  += $(CS_CLK_OBJ)
DIST += $(CS_CLK_SRC) $(CS_CLK_HDR)

$(rel)/ds1743.o:	$(rel)/ds1743.c $(rel)/ds1743.h
