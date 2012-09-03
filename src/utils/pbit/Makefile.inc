# src/utils/pbit/Makefile.inc

rel := src/utils/pbit

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PBIT_BAS := pbit
PBIT_SRC := $(foreach f,$(PBIT_BAS),$(rel)/$(f).c)
PBIT_OBJ := $(foreach f,$(PBIT_BAS),$(rel)/$(f).o)
PBIT_HDR := $(foreach f,$(PBIT_BAS),$(rel)/$(f).h)
PBIT_MAN1 := $(rel)/pbit.1
PBIT_BIN := $(rel)/pbit$(EXEEXT)

PBIT_OBJ_EXT := \
	src/lib/getopt.o \
	src/drivers/pbit/pbit.a \
	src/drivers/pfdc/pfdc.a

BIN  += $(PBIT_BIN)
MAN1 += $(PBIT_MAN1)
CLN  += $(PBIT_BIN) $(PBIT_OBJ)
DIST += $(PBIT_SRC) $(PBIT_HDR) $(PBIT_MAN1)

$(rel)/pbit.o:		$(rel)/pbit.c

$(rel)/pbit$(EXEEXT): $(PBIT_OBJ_EXT) $(PBIT_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PBIT_OBJ) $(PBIT_OBJ_EXT)
