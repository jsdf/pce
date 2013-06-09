# src/drivers/psi/Makefile.inc

rel := src/drivers/psi

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DEV_PSI_BAS := \
	psi-img-ana \
	psi-img-cp2 \
	psi-img-dc42 \
	psi-img-imd \
	psi-img-msa \
	psi-img-pfdc \
	psi-img-psi \
	psi-img-raw \
	psi-img-stx \
	psi-img-tc \
	psi-img-td0 \
	psi-img-xdf \
	psi-img \
	psi-io \
	psi

DEV_PSI_BAS2 := \
	psi-img-pfdc0 \
	psi-img-pfdc1 \
	psi-img-pfdc2 \
	psi-img-pfdc4

DEV_PSI_SRC := $(foreach f,$(DEV_PSI_BAS) $(DEV_PSI_BAS2),$(rel)/$(f).c)
DEV_PSI_OBJ := $(foreach f,$(DEV_PSI_BAS) $(DEV_PSI_BAS2),$(rel)/$(f).o)
DEV_PSI_HDR := $(foreach f,$(DEV_PSI_BAS),$(rel)/$(f).h)
DEV_PSI_ARC := $(rel)/psi.a

CLN  += $(DEV_PSI_ARC) $(DEV_PSI_OBJ)
DIST += $(DEV_PSI_SRC) $(DEV_PSI_HDR)

$(rel)/psi-img-ana.o:   $(rel)/psi-img-ana.c
$(rel)/psi-img-cp2.o:   $(rel)/psi-img-cp2.c
$(rel)/psi-img-dc42.o:  $(rel)/psi-img-dc42.c
$(rel)/psi-img-imd.o:   $(rel)/psi-img-imd.c
$(rel)/psi-img-msa.o:   $(rel)/psi-img-msa.c
$(rel)/psi-img-pfdc.o:  $(rel)/psi-img-pfdc.c
$(rel)/psi-img-pfdc0.o: $(rel)/psi-img-pfdc0.c
$(rel)/psi-img-pfdc1.o: $(rel)/psi-img-pfdc1.c
$(rel)/psi-img-pfdc2.o: $(rel)/psi-img-pfdc2.c
$(rel)/psi-img-pfdc4.o: $(rel)/psi-img-pfdc4.c
$(rel)/psi-img-psi.o:   $(rel)/psi-img-psi.c
$(rel)/psi-img-raw.o:   $(rel)/psi-img-raw.c
$(rel)/psi-img-stx.o:   $(rel)/psi-img-stx.c
$(rel)/psi-img-tc.o:    $(rel)/psi-img-tc.c
$(rel)/psi-img-td0.o:   $(rel)/psi-img-td0.c
$(rel)/psi-img-xdf.o:   $(rel)/psi-img-xdf.c
$(rel)/psi-img.o:       $(rel)/psi-img.c
$(rel)/psi-io.o:        $(rel)/psi-io.c
$(rel)/psi.o:           $(rel)/psi.c

$(rel)/psi.a: $(DEV_PSI_OBJ)
