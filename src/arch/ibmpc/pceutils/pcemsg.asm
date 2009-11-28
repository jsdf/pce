;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   pcemsg.asm                                                   *
;* Created:     2004-09-17 by Hampa Hug <hampa@hampa.ch>                     *
;* Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


; pcemsg [msg [val]]
; send a message to the emulation core


%include "pce.inc"


section .text

	org	0x100

	jmp	start


msg_error	db "error", 0x0d, 0x0a, 0x00


; print string ds:si
prt_string:
	push	ax
	push	si

.next:
	mov	ah, 0x0e
	lodsb
	or	al, al
	jz	.done
	int	0x10
	jmp	.next

.done:
	pop	si
	pop	ax
	ret


; get message string from ds:si to es:di
get_msg_str:
	jcxz	.done

.skip:
	cmp	byte [si], 32
	ja	.start
	inc	si
	loop	.skip

	jmp	.done

.start:
	cmp	byte [si], '"'
	je	.quote

.next:
	cmp	byte [si], 32
	jbe	.done
	movsb
	loop	.next

	jmp	.done

.quote:
	inc	si
	dec	cx

.next_quote:
	jcxz	.done
	lodsb
	dec	cx
	cmp	al, '"'
	je	.done
	stosb
	jmp	.next_quote

	stosb
	loop	.next_quote

.done:
	mov	al, 0x00
	stosb

	ret


start:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax

	mov	si, 0x0080		; SI points to parameters

	lodsb
	mov	ah, 0
	mov	cx, ax			; parameter size in CX

	mov	di, buffer

	push	di
	call	get_msg_str

	push	di
	call	get_msg_str

	pop	di
	pop	si
	pceh	PCEH_MSG
	jc	error

	mov	al, 0x00
	jmp	done

error:
	mov	si, msg_error
	call	prt_string

	mov	al, 0x01

done:
	mov	ah, 0x4c
	int	0x21

buffer:
