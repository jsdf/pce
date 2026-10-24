# src/arch/cpm80/Makefile.inc

rel := src/arch/cpm80

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_CPM80_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_CPM80_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_CPM80_LIBS    := $(LIBS)

ifeq "$(PCE_HOST_WINDOWS)" "1"
	PCE_CPM80_LIBS += -mconsole
endif

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_CPM80_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_CPM80_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_CPM80_BAS := bios cmd cpm80 main msg
PCE_CPM80_SRC := $(foreach f,$(PCE_CPM80_BAS),$(rel)/$(f).c)
PCE_CPM80_OBJ := $(foreach f,$(PCE_CPM80_BAS),$(rel)/$(f).o)
PCE_CPM80_HDR := $(foreach f,$(PCE_CPM80_BAS),$(rel)/$(f).h)
#PCE_CPM80_MAN1 := $(rel)/pce-cpm80.1
PCE_CPM80_ETC := $(rel)/pce-cpm80.cfg
PCE_CPM80_BIN := $(rel)/pce-cpm80$(EXEEXT)

PCE_CPM80_OBJ_EXT := \
	src/devices/memory.o \
	src/drivers/options.o \
	src/lib/brkpt.o \
	src/lib/cfg.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/getopt.o \
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
	$(CPU_8080_OBJ) \
	$(DRV_BLK_OBJ) \
	$(DRV_PRI_OBJ) \
	$(DRV_PSI_OBJ) \
	$(DRV_CHR_OBJ)

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_CPM80_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_CPM80)" "1"
BIN  += $(PCE_CPM80_BIN)
ETC  += $(PCE_CPM80_ETC)
MAN1 += $(PCE_CPM80_MAN1)
endif

CLN  += $(PCE_CPM80_BIN) $(PCE_CPM80_OBJ) $(PCE_CPM80_ETC)
DIST += $(PCE_CPM80_SRC) $(PCE_CPM80_HDR) $(PCE_CPM80_MAN1)
DIST += $(rel)/pce-cpm80.cfg.in

$(rel)/bios.o:  $(rel)/bios.c
$(rel)/cmd.o:   $(rel)/cmd.c
$(rel)/cpm80.o: $(rel)/cpm80.c
$(rel)/main.o:  $(rel)/main.c
$(rel)/msg.o:   $(rel)/msg.c

$(rel)/pce-cpm80$(EXEEXT): $(PCE_CPM80_OBJ_EXT) $(PCE_CPM80_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_CPM80_LDFLAGS) -o $@ $(PCE_CPM80_OBJ) $(PCE_CPM80_OBJ_EXT) $(PCE_CPM80_LIBS) -lm

$(rel)/pce-cpm80.cfg: $(rel)/pce-cpm80.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
