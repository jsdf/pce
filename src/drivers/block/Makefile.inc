# src/drivers/block/Makefile.inc

rel := src/drivers/block

DIRS += $(rel)
DIST += $(rel)/Makefile.inc

DRV_BLK_BAS := \
	blkchd \
	blkcow \
	blkdosem \
	blkpart \
	blkpbi \
	blkpce \
	blkpri \
	blkpsi \
	blkqed \
	blkram \
	blkraw \
	block

DRV_BLK_SRC := $(foreach f,$(DRV_BLK_BAS),$(rel)/$(f).c) $(DRV_BLK_PFDC_SRC)
DRV_BLK_OBJ := $(foreach f,$(DRV_BLK_BAS),$(rel)/$(f).o) $(DRV_BLK_PFDC_OBJ)
DRV_BLK_HDR := $(foreach f,$(DRV_BLK_BAS),$(rel)/$(f).h) $(DRV_BLK_PFDC_HDR)
DRV_BLK_ARC := $(rel)/block.a

CLN  += $(DRV_BLK_ARC) $(DRV_BLK_OBJ)
DIST += $(DRV_BLK_SRC) $(DRV_BLK_HDR)

$(rel)/blkchd.o:	$(rel)/blkchd.c
$(rel)/blkcow.o:	$(rel)/blkcow.c
$(rel)/blkdosem.o:	$(rel)/blkdosem.c
$(rel)/blkpart.o:	$(rel)/blkpart.c
$(rel)/blkpbi.o:	$(rel)/blkpbi.c
$(rel)/blkpce.o:	$(rel)/blkpce.c
$(rel)/blkpri.o:	$(rel)/blkpri.c
$(rel)/blkpsi.o:	$(rel)/blkpsi.c
$(rel)/blkqed.o:	$(rel)/blkqed.c
$(rel)/blkram.o:	$(rel)/blkram.c
$(rel)/blkraw.o:	$(rel)/blkraw.c
$(rel)/block.o:		$(rel)/block.c

$(rel)/block.a: $(DRV_BLK_OBJ)
