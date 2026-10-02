# Makefile

#CROSS = powerpc-linux-

CC = $(CROSS)gcc
LD = $(CROSS)gcc

CFLAGS = -Os -Wall
AFLAGS =

prefix = inst
bindir = $(prefix)/bin

BIN := pcecp pcemsg pcetime pcever

all: $(BIN)

clean:
	rm -f pce.o
	rm -f pcecp pcemsg pcetime pcever
	rm -f pcecp.o pcemsg.o pcetime.o pcever.o

install:
	mkdir -p $(bindir)
	cp $(BIN) $(bindir)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(AFLAGS) -c -o $@ $<

pce.o:     pce.S
pcecp.o:   pcecp.c pce.h config.h
pcemsg.o:  pcemsg.c pce.h config.h
pcetime.o: pcetime.c pce.h config.h
pcever.o:  pcever.c pce.h config.h

pcecp: pcecp.o pce.o
	$(LD) $(LDFLAGS) -o $@ pce.o pcecp.o

pcemsg: pcemsg.o pce.o
	$(LD) $(LDFLAGS) -o $@ pce.o pcemsg.o

pcetime: pcetime.o pce.o
	$(LD) $(LDFLAGS) -o $@ pce.o pcetime.o

pcever: pcever.o pce.o
	$(LD) $(LDFLAGS) -o $@ pce.o pcever.o
