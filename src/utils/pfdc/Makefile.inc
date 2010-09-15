# src/utils/pfdc/Makefile.inc

rel := src/utils/pfdc

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PFDC_BAS := main pfdc-img-io
PFDC_SRC := $(foreach f,$(PFDC_BAS),$(rel)/$(f).c)
PFDC_OBJ := $(foreach f,$(PFDC_BAS),$(rel)/$(f).o)
PFDC_HDR := $(foreach f,$(PFDC_BAS),$(rel)/$(f).h)
PFDC_MAN1 := $(rel)/pfdc.1
PFDC_BIN := $(rel)/pfdc$(EXEEXT)

PFDC_OBJ_EXT := $(DEV_BLK_PFDC_OBJ) src/lib/getopt.o

BIN  += $(PFDC_BIN)
MAN1 += $(PFDC_MAN1)
CLN  += $(PFDC_BIN) $(PFDC_OBJ)
DIST += $(PFDC_SRC) $(PFDC_HDR) $(PFDC_MAN1)

$(rel)/main.o:		$(rel)/main.c
$(rel)/pfdc-img-io.o:	$(rel)/pfdc-img-io.c

$(rel)/pfdc$(EXEEXT): $(PFDC_OBJ_EXT) $(PFDC_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PFDC_OBJ) $(PFDC_OBJ_EXT)
