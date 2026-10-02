/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   pcex.S                                                       *
 * Created:     2007-12-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2018 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#define MAC_HOOK_NOP    0
#define MAC_HOOK_STOP   1
#define MAC_HOOK_EXIT   2
#define MAC_HOOK_INSERT 3

#define MAC_HOOK_SONY        16
#define MAC_HOOK_SONY_OPEN   (MAC_HOOK_SONY + 0)
#define MAC_HOOK_SONY_PRIME  (MAC_HOOK_SONY + 1)
#define MAC_HOOK_SONY_CTRL   (MAC_HOOK_SONY + 2)
#define MAC_HOOK_SONY_STATUS (MAC_HOOK_SONY + 3)
#define MAC_HOOK_SONY_CLOSE  (MAC_HOOK_SONY + 4)

#define SONY_DRIVES   8
#define SONY_VAR_SIZE 0x0310

#define SONY_TRACK       0
#define SONY_WPROT       2
#define SONY_DISKINPLACE 3
#define SONY_INSTALLED   4
#define SONY_SIDES       5
#define SONY_QLINK       6
#define SONY_QTYPE       10
#define SONY_QDRIVENO    12
#define SONY_QREFNUM     14
#define SONY_QFSID       16
#define SONY_TWOSIDEFMT  18
#define SONY_NEWIF       19
#define SONY_DRIVEERRS   20


.section ".text"


start:
	.ascii	"PCEX"			/* marker */
	.long	0			/* version */

	.long	5			/* entry count */
	.long	(sony_driver - start)
	.long	(sony_check - start)
	.long	(sony_disk_icon - start)
	.long	(sony_disk_icon_pce - start)
	.long	(pcex_interrupt - start)


/* Sony driver */
sony_driver:
	.word	0x4f00			/* drvrFlags */
	.word	0x0000			/* drvrDelay */
	.word	0x0000			/* drvrEMask */
	.word	0x0000			/* drvrMenu */

	.word	(sony_open - sony_driver)	/* drvrOpen */
	.word	(sony_prime - sony_driver)	/* drvrPrime */
	.word	(sony_control - sony_driver)	/* drvrCtl */
	.word	(sony_status - sony_driver)	/* drvrStatus */
	.word	(sony_close - sony_driver)	/* drvrClose */

	.byte	5			/* drvrName */
	.ascii	".Sony"

	.align	2

/* sony - open */
sony_open:
	bsr.s	1f			/* set up data structures */

	illegal				/* signal the emulator */
	.word	MAC_HOOK_SONY_OPEN

	rts

1:
	movem.l	%d2/%a0-%a2, -(%a7)

	move.l	#SONY_VAR_SIZE, %d0
	.word	0xa71e			/* NewPtr */

	/*
	 * Clear the allocated memory
	 */
	move.l	%a0, %a2
	move.w	#(SONY_VAR_SIZE / 4 - 1), %d2
1:	clr.l	(%a2)+
	dbra	%d2, 1b

	move.l	%a1, (%a0)		/* dctl */
	move.l	%a0, 0x0134		/* SonyVars */

	/*
	 * System 6 unconditionally frees the first time manager task
	 * record enqueued by InsTime(). For this to work we install a
	 * dummy task here.
	 */
	cmpi.w	#0x0075, 0x400008	/* check if 64K ROM */
	bcs.s	1f
	move.l	%a0, -(%a7)
	lea	(16 + (SONY_DRIVES + 1) * 66)(%a0), %a0
	clr.l	6(%a0)			/* tmAddr */
	clr.l	10(%a0)			/* tmCount */
	.word	0xa058			/* InsTime */
	move.l	(%a7)+, %a0
1:

	/*
	 * initialize the drive queue. This is necessary for 64K ROMs.
	 */
	clr.w	0x308			/* DrvQHdr */
	clr.l	0x30a
	clr.l	0x30e

	lea	(8 + 1 * 66)(%a0), %a2
	moveq	#1, %d2

