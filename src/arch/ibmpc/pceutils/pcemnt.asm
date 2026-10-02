;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pcemnt.asm
; Created:      2020-04-10 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2020 Hampa Hug <hampa@hampa.ch>
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


; pcemnt [drive] [file]


%include "pce.inc"


section .text

	org	0x100

	jmp	start

msg_usage	db "pcemnt version ", PCE_VERSION_STR , 0x0d, 0x0a
		db 0x0d, 0x0a
		db "usage: pcemnt [drive] [filename]", 0x0d, 0x0a
		db 0x00

msg_notpce	db "pcemnt: not running under PCE", 0x0d, 0x0a, 0x00
msg_error	db "pcemnt: error", 0x0d, 0x0a, 0x00

msg_id		db "disk.id", 0
msg_insert	db "disk.insert", 0


%define PCE_USE_PRINT_STRING 1
%define PCE_USE_PARSE_OPT    1
%define PCE_USE_HOOK_CHECK   1
%define PCE_USE_HOOK         1

%include "pce-lib.inc"


start:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax

	cld

	call	pce_hook_check
	jc	.notpce

	mov	si, 0x0080		; parameters

	lodsb				; paramter size
	mov	cl, al
	xor	ch, ch

	mov	di, str1
	call	pce_parse_opt		; drive or file name
	jc	.usage

	mov	di, str2
	call	pce_parse_opt		; file name
	mov	di, str1
	jc	.insert

.id:
	mov	si, msg_id
	mov	ax, PCE_HOOK_SET_MSG
	call	pce_hook		; set drive id
	jc	.error

	mov	di, str2

.insert:
	mov	si, msg_insert
	mov	ax, PCE_HOOK_SET_MSG
	call	pce_hook		; insert disk
	jc	.error

.done_ok:
	xor	al, al

.exit:
	mov	ah, 0x4c
	int	0x21
	int	0x20

.usage:
	mov	si, msg_usage
	call	pce_print_string
	jmp	.done_ok

.error:
	mov	si, msg_error
	jmp	.done_err

.notpce:
	mov	si, msg_notpce
	;jmp	.done_err

.done_err:
	call	pce_print_string
	mov	al, 0x01
	jmp	.exit


section	.bss

str1		resb 256
str2		resb 256
