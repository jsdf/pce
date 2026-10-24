;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pceexit.asm
; Created:      2003-04-25 by Hampa Hug <hampa@hampa.ch>
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


; pceexit - terminate the emulator


%include "pce.inc"


section .text

	org	0x100

	jmp	start


msg_notpce	db "not running under PCE", 0x0d, 0x0a, 0x00
msg_hookerr	db "error", 0x0d, 0x0a, 0x00


%define PCE_USE_PRINT_STRING 1
%define PCE_USE_HOOK_CHECK   1
%define PCE_USE_HOOK         1

%include "pce-lib.inc"


start:
	mov	ax, cs
	mov	ds, ax

	call	pce_hook_check
	jc	.notpce

	mov	ax, PCE_HOOK_ABORT
	call	pce_hook
	jc	.hookerr

	xor	al, al
	jmp	.exit

.notpce:
	mov	si, msg_notpce
	jmp	.err

.hookerr:
	mov	si, msg_hookerr

.err:
	call	pce_print_string
	mov	al, 0x01
	;jmp	.exit

.exit:
	mov	ah, 0x4c
	int	0x21
	int	0x20