1:
	move.w	#0x0000, (%a2)		/* track */
	move.b	#0x00, SONY_WPROT(%a2)	/* write protected */
	move.b	#0x00, 3(%a2)		/* disk in place */
	move.b	#0x01, 4(%a2)		/* installed */
	move.b	#0xff, 5(%a2)		/* double sided drive */
	clr.l	6(%a2)			/* qLink */
	move.w	#3, 10(%a2)		/* qType */
	move.w	%d2, 12(%a2)		/* dqDrive */
	move.w	#0xfffb, 14(%a2)	/* dqRefNum */
	clr.w	16(%a2)			/* dqFSID */
	move.b	#0x00, 18(%a2)		/* double sided format */
	move.b	#0xff, 19(%a2)		/* new interface */
	move.w	#0x0000, 20(%a2)	/* drive errors */

	move.w	%d2, %d0		/* drive number */
	swap	%d0
	move.w	#0xfffb, %d0		/* driver ref */
	lea	6(%a2), %a0		/* qlink */
	.word	0xa04e			/* AddDrive */

	adda.w	#66, %a2
	addq.w	#1, %d2

	cmp.w	#(SONY_DRIVES), %d2
	bls	1b

	movem.l	(%a7)+, %d2/%a0-%a2

	moveq	#0, %d0

	move.w	%d0, 16(%a0)		/* ioResult */
	rts


/* sony - prime */
sony_prime:
	illegal
	.word	MAC_HOOK_SONY_PRIME
	rts


/* sony - control */
sony_control:
	illegal
	.word	MAC_HOOK_SONY_CTRL
	rts


/* sony - status */
sony_status:
	illegal
	.word	MAC_HOOK_SONY_STATUS
	rts


/* sony - close */
sony_close:
	moveq	#-24, %d0
	rts


/*
 * Generate disk insertion events for newly inserted disks
 */
sony_check:
	move.w	%sr, -(%a7)
	movem.l	%d0-%d7/%a0-%a6, -(%a7)

	movea.l	0x0134, %a1

	lea	(8 + 66 + 3)(%a1), %a1	/* disk in place */
	moveq.l	#1, %d1

1:
	cmp.b	#0x01, (%a1)
	bne.s	2f

	movea.w	#7, %a0			/* Disk Inserted */
	move.l	%d1, %d0		/* drive */
	.word	0xa02f			/* PostEvent */
	bra.s	3f

2:
	adda.w	#66, %a1
	add.w	#1, %d1
	cmp.w	#(SONY_DRIVES), %d1
	bls	1b

3:
	movem.l	(%a7)+, %d0-%d7/%a0-%a6
	move.w	(%a7)+, %sr
	rts


sony_disk_icon:
	.word	0x7fff, 0xfff0, 0x8100, 0x0108, 0x8100, 0x7104, 0x8100, 0x8902
	.word	0x8100, 0x8901, 0x8100, 0x8901, 0x8100, 0x8901, 0x8100, 0x8901
	.word	0x8100, 0x7101, 0x8100, 0x0101, 0x80ff, 0xfe01, 0x8000, 0x0001
	.word	0x8000, 0x0001, 0x8000, 0x0001, 0x87ff, 0xffe1, 0x8800, 0x0011
	.word	0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011
	.word	0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011
	.word	0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011
	.word	0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011, 0x7fff, 0xfffe

	.word	0x7fff, 0xfff0, 0xffff, 0xfff8, 0xffff, 0xfffc, 0xffff, 0xfffe
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7fff, 0xfffe
	.word	0x0000

sony_disk_icon_pce:
	.word	0x7fff, 0xfff0, 0x8100, 0x0108, 0x8100, 0x7104, 0x8100, 0x8902
	.word	0x8100, 0x8901, 0x8100, 0x8901, 0x8100, 0x8901, 0x8100, 0x8901
	.word	0x8100, 0x7101, 0x8100, 0x0101, 0x80ff, 0xfe01, 0x8000, 0x0001
	.word	0x8000, 0x0001, 0x8000, 0x0001, 0x87ff, 0xffe1, 0x8800, 0x0011
	.word	0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011
	.word	0x88fb, 0xde11, 0x884a, 0x5211, 0x884a, 0x1011, 0x887a, 0x1c11
	.word	0x8842, 0x1011, 0x8842, 0x5211, 0x88e3, 0xde11, 0x8800, 0x0011
	.word	0x8800, 0x0011, 0x8800, 0x0011, 0x8800, 0x0011, 0x7fff, 0xfffe

	.word	0x7fff, 0xfff0, 0xffff, 0xfff8, 0xffff, 0xfffc, 0xffff, 0xfffe
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	.word	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0x7fff, 0xfffe
	.word	0x0000


pcex_interrupt:
	movem.l	%d1-%d7/%a0-%a6, -(%sp)

	cmp.w	#0, %d0
	bne.s	1f

	dc.w	0xa856			/* ObscureCursor */

1:
	movem.l	(%sp)+, %d1-%d7/%a0-%a6
	move.l	(%sp)+, %d0
	rtr
