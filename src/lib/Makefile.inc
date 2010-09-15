# src/lib/Makefile.inc

rel := src/lib

DIRS += $(rel)
DIST += $(rel)/Makefile.inc $(rel)/stdint/stdint.h

LIBPCE_BAS := \
	brkpt \
	cmd \
	console \
	getopt \
	ihex \
	iniata \
	inidsk \
	iniram \
	initerm \
	load \
	log \
	monitor \
	msg \
	path \
	srec \
	string \
	sysdep

ifeq "$(PCE_ENABLE_TUN)" "1"
LIBPCE_BAS += tun
else
DIST += $(rel)/tun.c $(rel)/tun.h
endif

LIBPCE_SRC := $(foreach f,$(LIBPCE_BAS),$(rel)/$(f).c)
LIBPCE_OBJ := $(foreach f,$(LIBPCE_BAS),$(rel)/$(f).o)
LIBPCE_HDR := $(foreach f,$(LIBPCE_BAS),$(rel)/$(f).h)
LIBPCE_SDP := src/config.h

CLN  += $(LIBPCE_OBJ)
DIST += $(LIBPCE_SRC) $(LIBPCE_HDR)

$(rel)/brkpt.o:		$(rel)/brkpt.c
$(rel)/cmd.o:		$(rel)/cmd.c
$(rel)/console.o:	$(rel)/console.c
$(rel)/getopt.o:	$(rel)/getopt.c
$(rel)/ihex.o:		$(rel)/ihex.c
$(rel)/iniata.o:	$(rel)/iniata.c
$(rel)/inidsk.o:	$(rel)/inidsk.c
$(rel)/iniram.o:	$(rel)/iniram.c
$(rel)/initerm.o:	$(rel)/initerm.c
$(rel)/log.o:		$(rel)/log.c
$(rel)/monitor.o:	$(rel)/monitor.c
$(rel)/msg.o:		$(rel)/msg.c
$(rel)/path.o:		$(rel)/path.c
$(rel)/tun.o:		$(rel)/tun.c
$(rel)/srec.o:		$(rel)/srec.c
$(rel)/string.o:	$(rel)/string.c
$(rel)/sysdep.o:	$(rel)/sysdep.c
