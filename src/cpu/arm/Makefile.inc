# src/cpu/arm/Makefile.inc

rel := src/cpu/arm

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_ARM_BAS := arm copr14 copr15 disasm mmu opcodes
CPU_ARM_SRC := $(foreach f,$(CPU_ARM_BAS),$(rel)/$(f).c)
CPU_ARM_OBJ := $(foreach f,$(CPU_ARM_BAS),$(rel)/$(f).o)
CPU_ARM_HDR := $(foreach f,arm internal,$(rel)/$(f).h)
CPU_ARM_ARC := $(rel)/arm.a

CLN  += $(CPU_ARM_ARC) $(CPU_ARM_OBJ)
DIST += $(CPU_ARM_SRC) $(CPU_ARM_HDR)

$(rel)/arm.o:		$(rel)/arm.c
$(rel)/copr14.o:	$(rel)/copr14.c
$(rel)/copr15.o:	$(rel)/copr15.c
$(rel)/disasm.o:	$(rel)/disasm.c
$(rel)/mmu.o:		$(rel)/mmu.c
$(rel)/opcodes.o:	$(rel)/opcodes.c

$(rel)/arm.a: $(CPU_ARM_OBJ)
