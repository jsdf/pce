# src/cpu/e68000/Makefile.inc

rel := src/cpu/e68000

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_68K_BAS := cc disasm ea opcodes e68000
CPU_68K_SRC := $(foreach f,$(CPU_68K_BAS),$(rel)/$(f).c)
CPU_68K_OBJ := $(foreach f,$(CPU_68K_BAS),$(rel)/$(f).o)
CPU_68K_HDR := $(foreach f,e68000 internal,$(rel)/$(f).h)
CPU_68K_ARC := $(rel)/e68000.a

CLN  += $(CPU_68K_ARC) $(CPU_68K_OBJ)
DIST += $(CPU_68K_SRC) $(CPU_68K_HDR)

$(rel)/cc.o:		$(rel)/cc.c
$(rel)/disasm.o:	$(rel)/disasm.c
$(rel)/ea.o:		$(rel)/ea.c
$(rel)/opcodes.o:	$(rel)/opcodes.c
$(rel)/e68000.o:	$(rel)/e68000.c

$(rel)/e68000.a: $(CPU_68K_OBJ)
