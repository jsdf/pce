# src/drivers/pbit/Makefile.inc

rel := src/drivers/pbit

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_PBIT_BAS := \
	gcr-mac \
	mfm-ibm \
	pbit-io \
	pbit-io-pbit \
	pbit-io-tc \
	pbit

DRV_PBIT_SRC := $(foreach f,$(DRV_PBIT_BAS),$(rel)/$(f).c)
DRV_PBIT_OBJ := $(foreach f,$(DRV_PBIT_BAS),$(rel)/$(f).o)
DRV_PBIT_HDR := $(foreach f,$(DRV_PBIT_BAS),$(rel)/$(f).h)
DRV_PBIT_ARC := $(rel)/pbit.a

CLN  += $(DRV_PBIT_ARC) $(DRV_PBIT_OBJ)
DIST += $(DRV_PBIT_SRC) $(DRV_PBIT_HDR)

$(rel)/gcr-mac.o:	$(rel)/gcr-mac.c
$(rel)/mfm-ibm.o:	$(rel)/mfm-ibm.c
$(rel)/pbit-io.o:	$(rel)/pbit-io.c
$(rel)/pbit-io-pbit.o:	$(rel)/pbit-io-pbit.c
$(rel)/pbit-io-tc.o:	$(rel)/pbit-io-tc.c
$(rel)/pbit.o:		$(rel)/pbit.c

$(rel)/pbit.a: $(DRV_PBIT_OBJ)
