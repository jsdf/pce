# src/drivers/sound/Makefile.inc

rel := src/drivers/sound

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_SND_BAS  := filter sound sound-null sound-wav
DRV_SND_NBAS :=

ifeq "$(PCE_ENABLE_SOUND_OSS)" "1"
DRV_SND_BAS += sound-oss
else
DRV_SND_NBAS += sound-oss
endif

ifeq "$(PCE_ENABLE_SDL)" "1"
DRV_SND_BAS += sound-sdl
else
DRV_SND_NBAS += sound-sdl
endif

DRV_SND_SRC  := $(foreach f,$(DRV_SND_BAS),$(rel)/$(f).c)
DRV_SND_NSRC := $(foreach f,$(DRV_SND_NBAS),$(rel)/$(f).c)
DRV_SND_OBJ  := $(foreach f,$(DRV_SND_BAS),$(rel)/$(f).o)
DRV_SND_NOBJ := $(foreach f,$(DRV_SND_NBAS),$(rel)/$(f).o)
DRV_SND_HDR  := $(foreach f,$(DRV_SND_BAS),$(rel)/$(f).h)
DRV_SND_NHDR := $(foreach f,$(DRV_SND_NBAS),$(rel)/$(f).h)
DRV_SND_ARC  := $(rel)/sound.a

CLN  += $(DRV_SND_OBJ) $(DRV_SND_NOBJ) $(DRV_SND_ARC)
DIST += $(DRV_SND_SRC) $(DRV_SND_HDR) $(DRV_SND_NSRC) $(DRV_SND_NHDR)

$(rel)/sound-sdl.o:	$(rel)/sound-sdl.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(CFLAGS_DEFAULT) $(PCE_SDL_CFLAGS) -o $@ $<

$(rel)/filter.o:	$(rel)/filter.c
$(rel)/sound.o:		$(rel)/sound.c
$(rel)/sound-null.o:	$(rel)/sound-null.c
$(rel)/sound-oss.o:	$(rel)/sound-oss.c
$(rel)/sound-wav.o:	$(rel)/sound-wav.c

$(rel)/sound.a: $(DRV_SND_OBJ)
