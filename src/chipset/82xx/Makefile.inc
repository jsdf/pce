# src/chipset/82xx/Makefile.inc

rel := src/chipset/82xx

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CS_82XX_BAS := e8237 e8250 e8253 e8255 e8259 e8272
CS_82XX_SRC := $(foreach f,$(CS_82XX_BAS),$(rel)/$(f).c)
CS_82XX_OBJ := $(foreach f,$(CS_82XX_BAS),$(rel)/$(f).o)
CS_82XX_HDR := $(foreach f,$(CS_82XX_BAS),$(rel)/$(f).h)

CLN  += $(CS_82XX_OBJ)
DIST += $(CS_82XX_SRC) $(CS_82XX_HDR)

$(rel)/e8237.o:	$(rel)/e8237.c $(rel)/e8237.h
$(rel)/e8250.o:	$(rel)/e8250.c $(rel)/e8250.h
$(rel)/e8253.o:	$(rel)/e8253.c $(rel)/e8253.h
$(rel)/e8255.o:	$(rel)/e8255.c $(rel)/e8255.h
$(rel)/e8259.o:	$(rel)/e8259.c $(rel)/e8259.h
$(rel)/e8272.o:	$(rel)/e8272.c $(rel)/e8272.h
