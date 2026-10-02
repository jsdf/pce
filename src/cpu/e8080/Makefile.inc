# src/cpu/e8080/Makefile.inc

rel := src/cpu/e8080

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

CPU_8080_BAS := \
	dis_z80 \
	disasm \
	e8080 \
	flags \
	op_cb \
	op_dd \
	op_ed \
	op_fd \
	op_dd_cb \
	op_fd_cb \
	op_z80 \
	opcodes

CPU_8080_SRC := $(foreach f,$(CPU_8080_BAS),$(rel)/$(f).c)
CPU_8080_OBJ := $(foreach f,$(CPU_8080_BAS),$(rel)/$(f).o)
CPU_8080_HDR := $(foreach f,e8080 internal,$(rel)/$(f).h)
CPU_8080_ARC := $(rel)/e8080.a

CLN  += $(CPU_8080_ARC) $(CPU_8080_OBJ)
DIST += $(CPU_8080_SRC) $(CPU_8080_HDR)

$(rel)/dis_z80.o:  $(rel)/dis_z80.c
$(rel)/disasm.o:   $(rel)/disasm.c
$(rel)/e8080.o:    $(rel)/e8080.c
$(rel)/flags.o:    $(rel)/flags.c
$(rel)/op_cb.o:    $(rel)/op_cb.c
$(rel)/op_dd.o:    $(rel)/op_dd.c
$(rel)/op_ed.o:    $(rel)/op_ed.c
$(rel)/op_fd.o:    $(rel)/op_fd.c
$(rel)/op_dd_cb.o: $(rel)/op_dd_cb.c
$(rel)/op_fd_cb.o: $(rel)/op_fd_cb.c
$(rel)/op_z80.o:   $(rel)/op_z80.c
$(rel)/opcodes.o:  $(rel)/opcodes.c

$(rel)/e8080.a: $(CPU_8080_OBJ)
