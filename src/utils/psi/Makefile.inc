# src/utils/psi/Makefile.inc

rel := src/utils/psi

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PSI_BAS := main
PSI_SRC := $(foreach f,$(PSI_BAS),$(rel)/$(f).c)
PSI_OBJ := $(foreach f,$(PSI_BAS),$(rel)/$(f).o)
PSI_HDR := $(foreach f,$(PSI_BAS),$(rel)/$(f).h)
PSI_MAN1 := $(rel)/psi.1
PSI_BIN := $(rel)/psi$(EXEEXT)

PSI_OBJ_EXT := \
	src/drivers/psi/psi.a \
	src/lib/getopt.o

BIN  += $(PSI_BIN)
MAN1 += $(PSI_MAN1)
CLN  += $(PSI_BIN) $(PSI_OBJ)
DIST += $(PSI_SRC) $(PSI_HDR) $(PSI_MAN1)

$(rel)/main.o: $(rel)/main.c

$(rel)/psi$(EXEEXT): $(PSI_OBJ_EXT) $(PSI_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PSI_OBJ) $(PSI_OBJ_EXT)
