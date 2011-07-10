# src/drivers/video/Makefile.inc

rel := src/drivers/video

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_TRM_BAS  := font keys null terminal
DRV_TRM_NBAS :=

ifeq "$(PCE_ENABLE_X11)" "1"
DRV_TRM_BAS += x11
else
DRV_TRM_NBAS += x11
endif

ifeq "$(PCE_ENABLE_SDL)" "1"
DRV_TRM_BAS += sdl
else
DRV_TRM_NBAS += sdl
endif

DRV_TRM_SRC  := $(foreach f,$(DRV_TRM_BAS),$(rel)/$(f).c)
DRV_TRM_NSRC := $(foreach f,$(DRV_TRM_NBAS),$(rel)/$(f).c)
DRV_TRM_OBJ  := $(foreach f,$(DRV_TRM_BAS),$(rel)/$(f).o)
DRV_TRM_NOBJ := $(foreach f,$(DRV_TRM_NBAS),$(rel)/$(f).o)
DRV_TRM_HDR  := $(foreach f,$(DRV_TRM_BAS),$(rel)/$(f).h)
DRV_TRM_NHDR := $(foreach f,$(DRV_TRM_NBAS),$(rel)/$(f).h)
DRV_TRM_ARC  := $(rel)/terminal.a

CLN  += $(DRV_TRM_ARC) $(DRV_TRM_OBJ) $(DRV_TRM_NOBJ)
DIST += $(DRV_TRM_SRC) $(DRV_TRM_HDR) $(DRV_TRM_NSRC) $(DRV_TRM_NHDR)

$(rel)/x11.o:	$(rel)/x11.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(CFLAGS_DEFAULT) $(PCE_X11_CFLAGS) -o $@ $<

$(rel)/sdl.o:	$(rel)/sdl.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(CFLAGS_DEFAULT) $(PCE_SDL_CFLAGS) -o $@ $<

$(rel)/font.o:		$(rel)/font.c
$(rel)/keys.o:		$(rel)/keys.c
$(rel)/null.o:		$(rel)/null.c
$(rel)/term-old.o:	$(rel)/term-old.c
$(rel)/terminal.o:	$(rel)/terminal.c

$(rel)/terminal.a:	$(DRV_TRM_OBJ)
