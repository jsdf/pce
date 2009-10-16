# src/arch/ibmpc/bios/Makefile.inc

rel := src/arch/ibmpc/bios

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

NFLAGS := -Isrc/ -I$(srcdir)/src/arch/ibmpc/ -I$(srcdir)/src/arch/ibmpc/bios/

# ----------------------------------------------------------------------

PCE_IBMPC_PCEX_BAS := pcex
PCE_IBMPC_PCEX_SRC := $(rel)/pcex.asm
PCE_IBMPC_PCEX_ROM := $(rel)/pcex.rom
PCE_IBMPC_PCEX_BIN := $(rel)/ibmpc-pcex.rom
PCE_IBMPC_PCEX_SDP := src/arch/ibmpc/pce.inc
PCE_IBMPC_PCEX_REL := $(rel)

CLN  += $(PCE_IBMPC_PCEX_BIN) $(PCE_IBMPC_PCEX_OBJ)
DIST += $(PCE_IBMPC_PCEX_SRC) $(PCE_IBMPC_PCEX_ROM)

ifeq "$(PCE_BUILD_IBMPC)" "1"
TARGETS     += $(PCE_IBMPC_PCEX_BIN)
SHARE_IBMPC += $(PCE_IBMPC_PCEX_BIN)
endif

ifeq "$(PCE_BUILD_IBMPC_ROM)" "1"

$(rel)/ibmpc-pcex.rom: $(rel)/pcex.asm $(PCE_IBMPC_PCEX_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -DNOFILL -O6 -f bin -o $@ $<

else

$(rel)/ibmpc-pcex.rom: $(rel)/pcex.rom
	$(QP)echo "  CAT    $@"
	$(QR)cat $< > $@

endif

# ----------------------------------------------------------------------

PCE_IBMPC_EGA_SRC := $(rel)/ega.asm
PCE_IBMPC_EGA_HDR := $(rel)/fnt8x14.inc $(rel)/fnt8x8.inc
PCE_IBMPC_EGA_ROM := $(rel)/ega.rom
PCE_IBMPC_EGA_BIN := $(rel)/ibmpc-ega.rom
PCE_IBMPC_EGA_SDP := $(PCE_IBMPC_EGA_HDR) src/arch/ibmpc/pce.inc

CLN  += $(PCE_IBMPC_EGA_BIN) $(PCE_IBMPC_EGA_OBJ)
DIST += $(PCE_IBMPC_EGA_SRC) $(PCE_IBMPC_EGA_HDR) $(PCE_IBMPC_EGA_ROM)

ifeq "$(PCE_BUILD_IBMPC)" "1"
TARGETS     += $(PCE_IBMPC_EGA_BIN)
SHARE_IBMPC += $(PCE_IBMPC_EGA_BIN)
endif

ifeq "$(PCE_BUILD_IBMPC_ROM)" "1"

$(rel)/ibmpc-ega.rom: $(rel)/ega.asm $(PCE_IBMPC_EGA_SDP)
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) $(NFLAGS) -O6 -f bin -o $@ $<

else

$(rel)/ibmpc-ega.rom: $(rel)/ega.rom
	$(QP)echo "  CAT    $@"
	$(QR)cat $< > $@

endif
