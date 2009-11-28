;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   pcever.asm                                                   *
;* Created:     2003-10-13 by Hampa Hug <hampa@hampa.ch>                     *
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


; pcever
; Print the PCE version


%include "pce.inc"


section .text

	org	0x100

	jmp	start


str1	db "PCE version ", 0
str2	db " (", 0
str3	db ")", 0x0d, 0x0a, 0


; print the 16 bit unsigned integer in ax
prt_uint16:
	push	ax
	push	cx
	push	dx
	push	bx

	mov	bx, 10
	xor	cx, cx

.next1:
	xor	dx, dx

	div	bx
	add	dl, '0'
	push	dx
	inc	cx

	or	ax, ax
	jnz	.next1

.next2:
	pop	ax
	mov	ah, 0x0e
	xor	bx, bx
	int	0x10
	loop	.next2

	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


; print the character in al
prt_char:
	push	ax
	push	bx

	mov	ah, 0x0e
	xor	bx, bx
	int	0x10

	pop	bx
	pop	ax
	ret


; print the string at ds:di
prt_string:
	push	ax
	push	si

.next:
	mov	ah, 0x0e
	lodsb
	or	al, al
	jz	.done
	int	0x10
	jmp	.next

.done:
	pop	si
	pop	ax
	ret


start:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax

	mov	si, str1
	call	prt_string

	mov	di, vers
	pceh	PCEH_GET_VERS

	mov	si, vers
	call	prt_string

	mov	si, str2
	call	prt_string

	mov	cx, ax

	mov	ah, 0
	mov	al, ch
	call	prt_uint16

	mov	al, '.'
	call	prt_char

	mov	al, cl
	call	prt_uint16

	mov	al, '.'
	call	prt_char

	mov	al, dh
	call	prt_uint16

	mov	si, str3
	call	prt_string

	mov	ax, 0x4c00
	int	0x21

vers:
