;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   pcehimem.asm                                                 *
;* Created:     2003-09-02 by Hampa Hug <hampa@hampa.ch>                     *
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


%include "pce.inc"


section text

	dd	0xffffffff		; link to next driver
	dw	0x8000			; flags, plain character device
	dw	drv_strategy		; strategy offset
	dw	drv_interrupt		; interrupt offset
	db	"XMSXXXX0"		; driver name

reqadr:
	dd	0

saveint2f:
	dd	0


;-----------------------------------------------------------------------------
; int 2f handler
;-----------------------------------------------------------------------------
int_2f:
	cmp	ax, 0x4300		; xms installation check
	jne	.not4300

	mov	al, 0x80
	iret

.not4300:
	cmp	ax, 0x4310		; get xms driver entry point
	jne	.done

	mov	bx, cs
	mov	es, bx
	mov	bx, xms_handler
	iret

.done:
	jmp	far [cs:saveint2f]


;-----------------------------------------------------------------------------
; The XMS handler
;-----------------------------------------------------------------------------
xms_handler:
	jmp	short .handler
	nop
	nop
	nop

.handler:
	pceh	PCEH_XMS		; PCE hook
	retf


;-----------------------------------------------------------------------------
; driver strategy function
;-----------------------------------------------------------------------------
drv_strategy:
	mov	[cs:reqadr], bx
	mov	[cs:reqadr + 2], es
	retf


;-----------------------------------------------------------------------------
; driver interrupt function
;-----------------------------------------------------------------------------
drv_interrupt:
	pushf
	push	ax
	push	cx
	push	dx
	push	bx
	push	ds

	lds	bx, [cs:reqadr]		; request address

	mov	ah, [bx + 2]		; command

	or	ah, ah			; driver init
	jne	.not00

	call	drv_init
	jmp	.done

.not00:
	; unknown command
	mov	word [bx + 3], 0x8103

.done:
	pop	ds
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	popf
	retf


drv_end:


;-----------------------------------------------------------------------------
; func 00: init driver
;-----------------------------------------------------------------------------
drv_init:
	push	si

	mov	si, msg_init
	call	prt_string

	pceh	PCEH_XMS_INFO		; get xms info

	test	al, 0x01		; check if xms installed
	jnz	.xmsok

	mov	si, msg_noxms
	call	prt_string

	jmp	.doneerr

.xmsok:
	mov	si, msg_himem
	call	prt_string

	xchg	ax, dx			; XMS size in K
	call	prt_uint16

	mov	si, msg_xmsplus
	call	prt_string

	mov	ax, cx			; UMB size in paragraphs
	mov	cl, 6
	shr	ax, cl
	call	prt_uint16

	mov	si, msg_umbplus
	call	prt_string

	xor	ax, ax
	test	dl, 0x02		; check if HMA available
	jz	.nohma
	mov	ax, 64			; HMA is always 64K

.nohma:
	call	prt_uint16

	mov	si, msg_avail
	call	prt_string

	push	ds
	xor	ax, ax
	mov	ds, ax

	mov	ax, [4 * 0x002f]	; int 2f address
	mov	[cs:saveint2f], ax
	mov	ax, [4 * 0x002f + 2]
	mov	[cs:saveint2f + 2], ax

	mov	word [4 * 0x002f + 0], int_2f
	mov	word [4 * 0x002f + 2], cs
	pop	ds

	jmp	.doneok

.doneerr:
	mov	word [bx + 14], 0x0000	; end address
	mov	word [bx + 16], cs

	mov	word [bx + 3], 0x8100	; status
	jmp	.done

.doneok:
	mov	word [bx + 14], drv_end	; end address
	mov	word [bx + 16], cs

	mov	word [bx + 3], 0x0100	; status

.done:
	mov	word [bx + 18], 0	; param address
	mov	word [bx + 20], 0

	mov	word [bx + 23], 0x0000	; message flag

	mov	si, msg_nl
	call	prt_string

	pop	si
	ret


;-----------------------------------------------------------------------------
; print the string at CS:SI
;-----------------------------------------------------------------------------
prt_string:
	push	ax
	push	bx
	push	si

.next:
	mov	ah, 0x0e
	mov	bx, 0x0007
	mov	al, [cs:si]
	or	al, al
	jz	.done
	int	0x10
	inc	si
	jmp	.next

.done:
	pop	si
	pop	bx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; print the 16 bit unsigned integer in AX
;-----------------------------------------------------------------------------
prt_uint16:
	push	ax
	push	cx
	push	dx
	push	bx

	xor	cx, cx
	mov	bx, 10

.next1:
	xor	dx, dx
	div	bx
	push	dx
	inc	cx
	or	ax, ax
	jnz	.next1

.next2:
	pop	ax
	mov	ah, 0x0e
	add	al, '0'
	mov	bx, 0x0007
	int	0x10
	loop	.next2

	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


msg_nl:
	db	0x0d, 0x0a, 0x00

msg_init:
	db	"XMS: PCE XMS driver version ", PCE_VERSION_STR
	db	0x0d, 0x0a, 0x00

msg_noxms:
	db	"XMS: No XMS available", 0x0d, 0x0a, 0x00

msg_himem:
	db	"XMS: ", 0x00

msg_xmsplus:
	db	"K XMS + ", 0x00

msg_umbplus:
	db	"K UMB + ", 0x00

msg_avail:
	db	"K HMA available", 0x0d, 0x0a, 0x00
