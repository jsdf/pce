# src/arch/rc759/Makefile.inc

rel := src/arch/rc759

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_RC759_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_RC759_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_RC759_LIBS    := $(LIBS)

ifeq "$(PCE_ENABLE_SDL)" "1"
	PCE_RC759_CFLAGS += $(PCE_SDL_CFLAGS)
	PCE_RC759_LIBS += $(PCE_SDL_LIBS)
endif

ifeq "$(PCE_ENABLE_X11)" "1"
	PCE_RC759_CFLAGS += $(PCE_X11_CFLAGS)
	PCE_RC759_LIBS += $(PCE_X11_LIBS)
endif

ifeq "$(PCE_HOST_WINDOWS)" "1"
	PCE_RC759_LIBS += -mconsole
endif

ifeq "$(PCE_ENABLE_READLINE)" "1"
	PCE_RC759_LIBS += $(PCE_READLINE_LIBS)
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_RC759_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_RC759_BAS  := \
	cmd \
	fdc \
	keyboard \
	main \
	msg \
	nvm \
	parport \
	rc759 \
	rtc \
	speaker \
	video

PCE_RC759_SRC  := $(foreach f,$(PCE_RC759_BAS),$(rel)/$(f).c)
PCE_RC759_OBJ  := $(foreach f,$(PCE_RC759_BAS),$(rel)/$(f).o)
PCE_RC759_HDR  := $(foreach f,$(PCE_RC759_BAS),$(rel)/$(f).h)
PCE_RC759_MAN1 := $(rel)/pce-rc759.1
PCE_RC759_ETC  := $(rel)/pce-rc759.cfg
PCE_RC759_BIN  := $(rel)/pce-rc759$(EXEEXT)

PCE_RC759_OBJ_EXT := \
	src/cpu/e8086/e8086.a \
	src/chipset/80186/dma.o \
	src/chipset/80186/icu.o \
	src/chipset/80186/tcu.o \
	src/chipset/82xx/e8250.o \
	src/chipset/82xx/e8255.o \
	src/chipset/82xx/e8259.o \
	src/chipset/wd179x.o \
	src/devices/device.o \
	src/devices/memory.o \
	src/drivers/block/block.a \
	src/drivers/char/char.a \
	src/drivers/pbit/pbit.a \
	src/drivers/psi/psi.a \
	src/drivers/sound/sound.a \
	src/drivers/options.o \
	src/lib/brkpt.o \
	src/lib/cmd.o \
	src/lib/console.o \
	src/lib/getopt.o \
	src/lib/inidsk.o \
	src/lib/iniram.o \
	src/lib/initerm.o \
	src/lib/ihex.o \
	src/lib/srec.o \
	src/lib/load.o \
	src/lib/log.o \
	src/lib/monitor.o \
	src/lib/msg.o \
	src/lib/path.o \
	src/lib/string.o \
	src/lib/sysdep.o \
	src/libini/libini.a \
	src/drivers/block/block.a \
	src/drivers/psi/psi.a \
	$(DRV_TRM_OBJ)

ifeq "$(PCE_ENABLE_TUN)" "1"
PCE_RC759_OBJ_EXT += src/lib/tun.o
endif

ifeq "$(PCE_BUILD_RC759)" "1"
BIN  += $(PCE_RC759_BIN)
ETC  += $(PCE_RC759_ETC)
MAN1 += $(PCE_RC759_MAN1)
endif

CLN  += $(PCE_RC759_BIN) $(PCE_RC759_OBJ) $(PCE_RC759_ETC)
DIST += $(PCE_RC759_SRC) $(PCE_RC759_HDR) $(PCE_RC759_MAN1)
DIST += $(rel)/pce-rc759.cfg.in

$(rel)/cmd.o:		$(rel)/cmd.c
$(rel)/fdc.o:		$(rel)/fdc.c
$(rel)/keyboard.o:	$(rel)/keyboard.c
$(rel)/main.o:		$(rel)/main.c
$(rel)/msg.o:		$(rel)/msg.c
$(rel)/nvm.o:		$(rel)/nvm.c
$(rel)/parport.o:	$(rel)/parport.c
$(rel)/rc759.o:		$(rel)/rc759.c
$(rel)/rtc.o:		$(rel)/rtc.c
$(rel)/speaker.o:	$(rel)/speaker.c
$(rel)/video.o:		$(rel)/video.c

$(rel)/pce-rc759$(EXEEXT): $(PCE_RC759_OBJ_EXT) $(PCE_RC759_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_RC759_LDFLAGS) -o $@ $(PCE_RC759_OBJ) $(PCE_RC759_OBJ_EXT) $(PCE_RC759_LIBS) -lm

$(rel)/pce-rc759.cfg: $(rel)/pce-rc759.cfg.in
	$(QP)echo "  SED    $@"
	$(QR)sed -e "s]PCE_DIR_DATA]$(datadir)]g" < $< > $@
