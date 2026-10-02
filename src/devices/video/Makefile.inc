# src/devices/video/Makefile.inc

rel := src/devices/video

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DEV_VID_BAS := \
	cga \
	cga_font \
	ega \
	hgc \
	mda \
	mda_font \
	olivetti \
	olivetti_font \
	plantronics \
	vga \
	video \
	wy700 \
	wy700_font

DEV_VID_SRC := $(foreach f,$(DEV_VID_BAS),$(rel)/$(f).c)
DEV_VID_OBJ := $(foreach f,$(DEV_VID_BAS),$(rel)/$(f).o)
DEV_VID_HDR := $(foreach f,$(DEV_VID_BAS),$(rel)/$(f).h)
DEV_VID_ARC := $(rel)/video.a

CLN  += $(DEV_VID_ARC) $(DEV_VID_OBJ)
DIST += $(DEV_VID_SRC) $(DEV_VID_HDR)

$(rel)/cga.o:		$(rel)/cga.c
$(rel)/cga_font.o:	$(rel)/cga_font.c
$(rel)/ega.o:		$(rel)/ega.c
$(rel)/hgc.o:		$(rel)/hgc.c
$(rel)/mda.o:		$(rel)/mda.c
$(rel)/mda_font.o:	$(rel)/mda_font.c
$(rel)/olivetti.o:	$(rel)/olivetti.c
$(rel)/olivetti_font.o: $(rel)/olivetti_font.c
$(rel)/plantronics.o:	$(rel)/plantronics.c
$(rel)/vga.o:		$(rel)/vga.c
$(rel)/video.o:		$(rel)/video.c
$(rel)/wy700.o:		$(rel)/wy700.c
$(rel)/wy700_font.o:	$(rel)/wy700_font.c

$(rel)/video.a: $(DEV_VID_OBJ)
