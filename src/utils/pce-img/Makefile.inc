# src/utils/pce-img/Makefile.inc

rel := src/utils/pce-img

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PCEIMG_BAS := \
	commit \
	convert \
	cow \
	create \
	info \
	rebase \
	pce-img

PCEIMG_SRC := $(foreach f,$(PCEIMG_BAS),$(rel)/$(f).c)
PCEIMG_OBJ := $(foreach f,$(PCEIMG_BAS),$(rel)/$(f).o)
PCEIMG_HDR := $(rel)/pce-img.h
PCEIMG_MAN1 := $(rel)/pce-img.1
PCEIMG_BIN := $(rel)/pce-img$(EXEEXT)

PCEIMG_OBJ_EXT := \
	src/lib/getopt.o \
	src/lib/sysdep.o \
	$(DRV_BLK_OBJ) \
	$(DRV_PRI_OBJ) \
	$(DRV_PSI_OBJ)

BIN  += $(PCEIMG_BIN)
MAN1 += $(PCEIMG_MAN1)
CLN  += $(PCEIMG_BIN) $(PCEIMG_OBJ)
DIST += $(PCEIMG_SRC) $(PCEIMG_HDR) $(PCEIMG_MAN1)

$(rel)/commit.o:  $(rel)/commit.c
$(rel)/convert.o: $(rel)/convert.c
$(rel)/cow.o:     $(rel)/cow.c
$(rel)/create.o:  $(rel)/create.c
$(rel)/info.o:    $(rel)/info.c
$(rel)/rebase.o:  $(rel)/rebase.c
$(rel)/pce-img.o: $(rel)/pce-img.c

$(rel)/pce-img$(EXEEXT): $(PCEIMG_OBJ_EXT) $(PCEIMG_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PCEIMG_OBJ) $(PCEIMG_OBJ_EXT)
