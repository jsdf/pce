;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   pceemm.asm                                                   *
;* Created:     2003-10-18 by Hampa Hug <hampa@hampa.ch>                     *
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
	db	"EMMXXXX0"		; driver name

reqadr:
	dd	0

saveint67:
	dd	0


;-----------------------------------------------------------------------------
; int 67 handler
;-----------------------------------------------------------------------------
int_67:
	pceh	PCEH_EMS		; PCE hook
	iret


;-----------------------------------------------------------------------------
; strategy function
;-----------------------------------------------------------------------------
drv_strategy:
	mov	[cs:reqadr], bx
	mov	[cs:reqadr + 2], es
	retf


;-----------------------------------------------------------------------------
; interrupt function
;-----------------------------------------------------------------------------
drv_interrupt:
	pushf
	push	ax
	push	cx
	push	dx
	push	bx
	push	ds

	lds	bx, [cs:reqadr]

	mov	ah, [bx + 2]

	or	ah, ah			; driver init
	jne	.not00

	call	drv_init
	jmp	.done

.not00:
	cmp	ah, 0x0a		; get output status
	jne	.not0a

	mov	word [bx + 3], 0x0100
	jmp	.done

.not0a:
	cmp	ah, 0x0b		; flush output buffers
	jne	.not0b

	mov	word [bx + 3], 0x0100
	jmp	.done

.not0b:
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

	mov	si, msg_emm
	call	prt_string

	pceh	PCEH_EMS_INFO		; xms info

	test	al, 0x01		; check if ems installed
	jnz	.emsok

	mov	si, msg_noems
	call	prt_string

	jmp	.doneerr

.emsok:
	mov	ax, dx			; EMS size in K
	call	prt_uint16

	mov	si, msg_avail
	call	prt_string

	push	ds
	xor	ax, ax
	mov	ds, ax

	mov	dx, [4 * 0x0067 + 0]	; int 67 address
	mov	ax, [4 * 0x0067 + 2]
	mov	[cs:saveint67], dx
	mov	[cs:saveint67 + 2], ax

	mov	word [4 * 0x0067 + 0], int_67
	mov	word [4 * 0x0067 + 2], cs
	pop	ds

	jmp	.doneok

.doneerr:
	mov	word [bx + 14], 0x0000	; end address
	mov	word [bx + 16], cs

	mov	word [bx + 3], 0x8100	; driver status
	jmp	.done

.doneok:
	mov	word [bx + 14], drv_end	; end address
	mov     word [bx + 16], cs

	mov	word [bx + 3], 0x0100	; driver status

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


msg_init:
	db	"EMS: PCE EMS driver version ", PCE_VERSION_STR
	db	0x0d, 0x0a, 0x00

msg_emm:
	db	"EMS: ", 0x00

msg_avail:
	db	"K available", 0x0d, 0x0a, 0x00

msg_noems:
	db	"No EMS available", 0x0d, 0x0a, 0x00

msg_nl:
	db	0x0d, 0x0a, 0x00
