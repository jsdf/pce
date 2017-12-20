# src/utils/pfi/Makefile.inc

rel := src/utils/pfi

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PFI_UTIL_BAS := \
	comment \
	decode \
	delete \
	encode \
	export \
	info \
	track-edit \
	main

PFI_UTIL_SRC := $(foreach f,$(PFI_UTIL_BAS),$(rel)/$(f).c)
PFI_UTIL_OBJ := $(foreach f,$(PFI_UTIL_BAS),$(rel)/$(f).o)
PFI_UTIL_HDR := $(rel)/main.h
PFI_UTIL_MAN1 := $(rel)/pfi.1
PFI_UTIL_BIN := $(rel)/pfi$(EXEEXT)

PFI_UTIL_OBJ_EXT := \
	src/lib/getopt.o \
	$(DRV_PSI_OBJ) \
	$(DRV_PRI_OBJ) \
	$(DEV_PFI_OBJ)

BIN  += $(PFI_UTIL_BIN)
MAN1 += $(PFI_UTIL_MAN1)
CLN  += $(PFI_UTIL_BIN) $(PFI_UTIL_OBJ)
DIST += $(PFI_UTIL_SRC) $(PFI_UTIL_HDR) $(PFI_UTIL_MAN1)

$(rel)/comment.o:    $(rel)/comment.c
$(rel)/decode.o:     $(rel)/decode.c
$(rel)/delete.o:     $(rel)/delete.c
$(rel)/encode.o:     $(rel)/encode.c
$(rel)/export.o:     $(rel)/export.c
$(rel)/info.o:       $(rel)/info.c
$(rel)/track-edit.o: $(rel)/track-edit.c
$(rel)/main.o:       $(rel)/main.c

$(rel)/pfi$(EXEEXT): $(PFI_UTIL_OBJ_EXT) $(PFI_UTIL_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PFI_UTIL_OBJ) $(PFI_UTIL_OBJ_EXT) -lm
