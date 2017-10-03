;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pceclock.asm
; Created:      2012-02-11 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2012-2017 Hampa Hug <hampa@hampa.ch>
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


drv_start:
	dd	0xffffffff		; link to next driver
	dw	0x8008			; flags, clock character device
	dw	drv_strategy		; strategy offset
	dw	drv_interrupt		; interrupt offset
	db	"CLOCK$  "		; driver name

drv_funcs:
	dw	drv_init		; 00
	dw	drv_unknown		; 01
	dw	drv_unknown		; 02
	dw	drv_unknown		; 03
	dw	drv_read		; 04
	dw	drv_unknown		; 05
	dw	drv_input_status	; 06
	dw	drv_unknown		; 07
	dw	drv_write		; 08
	dw	drv_write		; 09
drv_funcs_end:


reqadr		dd	0
time		dd	0, 0
month_days	db	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31


%define PCE_USE_HOOK_CHECK 1
%define PCE_USE_HOOK_STOP  0
%define PCE_USE_HOOK       1

%include "pce-lib.inc"


;-----------------------------------------------------------------------------
; strategy function
;-----------------------------------------------------------------------------
drv_strategy:
	mov	[cs:reqadr + 0], bx
	mov	[cs:reqadr + 2], es
	retf


;-----------------------------------------------------------------------------
; interrupt function
;
; request:
;	0	1	record length
;	1	1	unit number
;	2	1	function
;	3	2	status
;	5	8	reserved
;-----------------------------------------------------------------------------
drv_interrupt:
	pushf
	push	ax
	push	bx
	push	si
	push	ds

	lds	bx, [cs:reqadr]

	xor	ah, ah
	mov	al, [bx + 2]		; function
	shl	ax, 1

	cmp	ax, (drv_funcs_end - drv_funcs)
	jae	.err

	mov	si, ax

	call	[cs:drv_funcs + si]

.done:
	mov	word [bx + 3], ax	; status

	pop	ds
	pop	si
	pop	bx
	pop	ax
	popf
	retf

.err:
	call	drv_unknown
	jmp	.done


;-----------------------------------------------------------------------------
; unknown function
;-----------------------------------------------------------------------------
drv_unknown:
	mov	ax, 0x8103
	ret


;-----------------------------------------------------------------------------
; func 04: read
;	13	1	media descriptor
;	14	4	buffer
;	18	2	byte count
;-----------------------------------------------------------------------------
drv_read:
	push	cx
	push	dx
	push	di
	push	es
	push	bx

	cmp	word [bx + 18], 6	; byte count
	jb	.err

	les	di, [bx + 14]		; buffer

	mov	ax, PCE_HOOK_GET_TIME_LOCAL
	call	pce_hook		; get host time
	jc	.err

	mov	word [es:di + 2], cx	; hours / minutes
	mov	word [es:di + 4], dx	; seconds / centiseconds

	cmp	bh, 11			; month
	ja	.err

	sub	ax, 1980		; year
	jc	.err

	mov	cx, ax
	shr	ax, 1
	shr	ax, 1
	mov	dx, 4 * 365 + 1
	mul	dx

	inc	ax			; leap day

	and	cl, 3
	jnz	.next_year

	cmp	bh, 2			; check if jan or feb
	sbb	ax, 0			; subtract leap day
	jmp	.leap

.next_year:
	add	ax, 365
	dec	cl
	jnz	.next_year

.leap:
	mov	cx, bx			; month / mday
	or	ch, ch			; month
	jz	.last_month

	mov	bx, month_days

.next_month:
	add	al, [cs:bx]		; days in month
	adc	ah, 0
	inc	bx
	dec	ch
	jnz	.next_month

.last_month:
	xor	ch, ch
	add	ax, cx			; day of month

	mov	word [es:di], ax	; days since 1980-01-01

	mov	cx, 6			; byte count
	mov	ax, 0x0100		; status

.done:
	pop	bx
	mov	word [bx + 18], cx	; byte count
	pop	es
	pop	di
	pop	dx
	pop	cx
	ret

.err:
	xor	cx, cx			; byte count
	mov	ax, 0x8100		; status
	jmp	.done


;-----------------------------------------------------------------------------
; func 06: input status
;-----------------------------------------------------------------------------
drv_input_status:
	mov	ax, 0x0100
	ret


;-----------------------------------------------------------------------------
; func 08: write
;	13	1	media descriptor
;	14	4	buffer
;	18	2	byte count
;-----------------------------------------------------------------------------
drv_write:
	mov	ax, 0x8100
	ret


drv_end:


;-----------------------------------------------------------------------------
; func 00: init driver
;	13	1	units
;	14	4	end address
;	18	4	param address
;-----------------------------------------------------------------------------
drv_init:
	push	si

	mov	si, msg_init
	call	print_string

	call	pce_hook_check
	jc	.nopce

	push	bx
	push	cx
	push	dx
	mov	ax, PCE_HOOK_GET_TIME_LOCAL
	call	pce_hook
	pop	dx
	pop	cx
	pop	bx
	jc	.nortc

	mov	word [bx + 14], drv_end	; end address
	mov     word [bx + 16], cs

	xor	ax, ax
	mov	byte [bx + 13], al	; units
	mov	word [bx + 18], ax	; param address
	mov	word [bx + 20], ax
	mov	word [bx + 23], ax	; message flag

	mov	ax, 0x0100		; status

.done:
	mov	si, msg_nl
	call	print_string

	pop	si
	ret

.nopce:
	mov	si, msg_nopce
	call	print_string
	jmp	.err

.nortc:
	mov	si, msg_nortc
	call	print_string

.err:
	mov	word [bx + 14], 0x0000	; end address
	mov	word [bx + 16], cs

	mov	ax, 0x8100		; status
	jmp	.done


;-----------------------------------------------------------------------------
; print the string at CS:SI
;-----------------------------------------------------------------------------
print_string:
	push	ax
	push	bx
	push	si

.next:
	cs
	lodsb
	or	al, al
	jz	.done
	mov	ah, 0x0e
	mov	bx, 0x0007
	int	0x10
	jmp	.next

.done:
	pop	si
	pop	bx
	pop	ax
	ret


msg_init	db	"PCE clock driver version ", PCE_VERSION_STR
msg_nl		db	0x0d, 0x0a, 0x00
msg_nopce	db	"pceclock: not running under PCE", 0x0d, 0x0a, 0x00
msg_nortc	db	"pceclock: RTC not supported", 0x0d, 0x0a, 0x00
