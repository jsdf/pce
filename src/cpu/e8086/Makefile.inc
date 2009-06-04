# src/cpu/e8086/Makefile.inc

rel := src/cpu/e8086

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_8086_BAS := disasm e8086 e80186 e80286r flags ea opcodes pqueue
CPU_8086_SRC := $(foreach f,$(CPU_8086_BAS),$(rel)/$(f).c)
CPU_8086_OBJ := $(foreach f,$(CPU_8086_BAS),$(rel)/$(f).o)
CPU_8086_HDR := $(foreach f,e8086 internal,$(rel)/$(f).h)
CPU_8086_ARC := $(rel)/e8086.a

CLN  += $(CPU_8086_ARC) $(CPU_8086_OBJ)
DIST += $(CPU_8086_SRC) $(CPU_8086_HDR)

$(rel)/disasm.o:	$(rel)/disasm.c
$(rel)/e8086.o:		$(rel)/e8086.c
$(rel)/e80186.o:	$(rel)/e80186.c
$(rel)/e80286r.o:	$(rel)/e80286r.c
$(rel)/flags.o:		$(rel)/flags.c
$(rel)/ea.o:		$(rel)/ea.c
$(rel)/opcodes.o:	$(rel)/opcodes.c
$(rel)/pqueue.o:	$(rel)/pqueue.c

$(rel)/e8086.a: $(CPU_8086_OBJ)
