;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pcever.asm
; Created:      2003-10-13 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2003-2017 Hampa Hug <hampa@hampa.ch>
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


; pcever - print the PCE version


%include "pce.inc"


section .text

	org	0x100

	jmp	start


%define PCE_USE_PRINT_CHAR    1
%define PCE_USE_PRINT_STRING  1
%define PCE_USE_PRINT_UINT16  1
%define PCE_USE_HOOK_CHECK    1
%define PCE_USE_HOOK          1

%include "pce-lib.inc"


start:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax

	cld

	call	pce_hook_check		; check for pce
	jc	.notpce

	mov	di, buffer
	mov	cx, 256
	mov	ax, PCE_HOOK_GET_VERSION_STR
	call	pce_hook		; get version string
	jc	.error

	mov	si, msg_str1
	call	pce_print_string
	mov	si, buffer
	call	pce_print_string
	mov	si, msg_str2
	call	pce_print_string

	mov	ax, PCE_HOOK_GET_VERSION
	call	pce_hook		; get version
	jc	.error

	xchg	cx, ax

	xor	ah, ah
	mov	bx, 10			; base

	mov	al, dh			; major
	call	pce_print_uint16
	mov	al, '.'
	call	pce_print_char
	mov	al, dl			; minor
	call	pce_print_uint16
	mov	al, '.'
	call	pce_print_char
	mov	al, ch			; micro
	call	pce_print_uint16
	mov	si, msg_str3
	call	pce_print_string

	xor	al, al
	jmp	.exit

.notpce:
	mov	si, msg_notpce
	jmp	.done_err

.error:
	mov	si, msg_error
	;jmp	.done_err

.done_err:
	push	si
	mov	si, msg_arg0
	call	pce_print_string
	pop	si
	call	pce_print_string
	mov	al, 0x01

.exit:
	mov	ah, 0x4c
	int	0x21
	int	0x20


section	.data

msg_arg0	db "pcever: "
msg_notpce	db "not running under PCE", 0x0d, 0x0a, 0
msg_error	db "error", 0x0d, 0x0a, 0

msg_str1	db "PCE version ", 0
msg_str2	db " (", 0
msg_str3	db ")", 0x0d, 0x0a, 0


section	.bss

buffer		resb	256
