# src/arch/sims32/Makefile.inc

rel := src/arch/sims32

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_SIMS32_CFLAGS := $(CFLAGS_DEFAULT)
PCE_SIMS32_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_SIMS32_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_SIMS32_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_SIMS32_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_SIMS32_BAS := cmd_s32 main sercons sims32
PCE_SIMS32_SRC := $(foreach f,$(PCE_SIMS32_BAS),$(rel)/$(f).c)
PCE_SIMS32_OBJ := $(foreach f,$(PCE_SIMS32_BAS),$(rel)/$(f).o)
PCE_SIMS32_HDR := $(foreach f,$(PCE_SIMS32_BAS),$(rel)/$(f).h)
#PCE_SIMS32_MAN1 := $(rel)/pce-sims32.1
PCE_SIMS32_ETC := $(rel)/pce-sims32.cfg
PCE_SIMS32_BIN := $(rel)/pce-sims32$(EXEEXT)

PCE_SIMS32_OBJ_EXT := \
	src/chipset/82xx/e8250.o \
	src/cpu/sparc32/sparc32.a \
	src/devices/device.o \
	src/devices/memory.o \
	src/devices/serport.o \
	src/drivers/options.o \
	src/drivers/char/char.a \
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

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_SIMS32_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_SIMS32)" "1"
BIN  += $(PCE_SIMS32_BIN)
ETC  += $(PCE_SIMS32_ETC)
#MAN1 += $(PCE_SIMS32_MAN1)
endif

CLN  += $(PCE_SIMS32_BIN) $(PCE_SIMS32_OBJ) $(PCE_SIMS32_ETC)
DIST += $(PCE_SIMS32_SRC) $(PCE_SIMS32_HDR) $(PCE_SIMS32_MAN1)
DIST += $(rel)/pce-sims32.cfg.in

$(rel)/cmd_s32.o:	$(rel)/cmd_s32.c
$(rel)/main.o:		$(rel)/main.c
$(rel)/sercons.o:	$(rel)/sercons.c
$(rel)/sims32.o:	$(rel)/sims32.c

$(rel)/pce-sims32$(EXEEXT): $(PCE_SIMS32_OBJ_EXT) $(PCE_SIMS32_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_SIMS32_LDFLAGS) -o $@ $(PCE_SIMS32_OBJ) $(PCE_SIMS32_OBJ_EXT) $(PCE_SIMS32_LIBS) -lm

$(rel)/pce-sims32.cfg: $(rel)/pce-sims32.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
