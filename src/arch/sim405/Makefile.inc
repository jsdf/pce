# src/arch/sim405/Makefile.inc

rel := src/arch/sim405

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_SIM405_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_SIM405_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_SIM405_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_SIM405_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_SIM405_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_SIM405_BAS := \
	cmd_ppc \
	hook \
	main \
	msg \
	pci \
	sim405 \
	sercons

PCE_SIM405_SRC := $(foreach f,$(PCE_SIM405_BAS),$(rel)/$(f).c)
PCE_SIM405_OBJ := $(foreach f,$(PCE_SIM405_BAS),$(rel)/$(f).o)
PCE_SIM405_HDR := $(foreach f,$(PCE_SIM405_BAS),$(rel)/$(f).h)
#PCE_SIM405_MAN1 := $(rel)/pce-sim405.1
PCE_SIM405_ETC := $(rel)/pce-sim405.cfg
PCE_SIM405_BIN := $(rel)/pce-sim405$(EXEEXT)

PCE_SIM405_OBJ_EXT := \
	src/chipset/82xx/e8250.o \
	src/chipset/clock/ds1743.o \
	src/chipset/ppc405/uic.o \
	src/devices/clock/ds1743.o \
	src/devices/ata.o \
	src/devices/device.o \
	src/devices/memory.o \
	src/devices/nvram.o \
	src/devices/pci.o \
	src/devices/pci-ata.o \
	src/devices/serport.o \
	src/devices/slip.o \
	src/drivers/options.o \
	src/lib/brkpt.o \
	src/lib/cfg.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/getopt.o \
	src/lib/iniata.o \
	src/lib/inidsk.o \
	src/lib/iniram.o \
	src/lib/log.o \
	src/lib/monitor.o \
	src/lib/msg.o \
	src/lib/path.o \
	src/lib/string.o \
	src/lib/sysdep.o \
	$(LIBPCE_LOAD_OBJ) \
	$(LIBINI_OBJ) \
	$(CPU_PPC405_OBJ) \
	$(DRV_BLK_OBJ) \
	$(DRV_PRI_OBJ) \
	$(DRV_PSI_OBJ) \
	$(DRV_CHR_OBJ)

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_SIM405_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_SIM405)" "1"
BIN  += $(PCE_SIM405_BIN)
ETC  += $(PCE_SIM405_ETC)
#MAN1 += $(PCE_SIM405_MAN1)
endif

CLN  += $(PCE_SIM405_BIN) $(PCE_SIM405_OBJ) $(PCE_SIM405_ETC)
DIST += $(PCE_SIM405_SRC) $(PCE_SIM405_HDR) $(PCE_SIM405_MAN1)
DIST += $(rel)/pce-sim405.cfg.in

$(rel)/cmd_ppc.o:	$(rel)/cmd_ppc.c
$(rel)/hook.o:		$(rel)/hook.c
$(rel)/sim405.o:	$(rel)/sim405.c
$(rel)/main.o:		$(rel)/main.c
$(rel)/pci.o:		$(rel)/pci.c
$(rel)/sercons.o:	$(rel)/sercons.c

$(rel)/pce-sim405$(EXEEXT): $(PCE_SIM405_OBJ_EXT) $(PCE_SIM405_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_SIM405_LDFLAGS) -o $@ $(PCE_SIM405_OBJ) $(PCE_SIM405_OBJ_EXT) $(PCE_SIM405_LIBS) -lm

$(rel)/pce-sim405.cfg: $(rel)/pce-sim405.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
