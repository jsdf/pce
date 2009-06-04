# src/arch/ibmpc/bios/Makefile.inc

rel := src/arch/ibmpc/bios

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

NFLAGS := -Isrc/ -I$(srcdir)/src/arch/ibmpc/ -I$(srcdir)/src/arch/ibmpc/bios/

# ----------------------------------------------------------------------

BIOS_BAS  := pce basic bios
BIOS_SRC := $(foreach f,$(BIOS_BAS),$(rel)/$(f).asm)
BIOS_OBJ := $(foreach f,$(BIOS_BAS) pce2,$(rel)/$(f).bin)
BIOS_LST := $(foreach f,$(BIOS_BAS) pce2,$(rel)/$(f).lst)
BIOS_ROM := $(rel)/pce-bios.rom
BIOS_HEX := $(rel)/pce-bios.ihex
BIOS_SDP := src/arch/ibmpc/pce.inc
BIOS_REL := $(rel)

CLN  += $(BIOS_ROM) $(BIOS_HEX) $(BIOS_OBJ) $(BIOS_LST)
DIST += $(BIOS_SRC)

ifeq "$(PCE_BUILD_IBMPC)" "1"
ifeq "$(PCE_HAVE_NASM)" "1"
TARGETS += $(BIOS_ROM)
SHARE   += $(BIOS_ROM)
ifeq "$(PCE_HAVE_IHEX)" "1"
TARGETS += $(BIOS_HEX)
SHARE   += $(BIOS_HEX)
endif
endif
endif

$(rel)/pce.bin: $(rel)/pce.asm $(BIOS_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -O6 -f bin -l $(BIOS_REL)/pce.lst -o $@ $<

$(rel)/pce2.bin: $(rel)/pce.asm $(BIOS_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -DNOFILL -O6 -f bin -l $(BIOS_REL)/pce2.lst -o $@ $<

$(rel)/bios.bin: $(rel)/bios.asm $(BIOS_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -DPATCH -f bin -l $(BIOS_REL)/bios.lst -o $@ $<

$(rel)/basic.bin: $(rel)/basic.asm $(BIOS_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -f bin -l $(BIOS_REL)/basic.lst -o $@ $<

$(rel)/pce-bios.rom: $(BIOS_OBJ)
	$(QP)echo "  CAT    $@"
	$(QR)cat $(BIOS_REL)/pce.bin $(BIOS_REL)/basic.bin $(BIOS_REL)/bios.bin > $@

$(rel)/pce-bios.ihex: $(BIOS_OBJ)
	$(QP)echo "  IHEX   $@"
	$(QR)$(IHEX) -e -o $@ \
	-s 0xf000 -a 0 -i $(BIOS_REL)/pce2.bin \
	-s 0xf600 -a 0 -i $(BIOS_REL)/basic.bin \
	-s 0xfe00 -a 0 -i $(BIOS_REL)/bios.bin

# ----------------------------------------------------------------------

EGA_BAS := ega
EGA_SRC := $(foreach f,$(EGA_BAS),$(rel)/$(f).asm)
EGA_HDR := $(rel)/fnt8x14.inc $(rel)/fnt8x8.inc
EGA_OBJ := $(foreach f,$(EGA_BAS),$(rel)/$(f).bin)
EGA_LST := $(foreach f,$(EGA_BAS),$(rel)/$(f).lst)
EGA_ROM := $(rel)/pce-ega.rom
EGA_HEX := $(rel)/pce-ega.ihex
EGA_SDP := $(EGA_HDR) src/arch/ibmpc/pce.inc

CLN  += $(EGA_ROM) $(EGA_HEX) $(EGA_OBJ) $(EGA_LST)
DIST += $(EGA_SRC) $(EGA_HDR)

ifeq "$(PCE_BUILD_IBMPC)" "1"
ifeq "$(PCE_HAVE_NASM)" "1"
TARGETS += $(EGA_ROM)
SHARE   += $(EGA_ROM)
ifeq "$(PCE_HAVE_IHEX)" "1"
TARGETS += $(EGA_HEX)
SHARE   += $(EGA_HEX)
endif
endif
endif

$(rel)/ega.bin: $(rel)/ega.asm $(EGA_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -O6 -f bin -l $(BIOS_REL)/ega.lst -o $@ $<

$(rel)/pce-ega.rom: $(EGA_OBJ)
	$(QP)echo "  CAT    $@"
	$(QR)cat $(EGA_OBJ) > $@

$(rel)/pce-ega.ihex: $(EGA_OBJ)
	$(QP)echo "  IHEX   $@"
	$(QR)$(IHEX) -e -o $@ -s 0xc000 -a 0 -i $(BIOS_REL)/ega.bin

# ----------------------------------------------------------------------

CLN += $(rel)/pce-bios-ega.ihex

ifeq "$(PCE_BUILD_IBMPC)" "1"
ifeq "$(PCE_HAVE_NASM)" "1"
ifeq "$(PCE_HAVE_IHEX)" "1"
TARGETS += $(rel)/pce-bios-ega.ihex
SHARE   += $(rel)/pce-bios-ega.ihex
endif
endif
endif

$(rel)/pce-bios-ega.ihex: $(rel)/ega.bin $(rel)/pce2.bin $(rel)/basic.bin $(rel)/bios.bin
	$(QP)echo "  IHEX   $@"
	$(QR)$(IHEX) -e -o $@ \
	-s 0xc000 -a 0 -i $(BIOS_REL)/ega.bin \
	-s 0xf000 -a 0 -i $(BIOS_REL)/pce2.bin \
	-s 0xf600 -a 0 -i $(BIOS_REL)/basic.bin \
	-s 0xfe00 -a 0 -i $(BIOS_REL)/bios.bin
