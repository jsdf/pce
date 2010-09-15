# src/devices/Makefile.inc

rel := src/devices

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DEV_BAS := ata device fdc memory nvram parport pci pci-ata serport slip
DEV_SRC := $(foreach f,$(DEV_BAS),$(rel)/$(f).c)
DEV_OBJ := $(foreach f,$(DEV_BAS),$(rel)/$(f).o)
DEV_HDR := $(foreach f,$(DEV_BAS),$(rel)/$(f).h)

CLN  += $(DEV_OBJ)
DIST += $(DEV_SRC) $(DEV_HDR)

$(rel)/ata.o:		$(rel)/ata.c
$(rel)/device.o:	$(rel)/device.c
$(rel)/fdc.o:		$(rel)/fdc.c
$(rel)/memory.o:	$(rel)/memory.c
$(rel)/nvram.o:		$(rel)/nvram.c
$(rel)/parport.o:	$(rel)/parport.c
$(rel)/pci.o:		$(rel)/pci.c
$(rel)/pci-ata.o:	$(rel)/pci-ata.c
$(rel)/serport.o:	$(rel)/serport.c
$(rel)/slip.o:		$(rel)/slip.c
