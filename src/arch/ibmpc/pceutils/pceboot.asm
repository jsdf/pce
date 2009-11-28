;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   pceboot.asm                                                  *
;* Created:     2003-09-01 by Hampa Hug <hampa@hampa.ch>                     *
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


; pceboot drive [r]
; set the boot drive and optionally reboot


%include "pce.inc"


section .text

	org	0x100

start:
	mov	ax, cs
	mov	ds, ax

	mov	si, 0x0080		; SI points to parameters

	lodsb
	mov	ah, 0
	mov	cx, ax			; parameter size in CX

	jcxz	done

	xor	dx, dx			; init drive number

skipspace1:
	cmp	byte [si], ' '
	jne	nextdigit
	inc	si
	loop	skipspace1

	jmp	done

nextdigit:
	mov	al, [si]
	cmp	al, '0'
	jb	digitdone
	cmp	al, '9'
	ja	digitdone

	inc	si

	mov	dh, dl
	shl	dl, 1
	shl	dl, 1
	shl	dl, 1
	add	dl, dh
	add	dl, dh
	sub	al, '0'
	add	dl, al
	loop	nextdigit

digitdone:
	mov	al, dl
	pceh	PCEH_SET_BOOT

skipspace2:
	jcxz	done

	lodsb
	dec	cx

	cmp	al, ' '
	je	skipspace2

	cmp	al, 'r'
	je	reboot
	cmp	al, 'R'
	je	reboot

	jmp	done

reboot:
	jmp	0xffff:0x0000

done:
	mov	ax, 0x4c00
	int	0x21
