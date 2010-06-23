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
