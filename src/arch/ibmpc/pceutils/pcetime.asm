;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    pcetime.asm
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


%include "pce.inc"


%define OPT_H 1
%define OPT_V 2
%define OPT_P 4
%define OPT_S 8


section .text

	org	0x100

	jmp	start

msg_version	db "pcetime version ", PCE_VERSION_STR, 0x0d, 0x0a, 0x00
msg_usage	db "pcetime: get time from the PCE host", 0x0d, 0x0a, 0x0d, 0x0a
		db "usage: pcetime [opts]", 0x0d, 0x0a
		db "  -h  Print this help", 0x0d, 0x0a
		db "  -p  Get host time and print it", 0x0d, 0x0a
		db "  -s  Get host time and set DOS system time", 0x0d, 0x0a
		db "  -v  Print version information", 0x0d, 0x0a
		db 0x00

%define PCE_USE_PRINT_STRING 1
%define PCE_USE_PRINT_CHAR   1
%define PCE_USE_HOOK_CHECK   1
%define PCE_USE_HOOK         1

%include "pce-lib.inc"


;-----------------------------------------------------------------------------
; void print_uint16 (unsigned val, unsigned base, unsigned min)
;-----------------------------------------------------------------------------
print_uint16:
	push	bp
	mov	bp, sp
	sub	sp, 256
	push	si

	lea	si, [bp - 256]
	xor	cx, cx
	mov	bx, [bp + 6]		; base
	mov	ax, [bp + 4]		; val

.next1:
	xor	dx, dx
	div	bx

	cmp	dl, 10
	jb	.isdigit
	add	dl, 'A' - 10 - '0';
.isdigit:
	add	dl, '0'

	mov	[si], dl
	inc	si
	inc	cx

	cmp	cx, [bp + 8]		; min
	jb	.next1

	or	ax, ax
	jnz	.next1

.next2:
	dec	si
	mov	al, [si]
	mov	ah, 0x0e
	mov	bx, 0x0007
	int	0x10
	loop	.next2

	pop	si
	mov	sp, bp
	pop	bp
	ret


;-----------------------------------------------------------------------------
; print_dec_2
;-----------------------------------------------------------------------------
print_dec_2:
	mov	dx, 2
	xor	ah, ah
.print:
	push	dx
	mov	dx, 10
	push	dx
	push	ax
	call	print_uint16
	add	sp, 6
	ret

;-----------------------------------------------------------------------------
; print_dec_4
;-----------------------------------------------------------------------------
print_dec_4:
	mov	dx, 4
	jmp	print_dec_2.print


;-----------------------------------------------------------------------------
; void dos_set_time (const void *buf)
;-----------------------------------------------------------------------------
dos_set_time:
	push	bp
	mov	bp, sp
	push	ds

	lds	bx, [bp + 4]

	mov	dx, [bx + 2]		; month / day
	and	dx, 0x7f7f
	add	dx, 0x0101
	mov	cx, [bx]		; year
	mov	ah, 0x2b
	int	0x21			; set date

	mov	cx, [bx + 4]		; hour / minute
	mov	dx, [bx + 6]		; second / centisecond
	mov	ah, 0x2d
	int	0x21			; set time

	pop	ds
	pop	bp
	ret


;-----------------------------------------------------------------------------
; void print_time (const void *buf)
;-----------------------------------------------------------------------------
print_time:
	push	bp
	mov	bp, sp
	push	si

	lds	si, [bp + 4]

	mov	ax, [si]		; year
	call	print_dec_4

	mov	al, '-'
	call	pce_print_char

	mov	al, [si + 3]		; month
	inc	ax
	call	print_dec_2

	mov	al, '-'
	call	pce_print_char

	mov	al, [si + 2]		; day
	inc	ax
	call	print_dec_2

	mov	al, ' '
	call	pce_print_char

	mov	al, [si + 5]		; hour
	call	print_dec_2

	mov	al, ':'
	call	pce_print_char

	mov	al, [si + 4]		; minute
	call	print_dec_2

	mov	al, ':'
	call	pce_print_char

	mov	al, [si + 7]		; second
	call	print_dec_2

	mov	al, '.'
	call	pce_print_char

	mov	al, [si + 6]		; centisecond
	call	print_dec_2

	mov	si, msg_nl
	call	pce_print_string

	pop	si
	pop	bp
	ret


