# src/drivers/pri/Makefile.inc

rel := src/drivers/pri

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_PRI_BAS := \
	gcr-mac \
	mfm-ibm \
	pri-img-pbit \
	pri-img-tc \
	pri-img \
	pri

DRV_PRI_SRC := $(foreach f,$(DRV_PRI_BAS),$(rel)/$(f).c)
DRV_PRI_OBJ := $(foreach f,$(DRV_PRI_BAS),$(rel)/$(f).o)
DRV_PRI_HDR := $(foreach f,$(DRV_PRI_BAS),$(rel)/$(f).h)
DRV_PRI_ARC := $(rel)/pri.a

CLN  += $(DRV_PRI_ARC) $(DRV_PRI_OBJ)
DIST += $(DRV_PRI_SRC) $(DRV_PRI_HDR)

$(rel)/gcr-mac.o:      $(rel)/gcr-mac.c
$(rel)/mfm-ibm.o:      $(rel)/mfm-ibm.c
$(rel)/pri-img-pbit.o: $(rel)/pri-img-pbit.c
$(rel)/pri-img-tc.o:   $(rel)/pri-img-tc.c
$(rel)/pri-img.o:      $(rel)/pri-img.c
$(rel)/pri.o:          $(rel)/pri.c

$(rel)/pri.a: $(DRV_PRI_OBJ)
