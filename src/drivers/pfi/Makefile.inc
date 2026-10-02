# src/drivers/pfi/Makefile.inc

rel := src/drivers/pfi

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DEV_PFI_BAS := \
	decode-bits \
	pfi-a2r \
	pfi-io \
	pfi-kryo \
	pfi-pfi \
	pfi-scp \
	pfi \
	track

DEV_PFI_SRC := $(foreach f,$(DEV_PFI_BAS),$(rel)/$(f).c)
DEV_PFI_OBJ := $(foreach f,$(DEV_PFI_BAS),$(rel)/$(f).o)
DEV_PFI_HDR := $(foreach f,$(DEV_PFI_BAS),$(rel)/$(f).h)
DEV_PFI_ARC := $(rel)/pfi.a

CLN  += $(DEV_PFI_ARC) $(DEV_PFI_OBJ)
DIST += $(DEV_PFI_SRC) $(DEV_PFI_HDR)

$(rel)/decode-bits.o: $(rel)/decode-bits.c
$(rel)/pfi-a2r.o:     $(rel)/pfi-a2r.c
$(rel)/pfi-io.o:      $(rel)/pfi-io.c
$(rel)/pfi-kryo.o:    $(rel)/pfi-kryo.c
$(rel)/pfi-pfi.o:     $(rel)/pfi-pfi.c
$(rel)/pfi-scp.o:     $(rel)/pfi-scp.c
$(rel)/pfi.o:         $(rel)/pfi.c
$(rel)/track.o:       $(rel)/track.c

$(rel)/pfi.a: $(DEV_PFI_OBJ)
