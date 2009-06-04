# Makefile

CROSS := m68k-linux-

AS := $(CROSS)as
CC := $(CROSS)gcc
LD := $(CROSS)ld
OC := $(CROSS)objcopy

AFLAGS = -m68000
LDFLAGS =

ADDR := 0xf80000

SRC := pcex.S

SREC := pcex.srec
IHEX := pcex.ihex
OBJ := pcex.o
BIN := pcex.bin


all: $(BIN)

srec: $(SREC)

ihex: $(IHEX)


clean:
	rm -f $(BIN) $(OBJ) $(SREC) $(IHEX) tmp.elf


%.o: %.S
	$(CC) $(AFLAGS) -c $< -o $@


%.bin: %.o
	$(LD) $(LDFLAGS) --entry=0 -Ttext $(ADDR) $< -o tmp.elf
	$(OC) --output-target binary tmp.elf $@
	rm -f tmp.elf
	chmod 644 $@


%.ihex: %.bin
	ihex -o $@ -a $(ADDR) -i $<


%.srec: %.bin
	srec -o $@ -a $(ADDR) -i $<


pcex.bin: pcex.o

pcex.o:	pcex.S

pcex.ihex: pcex.bin
pcex.srec: pcex.bin
