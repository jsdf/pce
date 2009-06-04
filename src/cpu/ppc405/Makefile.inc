# src/cpu/ppc405/Makefile.inc

rel := src/cpu/ppc405

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_PPC405_BAS := disasm mmu opcode13 opcode1f opcodes ppc405
CPU_PPC405_SRC := $(foreach f,$(CPU_PPC405_BAS),$(rel)/$(f).c)
CPU_PPC405_OBJ := $(foreach f,$(CPU_PPC405_BAS),$(rel)/$(f).o)
CPU_PPC405_HDR := $(foreach f,ppc405 internal,$(rel)/$(f).h)
CPU_PPC405_ARC := $(rel)/ppc405.a

CLN  += $(CPU_PPC405_ARC) $(CPU_PPC405_OBJ)
DIST += $(CPU_PPC405_SRC) $(CPU_PPC405_HDR)

$(rel)/disasm.o:	$(rel)/disasm.c
$(rel)/mmu.o:		$(rel)/mmu.c
$(rel)/opcode13.o:	$(rel)/opcode13.c
$(rel)/opcode1f.o:	$(rel)/opcode1f.c
$(rel)/opcodes.o:	$(rel)/opcodes.c
$(rel)/ppc405.o:	$(rel)/ppc405.c

$(rel)/ppc405.a: $(CPU_PPC405_OBJ)
