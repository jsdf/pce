# src/utils/pti/Makefile.inc

rel := src/utils/pti

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PTI_UTIL_BAS := \
	comment \
	ops \
	space \
	main

PTI_UTIL_SRC := $(foreach f,$(PTI_UTIL_BAS),$(rel)/$(f).c)
PTI_UTIL_OBJ := $(foreach f,$(PTI_UTIL_BAS),$(rel)/$(f).o)
PTI_UTIL_HDR := $(foreach f,$(PTI_UTIL_BAS),$(rel)/$(f).h)
PTI_UTIL_MAN1 := $(rel)/pti.1
PTI_UTIL_BIN := $(rel)/pti$(EXEEXT)

PTI_UTIL_OBJ_EXT := \
	src/lib/ciff.o \
	src/lib/endian.o \
	src/lib/getopt.o \
	src/lib/text.o \
	$(DRV_PTI_OBJ)

BIN  += $(PTI_UTIL_BIN)
MAN1 += $(PTI_UTIL_MAN1)
CLN  += $(PTI_UTIL_BIN) $(PTI_UTIL_OBJ)
DIST += $(PTI_UTIL_SRC) $(PTI_UTIL_HDR) $(PTI_UTIL_MAN1)

$(rel)/comment.o: $(rel)/comment.c
$(rel)/main.o:    $(rel)/main.c
$(rel)/ops.o:     $(rel)/ops.c
$(rel)/space.o:   $(rel)/space.c

$(rel)/pti$(EXEEXT): $(PTI_UTIL_OBJ_EXT) $(PTI_UTIL_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PTI_UTIL_OBJ) $(PTI_UTIL_OBJ_EXT) -lm
