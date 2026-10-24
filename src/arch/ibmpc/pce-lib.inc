;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    src/arch/ibmpc/pce-lib.inc
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


%ifndef PCE_LIB_INC
%define PCE_LIB_INC 1


%include "pce.inc"


%macro pce_set_pos 1
	times %1 - ($ - $$) db 0
%endmacro


%ifdef PCE_USE_PRINT_CHAR
;-----------------------------------------------------------------------------
; Print a character
;
; inp: AL = character
;-----------------------------------------------------------------------------
pce_print_char:
	push	ax
	push	bx

	mov	ah, 0x0e
	mov	bx, 0x0007
	int	0x10

	pop	bx
	pop	ax
	ret
%endif


%ifdef PCE_USE_PRINT_STRING
;-----------------------------------------------------------------------------
; Print a string
;
; inp: DS:SI = pointer to the string
;-----------------------------------------------------------------------------
pce_print_string:
	push	ax
	push	bx
	push	si

	cld

.next:
	mov	ah, 0x0e
	lodsb
	or	al, al
	jz	.done
	mov	bx, 0x0007
	int	0x10
	jmp	.next

.done:
	pop	si
	pop	bx
	pop	ax
	ret
%endif


%ifdef PCE_USE_PRINT_UINT16
;-----------------------------------------------------------------------------
; Print an unsigned integer
;
; inp: AX = value
;      BX = base
;-----------------------------------------------------------------------------
pce_print_uint16:
	push	ax
	push	cx
	push	dx
	push	bx

	xor	cx, cx

.next1:
	xor	dx, dx
	div	bx

	cmp	dl, 10
	jb	.isdigit
	add	dl, 'A' - 10 - '0';
.isdigit:
	add	dl, '0'

	push	dx
	inc	cx

	or	ax, ax
	jnz	.next1

.next2:
	pop	ax
	mov	ah, 0x0e
	mov	bx, 0x0007
	int	0x10
	loop	.next2

	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
%endif


%ifdef PCE_USE_PARSE_OPT
;-----------------------------------------------------------------------------
; Parse an option string
;
; inp: DS:SI = command line
;      CX      command line length
; out: ES:DI = option string
;      CX    = remaining command line length
;      CF    = 1 if there are no more options
;-----------------------------------------------------------------------------
pce_parse_opt:
	push	ax
	push	dx

	cld
	mov	ah, 1

.skip:
	jcxz	.done
	lodsb
	dec	cx
	cmp	al, 32
	jbe	.skip

	xor	ah, ah

	cmp	al, '"'
	je	.quote

.next:
	stosb
	jcxz	.done
	lodsb
	dec	cx
	cmp	al, 32
	jbe	.done
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
	stosb				; terminate the string

	shr	ah, 1			; set carry flag

	pop	dx
	pop	ax
	ret
%endif


%ifdef PCE_USE_HOOK_CHECK
;-----------------------------------------------------------------------------
; Check if we are running under PCE
;
; out: AX = 0000 if PCE
;      CF = 0 if PCE
;-----------------------------------------------------------------------------
pce_hook_check:
	xor	ax, ax
	db	0xd5, 0x00
	sub	ax, 0x0fce
	jnz	.notpce
	ret
.notpce:
	mov	ax, 0xffff
	stc
	ret
%endif


%ifdef PCE_USE_HOOK
;-----------------------------------------------------------------------------
; pce_hook
;-----------------------------------------------------------------------------
pce_hook:
	clc
	db	0xd5, 0x00
	ret
%endif


%ifdef PCE_USE_HOOK_STOP
;-----------------------------------------------------------------------------
; Stop the emulator
;-----------------------------------------------------------------------------
pce_hook_stop:
	push	ax
	mov	ax, PCE_HOOK_STOP
	db	0xd5, 0x00
	pop	ax
	ret
%endif


%endif
