# src/arch/vic20/Makefile.inc

rel := src/arch/vic20

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_VIC20_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_VIC20_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_VIC20_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_SDL)" "1"
	PCE_VIC20_CFLAGS += $(PCE_SDL_CFLAGS)
	PCE_VIC20_LIBS += $(PCE_SDL_LIBS)
endif

ifeq "$(PCE_ENABLE_X11)" "1"
	PCE_VIC20_CFLAGS += $(PCE_X11_CFLAGS)
	PCE_VIC20_LIBS += $(PCE_X11_LIBS)
endif

ifeq "$(PCE_HOST_WINDOWS)" "1"
	PCE_VIC20_LIBS += -mconsole
endif

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_VIC20_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_VIC20_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_VIC20_BAS := cmd keybd main msg setup vic20 video
PCE_VIC20_SRC := $(foreach f,$(PCE_VIC20_BAS),$(rel)/$(f).c)
PCE_VIC20_OBJ := $(foreach f,$(PCE_VIC20_BAS),$(rel)/$(f).o)
PCE_VIC20_HDR := $(foreach f,$(PCE_VIC20_BAS),$(rel)/$(f).h)
#PCE_VIC20_MAN1 := $(rel)/pce-vic20.1
PCE_VIC20_ETC := $(rel)/pce-vic20.cfg
PCE_VIC20_BIN := $(rel)/pce-vic20$(EXEEXT)

PCE_VIC20_OBJ_EXT := \
	src/chipset/e6522.o \
	src/chipset/e6560.o \
	src/devices/cassette.o \
	src/devices/memory.o \
	src/drivers/options.o \
	src/lib/brkpt.o \
	src/lib/cfg.o \
	src/lib/ciff.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/endian.o \
	src/lib/getopt.o \
	src/lib/iniram.o \
	src/lib/initerm.o \
	src/lib/log.o \
	src/lib/monitor.o \
	src/lib/msg.o \
	src/lib/path.o \
	src/lib/string.o \
	src/lib/sysdep.o \
	src/lib/text.o \
	$(LIBPCE_LOAD_OBJ) \
	$(LIBINI_OBJ) \
	$(CPU_6502_OBJ) \
	$(DRV_PTI_OBJ) \
	$(DRV_SND_OBJ) \
	$(DRV_TRM_OBJ)

ifeq "$(PCE_BUILD_VIC20)" "1"
BIN  += $(PCE_VIC20_BIN)
ETC  += $(PCE_VIC20_ETC)
#MAN1 += $(PCE_VIC20_MAN1)
endif

CLN  += $(PCE_VIC20_BIN) $(PCE_VIC20_OBJ) $(PCE_VIC20_ETC)
DIST += $(PCE_VIC20_SRC) $(PCE_VIC20_HDR) $(PCE_VIC20_MAN1)
DIST += $(rel)/pce-vic20.cfg.in

$(rel)/cmd.o:   $(rel)/cmd.c
$(rel)/keybd.o: $(rel)/keybd.c
$(rel)/main.o:  $(rel)/main.c
$(rel)/msg.o:   $(rel)/msg.c
$(rel)/setup.o: $(rel)/setup.c
$(rel)/vic20.o: $(rel)/vic20.c
$(rel)/video.o: $(rel)/video.c

$(rel)/pce-vic20$(EXEEXT): $(PCE_VIC20_OBJ_EXT) $(PCE_VIC20_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_VIC20_LDFLAGS) -o $@ $(PCE_VIC20_OBJ) $(PCE_VIC20_OBJ_EXT) $(PCE_VIC20_LIBS) -lm

$(rel)/pce-vic20.cfg: $(rel)/pce-vic20.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
