# src/utils/aym/Makefile.inc

rel := src/utils/aym

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

AYM_BAS := \
	decode \
	encode \
	play \
	main

AYM_SRC := $(foreach f,$(AYM_BAS),$(rel)/$(f).c)
AYM_OBJ := $(foreach f,$(AYM_BAS),$(rel)/$(f).o)
AYM_HDR := $(rel)/main.h
AYM_MAN1 := $(rel)/aym.1
AYM_BIN := $(rel)/aym$(EXEEXT)

AYM_OBJ_EXT := \
	src/arch/atarist/psg.o \
	src/lib/getopt.o \
	src/drivers/options.o \
	$(DRV_SND_OBJ)

ifeq "$(PCE_ENABLE_SDL)" "1"
	AYM_CFLAGS += $(PCE_SDL_CFLAGS)
	AYM_LIBS += $(PCE_SDL_LIBS)
endif

BIN  += $(AYM_BIN)
MAN1 += $(AYM_MAN1)
CLN  += $(AYM_BIN) $(AYM_OBJ)
DIST += $(AYM_SRC) $(AYM_HDR) $(AYM_MAN1)

$(rel)/decode.o:     $(rel)/decode.c
$(rel)/encode.o:     $(rel)/encode.c
$(rel)/play.o:       $(rel)/play.c
$(rel)/main.o:       $(rel)/main.c

$(rel)/aym$(EXEEXT): $(AYM_OBJ_EXT) $(AYM_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(AYM_OBJ) $(AYM_OBJ_EXT) $(AYM_LIBS) -lm
