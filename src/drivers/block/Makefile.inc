# src/drivers/block/Makefile.inc

rel := src/drivers/block

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DEV_BLK_BAS := blkcow blkdosem blkfdc blkpart blkpce blkqed blkram blkraw block
DEV_BLK_SRC := $(foreach f,$(DEV_BLK_BAS),$(rel)/$(f).c) $(DEV_BLK_PFDC_SRC)
DEV_BLK_OBJ := $(foreach f,$(DEV_BLK_BAS),$(rel)/$(f).o) $(DEV_BLK_PFDC_OBJ)
DEV_BLK_HDR := $(foreach f,$(DEV_BLK_BAS),$(rel)/$(f).h) $(DEV_BLK_PFDC_HDR)
DEV_BLK_ARC := $(rel)/block.a

CLN  += $(DEV_BLK_ARC) $(DEV_BLK_OBJ)
DIST += $(DEV_BLK_SRC) $(DEV_BLK_HDR)

$(rel)/blkcow.o:	$(rel)/blkcow.c
$(rel)/blkdosem.o:	$(rel)/blkdosem.c
$(rel)/blkfdc.o:	$(rel)/blkfdc.c
$(rel)/blkpart.o:	$(rel)/blkpart.c
$(rel)/blkpce.o:	$(rel)/blkpce.c
$(rel)/blkqed.o:	$(rel)/blkqed.c
$(rel)/blkram.o:	$(rel)/blkram.c
$(rel)/blkraw.o:	$(rel)/blkraw.c
$(rel)/block.o:		$(rel)/block.c

$(rel)/block.a: $(DEV_BLK_OBJ)
