# src/arch/ibmpc/pceutils/Makefile.inc

rel := src/arch/ibmpc/pceutils

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

$(rel)/%.com: $(rel)/%.asm
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) -O6 -Isrc/ -I$(srcdir)/src/arch/ibmpc/ -f bin $< -o $@

$(rel)/%.sys: $(rel)/%.asm
	$(QP)echo "  NASM   $@"
	$(QR)$(NASM) -O6 -Isrc/ -I$(srcdir)/src/arch/ibmpc/ -f bin $< -o $@


# ----------------------------------------------------------------------

PCEUTILS_BAS_COM := pceboot pceexit pcemsg pcestop pcetime pcever
PCEUTILS_BAS_SYS := pceemm pcehimem
PCEUTILS_BAS     := $(PCEUTILS_BAS_COM) $(PCEUTILS_BAS_SYS)
PCEUTILS_SRC     := $(foreach f,$(PCEUTILS_BAS),$(rel)/$(f).asm)
PCEUTILS_BIN_COM := $(foreach f,$(PCEUTILS_BAS_COM),$(rel)/$(f).com)
PCEUTILS_BIN_SYS := $(foreach f,$(PCEUTILS_BAS_SYS),$(rel)/$(f).sys)
PCEUTILS_BIN     := $(PCEUTILS_BIN_COM) $(PCEUTILS_BIN_SYS)
PCEUTILS_SDP     := src/config.inc src/arch/ibmpc/pce.inc

CLN  += $(PCEUTILS_BIN)
DIST += $(PCEUTILS_SRC)

ifeq "$(PCE_BUILD_IBMPC)" "1"
ifeq "$(PCE_HAVE_NASM)" "1"
TARGETS        += $(PCEUTILS_BIN)
SHARE_PCEUTILS := $(PCEUTILS_BIN)
endif
endif


$(rel)/pceboot.com:	$(rel)/pceboot.asm $(PCEUTILS_SDP)
$(rel)/pceemm.sys:	$(rel)/pceemm.asm $(PCEUTILS_SDP)
$(rel)/pceexit.com:	$(rel)/pceexit.asm $(PCEUTILS_SDP)
$(rel)/pcehimem.sys:	$(rel)/pcehimem.asm $(PCEUTILS_SDP)
$(rel)/pcemsg.com:	$(rel)/pcemsg.asm $(PCEUTILS_SDP)
$(rel)/pcestop.com:	$(rel)/pcestop.asm $(PCEUTILS_SDP)
$(rel)/pcetime.com:	$(rel)/pcetime.asm $(PCEUTILS_SDP)
$(rel)/pcever.com:	$(rel)/pcever.asm $(PCEUTILS_SDP)
