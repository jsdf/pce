# src/arch/spectrum/Makefile.inc

rel := src/arch/spectrum

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_SPECTRUM_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_SPECTRUM_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_SPECTRUM_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_SDL)" "1"
	PCE_SPECTRUM_CFLAGS += $(PCE_SDL_CFLAGS)
	PCE_SPECTRUM_LIBS += $(PCE_SDL_LIBS)
endif

ifeq "$(PCE_ENABLE_X11)" "1"
	PCE_SPECTRUM_CFLAGS += $(PCE_X11_CFLAGS)
	PCE_SPECTRUM_LIBS += $(PCE_X11_LIBS)
endif

ifeq "$(PCE_HOST_WINDOWS)" "1"
	PCE_SPECTRUM_LIBS += -mconsole
endif

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_SPECTRUM_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_SPECTRUM_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_SPECTRUM_BAS := cmd keyboard main msg snapshot spectrum video
PCE_SPECTRUM_SRC := $(foreach f,$(PCE_SPECTRUM_BAS),$(rel)/$(f).c)
PCE_SPECTRUM_OBJ := $(foreach f,$(PCE_SPECTRUM_BAS),$(rel)/$(f).o)
PCE_SPECTRUM_HDR := $(foreach f,$(PCE_SPECTRUM_BAS),$(rel)/$(f).h)
#PCE_SPECTRUM_MAN1 := $(rel)/pce-spectrum.1
PCE_SPECTRUM_ETC := $(rel)/pce-spectrum.cfg
PCE_SPECTRUM_BIN := $(rel)/pce-spectrum$(EXEEXT)

PCE_SPECTRUM_OBJ_EXT := \
	src/devices/cassette.o \
	src/devices/memory.o \
	src/devices/speaker.o \
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
	$(CPU_8080_OBJ) \
	$(DRV_PTI_OBJ) \
	$(DRV_CHR_OBJ) \
	$(DRV_SND_OBJ) \
	$(DRV_TRM_OBJ)

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_SPECTRUM_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_SPECTRUM)" "1"
BIN  += $(PCE_SPECTRUM_BIN)
ETC  += $(PCE_SPECTRUM_ETC)
#MAN1 += $(PCE_SPECTRUM_MAN1)
endif

CLN  += $(PCE_SPECTRUM_BIN) $(PCE_SPECTRUM_OBJ) $(PCE_SPECTRUM_ETC)
DIST += $(PCE_SPECTRUM_SRC) $(PCE_SPECTRUM_HDR) $(PCE_SPECTRUM_MAN1)
DIST += $(rel)/pce-spectrum.cfg.in

$(rel)/cmd.o:      $(rel)/cmd.c
$(rel)/keyboard.o: $(rel)/keyboard.c
$(rel)/main.o:     $(rel)/main.c
$(rel)/msg.o:      $(rel)/msg.c
$(rel)/snapshot.o: $(rel)/snapshot.c
$(rel)/spectrum.o: $(rel)/spectrum.c
$(rel)/video.o:    $(rel)/video.c

$(rel)/pce-spectrum$(EXEEXT): $(PCE_SPECTRUM_OBJ_EXT) $(PCE_SPECTRUM_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_SPECTRUM_LDFLAGS) -o $@ $(PCE_SPECTRUM_OBJ) $(PCE_SPECTRUM_OBJ_EXT) $(PCE_SPECTRUM_LIBS) -lm

$(rel)/pce-spectrum.cfg: $(rel)/pce-spectrum.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
