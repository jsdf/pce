# src/arch/atarist/Makefile.inc

rel := src/arch/atarist

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_ATARIST_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_ATARIST_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_ATARIST_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_SDL)" "1"
	PCE_ATARIST_CFLAGS += $(PCE_SDL_CFLAGS)
	PCE_ATARIST_LIBS += $(PCE_SDL_LIBS)
endif

ifeq "$(PCE_ENABLE_X11)" "1"
	PCE_ATARIST_CFLAGS += $(PCE_X11_CFLAGS)
	PCE_ATARIST_LIBS += $(PCE_X11_LIBS)
endif

ifeq "$(PCE_HOST_WINDOWS)" "1"
	PCE_ATARIST_LIBS += -mconsole
endif

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_ATARIST_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_ATARIST_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_ATARIST_BAS := \
	acsi \
	atarist \
	cmd \
	dma \
	fdc \
	ikbd \
	main \
	mem \
	msg \
	psg \
	rp5c15 \
	smf \
	video \
	viking

PCE_ATARIST_SRC  := $(foreach f,$(PCE_ATARIST_BAS),$(rel)/$(f).c)
PCE_ATARIST_OBJ  := $(foreach f,$(PCE_ATARIST_BAS),$(rel)/$(f).o)
PCE_ATARIST_HDR  := $(foreach f,$(PCE_ATARIST_BAS),$(rel)/$(f).h)
PCE_ATARIST_HDR  += $(rel)/psg_vol.h
PCE_ATARIST_MAN1 := $(rel)/pce-atarist.1
PCE_ATARIST_ETC  := $(rel)/pce-atarist.cfg
PCE_ATARIST_BIN  := $(rel)/pce-atarist$(EXEEXT)

PCE_ATARIST_OBJ_EXT := \
	src/chipset/82xx/e8250.o \
	src/chipset/e6850.o \
	src/chipset/e68901.o \
	src/chipset/e8530.o \
	src/chipset/wd179x.o \
	src/devices/device.o \
	src/devices/memory.o \
	src/drivers/options.o \
	src/lib/brkpt.o \
	src/lib/cfg.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/getopt.o \
	src/lib/inidsk.o \
	src/lib/iniram.o \
	src/lib/initerm.o \
	src/lib/log.o \
	src/lib/monitor.o \
	src/lib/msg.o \
	src/lib/msgdsk.o \
	src/lib/path.o \
	src/lib/string.o \
	src/lib/sysdep.o \
	$(LIBPCE_LOAD_OBJ) \
	$(LIBINI_OBJ) \
	$(CPU_68K_OBJ) \
	$(DRV_BLK_OBJ) \
	$(DRV_PSI_OBJ) \
	$(DRV_PRI_OBJ) \
	$(DRV_CHR_OBJ) \
	$(DRV_SND_OBJ) \
	$(DRV_TRM_OBJ)

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_ATARIST_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_ATARIST)" "1"
BIN  += $(PCE_ATARIST_BIN)
ETC  += $(PCE_ATARIST_ETC)
MAN1 += $(PCE_ATARIST_MAN1)
endif

CLN  += $(PCE_ATARIST_BIN) $(PCE_ATARIST_OBJ) $(PCE_ATARIST_ETC)
DIST += $(PCE_ATARIST_SRC) $(PCE_ATARIST_HDR) $(PCE_ATARIST_MAN1)
DIST += $(rel)/pce-atarist.cfg.in

$(rel)/acsi.o:    $(rel)/acsi.c
$(rel)/atarist.o: $(rel)/atarist.c
$(rel)/cmd.o:     $(rel)/cmd.c
$(rel)/dma.o:     $(rel)/dma.c
$(rel)/fdc.o:     $(rel)/fdc.c
$(rel)/ikbd.o:    $(rel)/ikbd.c
$(rel)/main.o:    $(rel)/main.c
$(rel)/mem.o:     $(rel)/mem.c
$(rel)/msg.o:     $(rel)/msg.c
$(rel)/psg.o:     $(rel)/psg.c
$(rel)/rp5c15.o:  $(rel)/rp5c15.c
$(rel)/smf.o:     $(rel)/smf.c
$(rel)/video.o:   $(rel)/video.c
$(rel)/viking.o:  $(rel)/viking.c

$(rel)/pce-atarist$(EXEEXT): $(PCE_ATARIST_OBJ_EXT) $(PCE_ATARIST_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_ATARIST_LDFLAGS) -o $@ $(PCE_ATARIST_OBJ) $(PCE_ATARIST_OBJ_EXT) $(PCE_ATARIST_LIBS) -lm

$(rel)/pce-atarist.cfg: $(rel)/pce-atarist.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
