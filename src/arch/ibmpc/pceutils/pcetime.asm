;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   pcetime.asm                                                  *
;* Created:     2003-04-25 by Hampa Hug <hampa@hampa.ch>                     *
;* Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
;*****************************************************************************

;*****************************************************************************
;* This program is free software. You can redistribute it and / or modify it *
;* under the terms of the GNU General Public License version 2 as  published *
;* by the Free Software Foundation.                                          *
;*                                                                           *
;* This program is distributed in the hope  that  it  will  be  useful,  but *
;* WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
;* Public License for more details.                                          *
;*****************************************************************************


; pcetime
; Set DOS date and time from the host


%include "pce.inc"


section text

	org	0x100

	jmp	start

; convert BCD in AL to binary in AX
get_bcd_2:
	push	dx
	xor	dx, dx
	mov	ah, al
	and	ax, 0xf00f
	shr	ah, 1
	add	dl, ah
	shr	ah, 1
	shr	ah, 1
	add	dl, ah
	add	dl, al
	xchg	ax, dx
	pop	dx
	ret


; set dos time from bios rtc time
set_time:
	mov	ah, 2
	int	0x1a

	mov	al, ch
	call	get_bcd_2
	mov	ch, al

	mov	al, cl
	call	get_bcd_2
	mov	cl, al

	mov	al, dh
	call	get_bcd_2
	mov	dh, al

	mov	dl, 0

	mov	ah, 0x2d
	int	0x21

	ret


; set dos date from bios rtc date
set_date:
	mov	ah, 4
	int	0x1a

	mov	al, cl
	call	get_bcd_2
	mov	cl, al

	mov	al, ch
	call	get_bcd_2
	mov	ah, 100
	mul	ah
	mov	ch, 0
	add	cx, ax

	mov	al, dh
	call	get_bcd_2
	mov	dh, al

	mov	al, dl
	call	get_bcd_2
	mov	dl, al

	mov	ah, 0x2b
	int	0x21

	ret


start:
	call	set_date
	call	set_time

	mov	ax, 0x4c00
	int	0x21
