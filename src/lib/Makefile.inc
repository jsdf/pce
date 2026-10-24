# src/lib/Makefile.inc

rel := src/lib

DIRS += $(rel)
DIST += $(rel)/Makefile.inc $(rel)/stdint/stdint.h

LIBPCE_BAS := \
	brkpt \
	cfg \
	ciff \
	cmd \
	console \
	endian \
	getopt \
	ihex \
	iniata \
	inidsk \
	iniram \
	initerm \
	load \
	log \
	mhex \
	monitor \
	msg \
	msgdsk \
	path \
	srec \
	string \
	sysdep \
	text \
	thex

ifeq "$(PCE_ENABLE_TUN)" "1"
LIBPCE_BAS += tun
else
DIST += $(rel)/tun.c $(rel)/tun.h
endif

LIBPCE_SRC := $(foreach f,$(LIBPCE_BAS),$(rel)/$(f).c)
LIBPCE_OBJ := $(foreach f,$(LIBPCE_BAS),$(rel)/$(f).o)
LIBPCE_HDR := $(foreach f,$(LIBPCE_BAS),$(rel)/$(f).h)
LIBPCE_SDP := src/config.h

LIBPCE_LOAD_OBJ := $(rel)/ihex.o $(rel)/mhex.o $(rel)/srec.o $(rel)/thex.o $(rel)/load.o

CLN  += $(LIBPCE_OBJ)
DIST += $(LIBPCE_SRC) $(LIBPCE_HDR)

$(rel)/brkpt.o:		$(rel)/brkpt.c
$(rel)/cfg.o:		$(rel)/cfg.c
$(rel)/ciff.o:		$(rel)/ciff.c
$(rel)/cmd.o:		$(rel)/cmd.c
$(rel)/console.o:	$(rel)/console.c
$(rel)/endian.o:	$(rel)/endian.c
$(rel)/getopt.o:	$(rel)/getopt.c
$(rel)/ihex.o:		$(rel)/ihex.c
$(rel)/iniata.o:	$(rel)/iniata.c
$(rel)/inidsk.o:	$(rel)/inidsk.c
$(rel)/iniram.o:	$(rel)/iniram.c
$(rel)/initerm.o:	$(rel)/initerm.c
$(rel)/log.o:		$(rel)/log.c
$(rel)/mhex.o:		$(rel)/mhex.c
$(rel)/monitor.o:	$(rel)/monitor.c
$(rel)/msg.o:		$(rel)/msg.c
$(rel)/msgdsk.o:	$(rel)/msgdsk.c
$(rel)/path.o:		$(rel)/path.c
$(rel)/tun.o:		$(rel)/tun.c
$(rel)/srec.o:		$(rel)/srec.c
$(rel)/string.o:	$(rel)/string.c
$(rel)/sysdep.o:	$(rel)/sysdep.c
$(rel)/text.o:		$(rel)/text.c
$(rel)/thex.o:		$(rel)/thex.c
