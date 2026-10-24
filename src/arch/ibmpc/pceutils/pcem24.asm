;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pcem24.asm
; Created:      2017-09-10 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2017-2020 Hampa Hug <hampa@hampa.ch>
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; This program is free software. You can redistribute it and / or modify it
; under the terms of the GNU General Public License version 2 as  published
; by the Free Software Foundation.
;
; This program is distributed in the hope  that  it  will  be  useful,  but
; WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General
; Public License for more details.
;-----------------------------------------------------------------------------


%include "pce.inc"


section .text

	org	0x0100

entry:
	jmp	start


oldint10	dw 0, 0


;-----------------------------------------------------------------------------
; Set the M24 640 * 400 * 2 mode
;-----------------------------------------------------------------------------
m24_set:
	pushf
	push	ax
	push	cx
	push	dx
	push	di
	push	es

	push	ax

	; set cga mode 6
	mov	ax, 0x0006
	pushf
	call	far [cs:oldint10]

	; enable m20 high res mode
	mov	dx, 0x03de
	mov	al, 0x01
	out	dx, al

	pop	ax

	; set the BIOS video mode
	mov	dx, 0x0040
	mov	es, dx
	mov	byte [es:0x0049], al

	; clear rest of video memory
	mov	ax, 0xbc00
	mov	es, ax
	xor	di, di
	mov	cx, 0x4000 / 2
	xor	ax, ax
	cld
	rep	stosw

	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	ax
	popf
	ret


;-----------------------------------------------------------------------------
; Reset high res mode
;-----------------------------------------------------------------------------
m24_reset:
	push	ax
	push	dx

	; clear m20 high res mode
	mov	dx, 0x03de
	xor	al, al
	out	dx, al

	pop	dx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; INT 10 handler
;-----------------------------------------------------------------------------
newint10:
	or	ah, ah			; check if set mode
	jnz	.skip

	cmp	al, 0x40		; 640 * 400 (80 * 25)
	je	.hires
	cmp	al, 0x48		; 640 * 400 (80 * 50)
	je	.hires
	cmp	al, 0x06
	jbe	.lores

	iret

.lores:
	call	m24_reset		; reset high res mode
	jmp	.skip

.hires:
	call	m24_set			; set high res mode
	iret

.skip:
	jmp	far [cs:oldint10]

res_end:


msg	db	"Olivetti M24 BIOS extension version ", PCE_VERSION_STR
	db	0x0d, 0x0a, 0


%define PCE_USE_PRINT_STRING 1

%include "pce-lib.inc"


;-----------------------------------------------------------------------------
; Initialization
;-----------------------------------------------------------------------------
start:
	mov	ax, cs
	mov	ds, ax

	mov	si, msg
	call	pce_print_string	; print message

	mov	ax, 0x3510
	int	0x21			; get int 0x10 address

	mov	[oldint10 + 0], bx
	mov	[oldint10 + 2], es

	mov	ax, 0x2510
	mov	dx, newint10
	int	0x21			; set int 0x10 address

	mov	ax, 0x3100
	mov	dx, (0x100 + res_end - entry + 15) / 16
	int	0x21			; terminate and stay resident

.notreached:
	jmp	.notreached
