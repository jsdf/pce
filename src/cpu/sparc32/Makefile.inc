# src/cpu/sparc32/Makefile.inc

rel := src/cpu/sparc32

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_SPARC32_BAS := disasm mmu opcodes sparc32
CPU_SPARC32_SRC := $(foreach f,$(CPU_SPARC32_BAS),$(rel)/$(f).c)
CPU_SPARC32_OBJ := $(foreach f,$(CPU_SPARC32_BAS),$(rel)/$(f).o)
CPU_SPARC32_HDR := $(foreach f,sparc32 internal,$(rel)/$(f).h)
CPU_SPARC32_ARC := $(rel)/sparc32.a

CLN  += $(CPU_SPARC32_ARC) $(CPU_SPARC32_OBJ)
DIST += $(CPU_SPARC32_SRC) $(CPU_SPARC32_HDR)

$(rel)/disasm.o:	$(rel)/disasm.c
$(rel)/mmu.o:		$(rel)/mmu.c
$(rel)/opcodes.o:	$(rel)/opcodes.c
$(rel)/sparc32.o:	$(rel)/sparc32.c

$(rel)/sparc32.a: $(CPU_SPARC32_OBJ)
