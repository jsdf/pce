# src/arch/macplus/pcex/Makefile.inc

rel := src/arch/macplus/pcex

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

MACX_CROSS := m68k-linux-

MACX_CC := $(MACX_CROSS)gcc
MACX_LD := $(MACX_CROSS)ld
MACX_OC := $(MACX_CROSS)objcopy

MACX_CFLAGS = -m68000

MACX_ADDR := 0xf80000

# ----------------------------------------------------------------------

PCE_MACPLUS_PCEX_SRC := $(rel)/pcex.S
PCE_MACPLUS_PCEX_OBJ := $(rel)/pcex.o
PCE_MACPLUS_PCEX_ELF := $(rel)/pcex.elf
PCE_MACPLUS_PCEX_ROM := $(rel)/pcex.rom
PCE_MACPLUS_PCEX_BIN := $(rel)/macplus-pcex.rom
PCE_MACPLUS_PCEX_CLN := pcex.o pcex.elf macplus-pcex.rom

CLN  += $(foreach f,$(PCE_MACPLUS_PCEX_CLN),$(rel)/$(f))
DIST += $(PCE_MACPLUS_PCEX_SRC) $(PCE_MACPLUS_PCEX_ROM)

ifeq "$(PCE_BUILD_MACPLUS)" "1"
TARGETS       += $(PCE_MACPLUS_PCEX_BIN)
SHARE_MACPLUS += $(PCE_MACPLUS_PCEX_BIN)
endif


ifeq "$(PCE_BUILD_MACPLUS_ROM)" "1"
$(rel)/macplus-pcex.rom: $(rel)/pcex.S
	$(QP)echo "  CC68   $(PCE_MACPLUS_PCEX_OBJ)"
	$(QR)$(MACX_CC) $(MACX_CFLAGS) -c -o $(PCE_MACPLUS_PCEX_OBJ) $<

	$(QP)echo "  LD68   $(PCE_MACPLUS_PCEX_ELF)"
	$(QR)$(MACX_LD) --entry=0 -Ttext $(MACX_ADDR) -o $(PCE_MACPLUS_PCEX_ELF) $(PCE_MACPLUS_PCEX_OBJ)

	$(QP)echo "  OC68   $(PCE_MACPLUS_PCEX_ROM)"
	$(QR)$(MACX_OC) --output-target binary $(PCE_MACPLUS_PCEX_ELF) $@
	$(QR)chmod 644 $@
else
$(rel)/macplus-pcex.rom: $(rel)/pcex.rom
	$(QP)echo "  CAT    $@"
	$(QR)cat $< > $@
endif
