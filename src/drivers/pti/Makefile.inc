# src/drivers/pti/Makefile.inc

rel := src/drivers/pti

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_PTI_BAS := \
	pti-img-cas \
	pti-img-pcm \
	pti-img-pti \
	pti-img-tap \
	pti-img-txt \
	pti-img-wav \
	pti-io \
	pti

DRV_PTI_SRC := $(foreach f,$(DRV_PTI_BAS),$(rel)/$(f).c)
DRV_PTI_OBJ := $(foreach f,$(DRV_PTI_BAS),$(rel)/$(f).o)
DRV_PTI_HDR := $(foreach f,$(DRV_PTI_BAS),$(rel)/$(f).h)
DRV_PTI_ARC := $(rel)/pti.a

CLN  += $(DRV_PTI_ARC) $(DRV_PTI_OBJ)
DIST += $(DRV_PTI_SRC) $(DRV_PTI_HDR)

$(rel)/pti-img-cas.o: $(rel)/pti-img-cas.c
$(rel)/pti-img-pcm.o: $(rel)/pti-img-pcm.c
$(rel)/pti-img-pti.o: $(rel)/pti-img-pti.c
$(rel)/pti-img-tap.o: $(rel)/pti-img-tap.c
$(rel)/pti-img-txt.o: $(rel)/pti-img-txt.c
$(rel)/pti-img-wav.o: $(rel)/pti-img-wav.c
$(rel)/pti-io.o:      $(rel)/pti-io.c
$(rel)/pti.o:         $(rel)/pti.c

$(rel)/pti.a: $(DRV_PTI_OBJ)
