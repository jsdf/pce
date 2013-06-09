# src/utils/psi/Makefile.inc

rel := src/utils/psi

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

PSI_BAS := \
	comment \
	delete \
	edit \
	info \
	list \
	load \
	merge \
	new \
	reorder \
	save \
	main

PSI_SRC := $(foreach f,$(PSI_BAS),$(rel)/$(f).c)
PSI_OBJ := $(foreach f,$(PSI_BAS),$(rel)/$(f).o)
PSI_HDR := $(foreach f,$(PSI_BAS),$(rel)/$(f).h)
PSI_MAN1 := $(rel)/psi.1
PSI_BIN := $(rel)/psi$(EXEEXT)

PSI_OBJ_EXT := \
	src/drivers/psi/psi.a \
	src/lib/getopt.o

BIN  += $(PSI_BIN)
MAN1 += $(PSI_MAN1)
CLN  += $(PSI_BIN) $(PSI_OBJ)
DIST += $(PSI_SRC) $(PSI_HDR) $(PSI_MAN1)

$(rel)/comment.o: $(rel)/comment.c
$(rel)/delete.o:  $(rel)/delete.c
$(rel)/edit.o:    $(rel)/edit.c
$(rel)/info.o:    $(rel)/info.c
$(rel)/list.o:    $(rel)/list.c
$(rel)/load.o:    $(rel)/load.c
$(rel)/main.o:    $(rel)/main.c
$(rel)/main.o:    $(rel)/merge.c
$(rel)/new.o:     $(rel)/new.c
$(rel)/save.o:    $(rel)/save.c
$(rel)/reorder.o: $(rel)/reorder.c

$(rel)/psi$(EXEEXT): $(PSI_OBJ_EXT) $(PSI_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(LDFLAGS_DEFAULT) -o $@ $(PSI_OBJ) $(PSI_OBJ_EXT)
