# src/arch/sim405/pceutils/Makefile.inc

rel := src/arch/sim405/pceutils

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

FILES := Makefile config.h pce.S pce.h pcecp.c pcemsg.c pcetime.c pcever.c

DIST += $(foreach f,$(FILES),$(rel)/$(f))
