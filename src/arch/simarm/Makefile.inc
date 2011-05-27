# src/arch/simarm/Makefile.inc

rel := src/arch/simarm

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_SIMARM_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_SIMARM_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_SIMARM_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_SIMARM_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_SIMARM_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_SIMARM_BAS := cmd_arm intc main pci sercons simarm timer
PCE_SIMARM_SRC := $(foreach f,$(PCE_SIMARM_BAS),$(rel)/$(f).c)
PCE_SIMARM_OBJ := $(foreach f,$(PCE_SIMARM_BAS),$(rel)/$(f).o)
PCE_SIMARM_HDR := $(foreach f,$(PCE_SIMARM_BAS),$(rel)/$(f).h)
#PCE_SIMARM_MAN1 := $(rel)/pce-simarm.1
PCE_SIMARM_ETC := $(rel)/pce-simarm.cfg
PCE_SIMARM_BIN := $(rel)/pce-simarm$(EXEEXT)

PCE_SIMARM_OBJ_EXT := \
	src/cpu/arm/arm.a \
	src/chipset/82xx/e8250.o \
	src/devices/pci.o \
	src/devices/pci-ata.o \
	src/devices/ata.o \
	src/devices/device.o \
	src/devices/memory.o \
	src/devices/serport.o \
	src/drivers/options.o \
	src/drivers/char/char.a \
	src/lib/brkpt.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/ihex.o \
	src/lib/iniata.o \
	src/lib/inidsk.o \
	src/lib/iniram.o \
	src/lib/load.o \
	src/lib/log.o \
	src/lib/monitor.o \
	src/lib/msg.o \
	src/lib/path.o \
	src/lib/srec.o \
	src/lib/string.o \
	src/lib/sysdep.o \
	src/libini/libini.a \
	$(DEV_BLK_OBJ)

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_SIMARM_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_SIMARM)" "1"
BIN  += $(PCE_SIMARM_BIN)
ETC  += $(PCE_SIMARM_ETC)
#MAN1 += $(PCE_SIMARM_MAN1)
endif

CLN  += $(PCE_SIMARM_BIN) $(PCE_SIMARM_OBJ) $(PCE_SIMARM_ETC)
DIST += $(PCE_SIMARM_SRC) $(PCE_SIMARM_HDR) $(PCE_SIMARM_MAN1)
DIST += $(rel)/pce-simarm.cfg.in

$(rel)/cmd_arm.o:	$(rel)/cmd_arm.c
$(rel)/intc.o:		$(rel)/intc.c
$(rel)/main.o:		$(rel)/main.c
$(rel)/pci.o:		$(rel)/pci.c
$(rel)/sercons.o:	$(rel)/sercons.c
$(rel)/simarm.o:	$(rel)/simarm.c
$(rel)/timer.o:		$(rel)/timer.c

$(rel)/pce-simarm$(EXEEXT): $(PCE_SIMARM_OBJ_EXT) $(PCE_SIMARM_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_SIMARM_LDFLAGS) -o $@ $(PCE_SIMARM_OBJ) $(PCE_SIMARM_OBJ_EXT) $(PCE_SIMARM_LIBS) -lm

$(rel)/pce-simarm.cfg: $(rel)/pce-simarm.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
