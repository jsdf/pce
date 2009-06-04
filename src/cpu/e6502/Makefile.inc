# src/cpu/e6502/Makefile.inc

rel := src/cpu/e6502

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_6502_BAS := disasm e6502 ea opcodes
CPU_6502_SRC := $(foreach f,$(CPU_6502_BAS),$(rel)/$(f).c)
CPU_6502_OBJ := $(foreach f,$(CPU_6502_BAS),$(rel)/$(f).o)
CPU_6502_HDR := $(foreach f,e6502 internal,$(rel)/$(f).h)
CPU_6502_ARC := $(rel)/e6502.a

CLN  += $(CPU_6502_ARC) $(CPU_6502_OBJ)
DIST += $(CPU_6502_SRC) $(CPU_6502_HDR)

$(rel)/disasm.o:	$(rel)/disasm.c
$(rel)/e6502.o:		$(rel)/e6502.c
$(rel)/ea.o:		$(rel)/ea.c
$(rel)/opcodes.o:	$(rel)/opcodes.c

$(rel)/e6502.a: $(CPU_6502_OBJ)
