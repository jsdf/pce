# src/drivers/char/Makefile.inc

rel := src/drivers/char

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_CHR_BAS  := char char-null char-stdio
DRV_CHR_NBAS :=

ifeq "$(PCE_ENABLE_CHAR_POSIX)" "1"
DRV_CHR_BAS += char-posix
else
DRV_CHR_NBAS += char-posix
endif

ifeq "$(PCE_ENABLE_CHAR_PPP)" "1"
DRV_CHR_BAS += char-ppp
else
DRV_CHR_NBAS += char-ppp
endif

ifeq "$(PCE_ENABLE_CHAR_PTY)" "1"
DRV_CHR_BAS += char-pty
else
DRV_CHR_NBAS += char-pty
endif

ifeq "$(PCE_ENABLE_CHAR_SLIP)" "1"
DRV_CHR_BAS += char-slip
else
DRV_CHR_NBAS += char-slip
endif

ifeq "$(PCE_ENABLE_CHAR_TCP)" "1"
DRV_CHR_BAS += char-tcp
else
DRV_CHR_NBAS += char-tcp
endif

ifeq "$(PCE_ENABLE_CHAR_TIOS)" "1"
DRV_CHR_BAS += char-tios
else
DRV_CHR_NBAS += char-tios
endif

DRV_CHR_SRC  := $(foreach f,$(DRV_CHR_BAS),$(rel)/$(f).c)
DRV_CHR_NSRC := $(foreach f,$(DRV_CHR_NBAS),$(rel)/$(f).c)
DRV_CHR_OBJ  := $(foreach f,$(DRV_CHR_BAS),$(rel)/$(f).o)
DRV_CHR_NOBJ := $(foreach f,$(DRV_CHR_NBAS),$(rel)/$(f).o)
DRV_CHR_HDR  := $(foreach f,$(DRV_CHR_BAS),$(rel)/$(f).h)
DRV_CHR_NHDR := $(foreach f,$(DRV_CHR_NBAS),$(rel)/$(f).h)
DRV_CHR_ARC  := $(rel)/char.a

CLN  += $(DRV_CHR_OBJ) $(DRV_CHR_NOBJ) $(DRV_CHR_ARC)
DIST += $(DRV_CHR_SRC) $(DRV_CHR_HDR) $(DRV_CHR_NSRC) $(DRV_CHR_NHDR)

$(rel)/char.o:		$(rel)/char.c
$(rel)/char-null.o:	$(rel)/char-null.c
$(rel)/char-posix.o:	$(rel)/char-posix.c
$(rel)/char-ppp.o:	$(rel)/char-ppp.c
$(rel)/char-pty.o:	$(rel)/char-pty.c
$(rel)/char-slip.o:	$(rel)/char-slip.c
$(rel)/char-stdio.o:	$(rel)/char-stdio.c
$(rel)/char-tcp.o:	$(rel)/char-tcp.c
$(rel)/char-tios.o:	$(rel)/char-tios.c

$(rel)/char.a: $(DRV_CHR_OBJ)
