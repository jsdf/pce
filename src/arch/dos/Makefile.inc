# src/arch/dos/Makefile.inc

rel := src/arch/dos

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

# ----------------------------------------------------------------------

PCE_DOS_CFLAGS  := $(CFLAGS_DEFAULT)
PCE_DOS_LDFLAGS := $(LDFLAGS_DEFAULT)
PCE_DOS_LIBS    := $(LIBS)

ifeq "$(PCE_HOST_WINDOWS)" "1"
	PCE_DOS_LIBS += -mconsole
endif

$(rel)/%.o: $(rel)/%.c
	$(QP)echo "  CC     $@"
	$(QR)$(CC) -c $(PCE_DOS_CFLAGS) -o $@ $<

# ----------------------------------------------------------------------

PCE_DOS_BAS := \
	dos \
	dosmem \
	exec \
	int \
	int10 \
	int21 \
	main \
	path

PCE_DOS_SRC  := $(foreach f,$(PCE_DOS_BAS),$(rel)/$(f).c)
PCE_DOS_OBJ  := $(foreach f,$(PCE_DOS_BAS),$(rel)/$(f).o)
PCE_DOS_HDR  := $(foreach f,$(PCE_DOS_BAS),$(rel)/$(f).h)
PCE_DOS_MAN1 := $(rel)/pce-dos.1
PCE_DOS_BIN  := $(rel)/pce-dos$(EXEEXT)

PCE_DOS_OBJ_EXT := \
	src/lib/getopt.o \
	src/lib/sysdep.o \
	$(CPU_8086_OBJ)

#ifeq "$(PCE_BUILD_DOS)" "1"
BIN  += $(PCE_DOS_BIN)
MAN1 += $(PCE_DOS_MAN1)
#endif

CLN  += $(PCE_DOS_BIN) $(PCE_DOS_OBJ)
DIST += $(PCE_DOS_SRC) $(PCE_DOS_HDR) $(PCE_DOS_MAN1)

$(rel)/dos.o:    $(rel)/dos.c
$(rel)/dosmem.o: $(rel)/dosmem.c
$(rel)/exec.o:   $(rel)/exec.c
$(rel)/int.o:    $(rel)/int.c
$(rel)/int10.o:  $(rel)/int10.c
$(rel)/int21.o:  $(rel)/int21.c
$(rel)/main.o:   $(rel)/main.c
$(rel)/path.o:   $(rel)/path.c

$(rel)/pce-dos$(EXEEXT): $(PCE_DOS_OBJ_EXT) $(PCE_DOS_OBJ)
	$(QP)echo "  LD     $@"
	$(QR)$(LD) $(PCE_DOS_LDFLAGS) -o $@ $(PCE_DOS_OBJ) $(PCE_DOS_OBJ_EXT) $(PCE_DOS_LIBS) -lm
