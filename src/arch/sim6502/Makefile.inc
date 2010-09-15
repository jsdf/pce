# src/arch/sim6502/Makefile.inc

rel := src/arch/sim6502

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_SIM6502_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_SIM6502_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_SIM6502_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_SIM6502_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_SIM6502_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_SIM6502_BAS := console main sim6502
PCE_SIM6502_SRC := $(foreach f,$(PCE_SIM6502_BAS),$(rel)/$(f).c)
PCE_SIM6502_OBJ := $(foreach f,$(PCE_SIM6502_BAS),$(rel)/$(f).o)
PCE_SIM6502_HDR := $(foreach f,$(PCE_SIM6502_BAS),$(rel)/$(f).h)
#PCE_SIM6502_MAN1 := $(rel)/pce-sim405.1
PCE_SIM6502_ETC := $(rel)/pce-sim6502.cfg
PCE_SIM6502_BIN := $(rel)/pce-sim6502$(EXEEXT)

PCE_SIM6502_OBJ_EXT := \
	src/cpu/e6502/e6502.a \
	src/devices/memory.o \
	src/lib/brkpt.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/ihex.o \
	src/lib/iniram.o \
	src/lib/load.o \
	src/lib/log.o \
	src/lib/monitor.o \
	src/lib/path.o \
	src/lib/srec.o \
	src/lib/sysdep.o \
	src/libini/libini.a

ifeq "$(PCE_BUILD_SIM6502)" "1"
BIN  += $(PCE_SIM6502_BIN)
ETC  += $(PCE_SIM6502_ETC)
#MAN1 += $(PCE_SIM6502_MAN1)
endif

CLN  += $(PCE_SIM6502_BIN) $(PCE_SIM6502_OBJ) $(PCE_SIM6502_ETC)
DIST += $(PCE_SIM6502_SRC) $(PCE_SIM6502_HDR) $(PCE_SIM6502_MAN1)
DIST += $(rel)/pce-sim6502.cfg.in

$(rel)/console.o:	$(rel)/console.c
$(rel)/main.o:		$(rel)/main.c
$(rel)/sim6502.o:	$(rel)/sim6502.c

$(rel)/pce-sim6502$(EXEEXT): $(PCE_SIM6502_OBJ_EXT) $(PCE_SIM6502_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_SIM6502_LDFLAGS) -o $@ $(PCE_SIM6502_OBJ) $(PCE_SIM6502_OBJ_EXT) $(PCE_SIM6502_LIBS) -lm

$(rel)/pce-sim6502.cfg: $(rel)/pce-sim6502.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
