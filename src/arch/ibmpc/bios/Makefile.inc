# src/arch/ibmpc/bios/Makefile.inc

rel := src/arch/ibmpc/bios

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

NFLAGS := -Isrc/ -I$(srcdir)/src/arch/ibmpc/ -I$(srcdir)/src/arch/ibmpc/bios/

# ----------------------------------------------------------------------

PCE_BIOS_BAS := pce
PCE_BIOS_SRC := $(foreach f,$(PCE_BIOS_BAS),$(rel)/$(f).asm)
PCE_BIOS_LST := $(foreach f,$(PCE_BIOS_BAS),$(rel)/$(f).lst)
PCE_BIOS_ROM := $(rel)/pce.rom
PCE_BIOS_HEX := $(rel)/pce.ihex
PCE_BIOS_SDP := src/arch/ibmpc/pce.inc
PCE_BIOS_REL := $(rel)

CLN  += $(PCE_BIOS_ROM) $(PCE_BIOS_HEX) $(PCE_BIOS_OBJ) $(PCE_BIOS_LST)
DIST += $(PCE_BIOS_SRC)

ifeq "$(PCE_BUILD_IBMPC)" "1"
ifeq "$(PCE_HAVE_NASM)" "1"
TARGETS += $(PCE_BIOS_ROM)
SHARE   += $(PCE_BIOS_ROM)
ifeq "$(PCE_HAVE_IHEX)" "1"
TARGETS += $(PCE_BIOS_HEX)
endif
endif
endif

$(rel)/pce.rom: $(rel)/pce.asm $(PCE_BIOS_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -DNOFILL -O6 -f bin -l $(PCE_BIOS_REL)/pce.lst -o $@ $<

$(rel)/pce.ihex: $(rel)/pce.rom
	$(QP)echo "  IHEX   $@"
	$(QR)$(IHEX) -e -o $@ \
	-s 0xf000 -a 0 -i $(PCE_BIOS_REL)/pce.rom

# ----------------------------------------------------------------------

PCE_EGA_BAS := ega
PCE_EGA_SRC := $(foreach f,$(PCE_EGA_BAS),$(rel)/$(f).asm)
PCE_EGA_HDR := $(rel)/fnt8x14.inc $(rel)/fnt8x8.inc
PCE_EGA_LST := $(foreach f,$(PCE_EGA_BAS),$(rel)/$(f).lst)
PCE_EGA_ROM := $(rel)/pce-ega.rom
PCE_EGA_HEX := $(rel)/pce-ega.ihex
PCE_EGA_SDP := $(PCE_EGA_HDR) src/arch/ibmpc/pce.inc

CLN  += $(PCE_EGA_ROM) $(PCE_EGA_HEX) $(PCE_EGA_OBJ) $(PCE_EGA_LST)
DIST += $(PCE_EGA_SRC) $(PCE_EGA_HDR)

ifeq "$(PCE_BUILD_IBMPC)" "1"
ifeq "$(PCE_HAVE_NASM)" "1"
TARGETS += $(PCE_EGA_ROM)
SHARE   += $(PCE_EGA_ROM)
ifeq "$(PCE_HAVE_IHEX)" "1"
TARGETS += $(PCE_EGA_HEX)
endif
endif
endif

$(rel)/pce-ega.rom: $(rel)/ega.asm $(PCE_EGA_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -O6 -f bin -l $(PCE_BIOS_REL)/pce-ega.lst -o $@ $<

$(rel)/pce-ega.ihex: $(rel)/pce-ega.rom
	$(QP)echo "  IHEX   $@"
	$(QR)$(IHEX) -e -o $@ -s 0xc000 -a 0 -i $(PCE_BIOS_REL)/pce-ega.rom