;-----------------------------------------------------------------------------
; cmd_parse
;-----------------------------------------------------------------------------
cmd_parse:
	push	si

	xor	ah, ah
	lodsb				; byte count
	mov	cx, ax
	xor	dx, dx

.next:
	jcxz	.done
	lodsb
	dec	cx

	cmp	al, ' '
	je	.space
	cmp	al, 0x09
	je	.space
	or	dh, dh
	jnz	.opt
	cmp	al, '-'
	je	.hyphen
	cmp	al, '/'
	je	.hyphen

	jmp	.err

.space:
	xor	dh, dh
	jmp	.next

.hyphen:
	mov	dh, 1
	jmp	.next

.opt:
	cmp	al, 'h'
	je	.opt_h
	cmp	al, 'H'
	je	.opt_h
	cmp	al, 'p'
	je	.opt_p
	cmp	al, 'P'
	je	.opt_p
	cmp	al, 's'
	je	.opt_s
	cmp	al, 'S'
	je	.opt_s
	cmp	al, 'v'
	je	.opt_v
	cmp	al, 'V'
	je	.opt_v
	jmp	.err

.opt_h:
	or	dl, OPT_H
	jmp	.next

.opt_p:
	or	dl, OPT_P
	jmp	.next

.opt_s:
	or	dl, OPT_S
	jmp	.next

.opt_v:
	or	dl, OPT_V
	jmp	.next

.err:
	xor	ax, ax
	stc
	jmp	.ret

.done:
	mov	al, dl
	xor	ah, ah
	clc

.ret:
	pop	si
	ret


;-----------------------------------------------------------------------------
start:
	mov	ax, cs
	mov	ds, ax
	cld

	mov	si, 0x0080
	call	cmd_parse
	jc	.err_cl

	or	al, al
	jnz	.opts_ok
	mov	al, OPT_H
.opts_ok:
	mov	[opts], al

	test	byte [opts], OPT_H
	jz	.no_help
	mov	si, msg_usage
	call	pce_print_string
.no_help:

	test	byte [opts], OPT_V
	jz	.no_version
	mov	si, msg_version
	call	pce_print_string
.no_version:

	mov	si, buffer

	test	byte [opts], OPT_S | OPT_P
	jz	.no_get

	call	pce_hook_check
	jc	.err_notpce

	mov	ax, PCE_HOOK_GET_TIME_LOCAL
	call	pce_hook
	jc	.err_get

	mov	[si], ax
	mov	[si + 2], bx
	mov	[si + 4], cx
	mov	[si + 6], dx

.no_get:
	test	byte [opts], OPT_S
	jz	.no_set
	push	ds
	push	si
	call	dos_set_time
	add	sp, 4

.no_set:
	test	byte [opts], OPT_P
	jz	.no_print
	push	ds
	push	si
	call	print_time
	add	sp, 4

.no_print:
	xor	al, al
	jmp	.exit

.err_notpce:
	mov	si, msg_notpce
	jmp	.done_err

.err_cl:
	mov	si, msg_error_cl
	jmp	.done_err

.err_get:
	mov	si, msg_error_get
	;jmp	.done_err

.done_err:
	push	si
	mov	si, msg_arg0
	call	pce_print_string
	pop	si
	call	pce_print_string
	mov	al, 0x01
	;jmp	.exit

.exit:
	mov	ah, 0x4c
	int	0x21
	int	0x20


;-----------------------------------------------------------------------------
section	.data

msg_arg0	db "pcetime: ", 0
msg_notpce	db "not running under PCE", 0x0d, 0x0a, 0x00
msg_nl          db 0x0d, 0x0a, 0x00
msg_error_cl	db "bad command line", 0x0d, 0x0a, 0x00
msg_error_get	db "error reading host time", 0x0d, 0x0a, 0x00


;-----------------------------------------------------------------------------
section .bss

opts		resb	1
buffer		resb	8
