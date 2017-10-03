;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pcemsg.asm
; Created:      2004-09-17 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2004-2017 Hampa Hug <hampa@hampa.ch>
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


; pcemsg [msg [val]]
; send a message to the emulation core


%include "pce.inc"


section .text

	org	0x100

	jmp	start


msg_notpce	db "pcemsg: not running under PCE", 0x0d, 0x0a, 0x00
msg_error	db "pcemsg: error", 0x0d, 0x0a, 0x00


%define PCE_USE_PRINT_STRING 1
%define PCE_USE_HOOK_CHECK   1
%define PCE_USE_HOOK         1

%include "pce-lib.inc"


; copy a string
pce_set_string:
	push	ax
	push	dx

.skip:
	jcxz	.done
	lodsb
	dec	cx
	cmp	al, 32
	jbe	.skip

	cmp	al, '"'
	je	.quote

.next:
	stosb
	jcxz	.done
	lodsb
	dec	cx
	cmp	al, 32
	je	.done
	jmp	.next

.quote:
	jcxz	.done
	lodsb
	dec	cx
	cmp	al, '"'
	je	.done
	stosb
	jmp	.quote

.done:
	xor	al, al
	stosb

	pop	dx
	pop	ax
	ret


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

	mov	di, str_msg
	call	pce_set_string

	mov	di, str_val
	call	pce_set_string

	mov	si, str_msg
	mov	di, str_val
	mov	ax, PCE_HOOK_SET_MSG
	call	pce_hook
	jc	.error

.done:
	xor	al, al
	jmp	.exit

.error:
	mov	si, msg_error
	jmp	.done_err

.notpce:
	mov	si, msg_notpce
	;jmp	.done_err

.done_err:
	call	pce_print_string
	mov	ax, 0x4c01
	int	0x21

.exit:
	mov	ah, 0x4c
	int	0x21
	int	0x20


section	.bss

str_msg		resb 256
str_val		resb 256
