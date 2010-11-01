;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   src/arch/ibmpc/bios/pcex.asm                                 *
;* Created:     2003-04-14 by Hampa Hug <hampa@hampa.ch>                     *
;* Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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


section .text


;-----------------------------------------------------------------------------

init:
	jmp	start

	set_pos	4
pce_ext:
	db	"PCEX"				; marker
	dw	pce_ext_end - pce_ext
	dw	0

	set_pos	12
	jmp	start

	set_pos	16
	jmp	int_19
pce_ext_end:


msg_init	db "PCE IBM PC BIOS extension"
		db 13, 10, 13, 10, 0

msg_memchk1	db "Memory size:    ", 0
msg_memchk2	db "KB", 13, 0

msg_serial	db "Serial ports:   ", 0
msg_parallel	db "Parallel ports: ", 0

msg_rom1	db "ROM[", 0
msg_rom2	db "]:", 0
msg_cksmok	db " checksum ok", 0
msg_cksmbad	db " bad checksum", 0


start:
	cli
	mov	ax, 0x0050
	mov	ss, ax
	mov	sp, 1024

	call	set_bios_ds

	call	init_data
	call	init_int
	call	init_pic
	call	init_pit
	call	init_ppi
	call	init_dma

	sti

	call	init_video
	call	init_rom1
	call	init_banner
	call	init_mem
	call	init_misc
	call	init_keyboard
	call	init_serport
	call	init_parport
	call	init_time
	call	init_biosdata
	call	init_rom2

	call	prt_nl

	push	cs
	pop	ds

	cli

	mov	di, 0x0050
	mov	es, di
	xor	di, di
	xor	ax, ax
	mov	cx, 0x8000 - 0x0500
	rep	stosw

	mov	ax, 0x0030
	mov	ss, ax
	mov	sp, 0x0100

	sti

	int	0x19

done:
	jmp	done


;-----------------------------------------------------------------------------

init_data:
	push	ax
	push	cx
	push	di
	push	es

	cld

	push	ds
	pop	es

	xor	di, di
	xor	ax, ax
	mov	cx, 256 / 2
	rep	stosw

	pop	es
	pop	di
	pop	cx
	pop	ax
	ret


init_biosdata:
	push	ax
	push	ds

	mov	ax, 0x0040
	mov	ds, ax

	pceh	PCEH_GET_HDCNT
	mov	[0x0075], al

	; keyboard status 1
	; (non-functional on PC, but some programs need it)
	;mov     [0x0096], byte 0x10

	pop	ds
	pop	ax
	ret


init_int:
	push	ax
	push	cx
	push	si
	push	di
	push	es
	push	ds

	cld

	mov	ax, cs
	mov	ds, ax
	mov	si, inttab
	xor	di, di
	mov	es, di
	mov	cx, 32

.next:
	movsw					; offset
	lodsw

	or	ax, ax
	jnz	.seg_ok
	mov	ax, cs

.seg_ok:
	stosw
	loop	.next


%ifdef INIT_ALL_INTERRUPTS
	mov	cx, 256 - 32

.next:
	mov	ax, int_default
	stosw
	mov	ax, cs
	stosw
	loop	.next
%else
	xor	ax, ax
	mov	cx, 2 * (256 - 32)
	rep	stosw				; set interrupts 21 to ff to 0000:0000
%endif

						; set int 0x40 == int 0x13
	mov	ax, [es:4 * 0x13 + 0]
	mov	cx, [es:4 * 0x13 + 2]
	mov	[es:4 * 0x40 + 0], ax
	mov	[es:4 * 0x40 + 2], cx

	pop	ds
	pop	es
	pop	di
	pop	si
	pop	cx
	pop	ax
	ret


init_pic:
	mov	al, 0x13
	out	0x20, al

	mov	al, 0x08
	out	0x21, al

	mov	al, 0x01
	out	0x21, al

	mov	al, 0x00
	out	0x21, al
	ret


init_pit:
	mov	al, 0x36			; channel 0 mode 3
	out	0x43, al
	mov	al, 0
	out	0x40, al
	out	0x40, al

	mov	al, 0x54			; channel 1 mode 2
	out	0x43, al
	mov	al, 0x12
	out	0x41, al

	mov	al, 0xb6			; channel 2 mode 3
	out	0x43, al
	mov	al, 0x00
	out	0x42, al
	out	0x42, al
	ret


;-----------------------------------------------------------------------------
; Initialize the video 8255 PPI
;-----------------------------------------------------------------------------
init_ppi:
	push	ax
	push	cx

	mov	al, 0x99
	out	0x63, al			; set up ppi ports

	cmp	byte [cs:0xfffe], 0xfe		; check if pc/xt
	je	.xt

	mov	al, 0xfc
	out	0x61, al

	in	al, 0x60			; get sw1
	xor	ah, ah

	mov	[0x0010], ax			; equipment word

	mov	al, 0x7c
	out	0x61, al
	jmp	.done

.xt:
	mov	al, 0xfc
	out	0x61, al

	in	al, 0x62			; get sw high
	mov	ah, al

	mov	al, 0x74
	out	0x61, al

	in	al, 0x62			; get sw low
	and	al, 0x0f

	mov	cl, 4
	shl	ah, cl
	or	al, ah
	xor	ah, ah

	mov	[0x0010], ax

.done:
	pop	cx
	pop	ax
	ret


init_dma:
	push	ax

	mov	al, 0
	out	0x0d, al			; master clear

	out	0x0c, al			; clear flip flop

	out	0x00, al			; address
	out	0x00, al

	mov	al, 0xff
	out	0x01, al			; count
	out	0x01, al

	mov	al, 0x58			; mode channel 0
	out	0x0b, al

	mov	al, 0x41
	out	0x0b, al

	inc	ax
	out	0x0b, al

	inc	ax
	out	0x0b, al

	pop	ax
	ret


;-----------------------------------------------------------------------------
; Initialize the video mode
;-----------------------------------------------------------------------------
init_video:
	push	ax
	push	si

	mov	ax, [0x0010]
	and	al, 0x30

	cmp	al, 0x30
	je	.mda

	cmp	al, 0x20
	je	.cga

	jmp	.done

.cga:
	mov	ax, 0x0003
	int	0x10
	jmp	.done

.mda:
	mov	ax, 0x0007
	int	0x10
	jmp	.done

.done:
	pop	si
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Print the startup message
;-----------------------------------------------------------------------------
init_banner:
	push	ax
	push	si

	mov	si, msg_init
	call	prt_string

	pop	si
	pop	ax
	ret


init_misc:
	xor	ax, ax

	mov	[0x006c], ax
	mov	[0x006e], ax
	mov	[0x0070], al

	mov	[0x0000], ax			; COM1
	mov	[0x0002], ax
	mov	[0x0004], ax
	mov	[0x0006], ax
	mov	[0x0008], ax			; LPT1
	mov	[0x000a], ax
	mov	[0x000c], ax
	mov	[0x000e], ax

	ret


; check rom checksum at ES:0000
; returns ah=00 if ok
check_rom:
	push	cx
	push	bx

	mov	ch, [es:0x0002]
	shl	ch, 1
	mov	cl, 0

	xor	bx, bx
	mov	ah, 0

.cksum:
	add	ah, [es:bx]
	inc	bx
	loop	.cksum

	pop	bx
	pop	cx
	ret


; start rom at ES:0000
start_rom:
	push	ax
	push	cx
	push	dx
	push	bx
	push	bp
	push	si
	push	di
	push	es
	push	ds

	push	cs
	mov	ax, .romret
	push	ax
	push	es
	mov	ax, 3
	push	ax
	retf

.romret:
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	bp
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


; initialize rom extensions in the range [BX:0000..DX:0000]
init_rom_range:
	push	ax
	push	bx
	push	es

.next:
	mov	es, bx
	cmp	word [es:0x0000], 0xaa55	; rom id
	jne	.norom

	call	check_rom			; calculate checksum
	or	ah, ah
	jnz	.norom

	mov	ah, [es:0x0002]			; rom size / 512
	mov	al, 0
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1				; rom size in paragraphs
	add	ax, 0x007f
	and	ax, 0xff80			; round up to 2K
	jz	.norom

	add	bx, ax

	call	start_rom
	jmp	.skiprom

.norom:
	add	bx, 0x0080			; 2KB

.skiprom:
	cmp	bx, dx
	jb	.next

	pop	es
	pop	bx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Initialize rom extensions in the range [C000..C7FF]
;-----------------------------------------------------------------------------
init_rom1:
	push	dx
	push	bx

	mov	bx, 0xc000
	mov	dx, 0xc800
	call	init_rom_range

	pop	bx
	pop	dx
	ret


;-----------------------------------------------------------------------------
; Initialize rom extensions in the range [C800..DFFF]
;-----------------------------------------------------------------------------
init_rom2:
	push	dx
	push	bx

	mov	bx, 0xc800
	mov	dx, 0xe000
	call	init_rom_range

	pop	bx
	pop	dx
	ret


init_mem:
	push	ax
	push	cx
	push	bx
	push	si
	push	es

	mov	cx, 64
	mov	bx, 0x1000

.next:
	mov	es, bx

	mov	ax, 0xaa55

	xchg	[es:0], al
	xchg	[es:0], al
	xchg	[es:0], ah
	xchg	[es:0], ah

	cmp	ax, 0xaa55
	jne	.done

	xchg	[es:0 + 1023], al
	xchg	[es:0 + 1023], al
	xchg	[es:0 + 1023], ah
	xchg	[es:0 + 1023], ah

	cmp	ax, 0xaa55
	jne	.done

	inc	cx
	add	bx, 1024 / 16

	cmp	cx, 640
	jae	.done

	jmp	.next

.done:
	mov	si, msg_memchk1
	call	prt_string

	mov	ax, cx
	call	prt_uint16

	mov	si, msg_memchk2
	call	prt_string

	call	prt_nl

	mov	[0x0013], cx

	pop	es
	pop	si
	pop	bx
	pop	cx
	pop	ax
	ret


init_keyboard:
	mov	[0x0080], word 0x001e		; keyboard buffer start
	mov	[0x0082], word 0x003e		; keyboard buffer end

	mov	[0x001a], word 0x001e
	mov	[0x001c], word 0x001e

	mov	[0x0017], byte 0x00		; keyboard status 1
	mov	[0x0018], byte 0x00		; keyboard status 2

	ret


init_serport:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si

						; get com info
	pceh	PCEH_GET_COM

	mov	[0x0000], ax
	mov	[0x0002], bx
	mov	[0x0004], cx
	mov	[0x0006], dx

	sub	ax, 1
	cmc
	mov	ax, 0
	adc	ax, 0

	sub	bx, 1
	cmc
	adc	ax, 0

	sub	cx, 1
	cmc
	adc	ax, 0

	sub	dx, 1
	cmc
	adc	ax, 0

	mov	si, msg_serial
	call	prt_string
	call	prt_uint16

	mov	cl, 9
	shl	ax, cl

	mov	dx, [0x0010]
	and	dx, ~0x0e00
	or	dx, ax
	mov	[0x0010], dx

	call	prt_nl

	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


init_parport:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si

						; get lpt info
	pceh	PCEH_GET_LPT

	mov	[0x0008], ax
	mov	[0x000a], bx
	mov	[0x000c], cx
	mov	[0x000e], dx

	sub	ax, 1
	cmc
	mov	ax, 0
	adc	ax, 0

	sub	bx, 1
	cmc
	adc	ax, 0

	sub	cx, 1
	cmc
	adc	ax, 0

	sub	dx, 1
	cmc
	adc	ax, 0

	mov	si, msg_parallel
	call	prt_string
	call	prt_uint16

	mov	cl, 14
	shl	ax, cl

	and	byte [0x0011], 0x3f
	or	word [0x0010], ax

	call	prt_nl

	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


get_bcd:
	push	dx
	xor	dx, dx
	mov	ah, al
	and	ax, 0xf00f
	shr	ah, 1
	add	dl, ah
	shr	ah, 1
	shr	ah, 1
	add	dl, ah
	add	dl, al
	xchg	ax, dx
	pop	dx
	ret

init_time:
	push	ax
	push	dx
	push	si
	push	di

	mov	ah, 2
	int	0x1a

						; 18 * seconds
	mov	al, dh
	call	get_bcd
	mov	dx, 18
	mul	dx
	mov	si, ax
	mov	di, dx

						; 1092 * minutes
	mov	al, cl
	call	get_bcd
	mov	dx, 1092
	mul	dx
	add	si, ax
	adc	di, dx

						; 65539 * hours
	mov	al, ch
	call	get_bcd
	add	di, ax
	mov	dx, ax
	shl	ax, 1
	add	ax, dx
	add	si, ax
	adc	di, 0

	mov	[0x006c], si
	mov	[0x006e], di
	mov	[0x0070], byte 0

	pop	di
	pop	si
	pop	dx
	pop	ax
	ret

set_bios_ds:
	mov	ds, [cs:.bios_ds]
	ret

.bios_ds:
	dw	0x0040


; print string at CS:SI
prt_string:
	push	ax
	push	bx
	push	si

	xor	bx, bx

.next:
	cs	lodsb
	or	al, al
	jz	.done

	mov	ah, 0x0e
	int	0x10

	jmp	short .next

.done:
	pop	si
	pop	bx
	pop	ax
	ret


; print newline
prt_nl:
	push	ax
	push	bx

	mov	ax, 0x0e0d
	xor	bx, bx
	int	0x10

	mov	ax, 0x0e0a
	int	0x10

	pop	bx
	pop	ax
	ret


; print a 16 bit unsigned integer in ax
prt_uint16:
	push	ax
	push	cx
	push	dx
	push	bx

	mov	bx, 10
	xor	cx, cx

.next1:
	xor	dx, dx

	div	bx
	add	dl, '0'
	push	dx
	inc	cx

	or	ax, ax
	jnz	.next1

.next2:
	pop	ax
	mov	ah, 0x0e
	xor	bx, bx
	int	0x10
	loop	.next2

	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------

inttab:
	dw	int_00, 0x0000			; 00: F000:FF47
	dw	int_01, 0x0000			; 01: F000:FF47
	dw	0xe2c3, 0xf000			; 02: F000:E2C3
	dw	int_03, 0x0000			; 03: F000:FF47
	dw	int_04, 0x0000			; 04: F000:FF47
	dw	0xff54, 0xf000			; 05: F000:FF54
	dw	int_06, 0x0000			; 06: F000:FF47
	dw	int_07, 0x0000			; 07: F000:FF47
	dw	0xfea5, 0xf000			; 08: F000:FEA5
	dw	0xe987, 0xf000			; 09: F000:E987
	dw	0xe6dd, 0xf000			; 0A: F000:E6DD
	dw	0xe6dd, 0xf000			; 0B: F000:E6DD
	dw	0xe6dd, 0xf000			; 0C: F000:E6DD
	dw	0xe6dd, 0xf000			; 0D: F000:E6DD
	dw	0xef57, 0xf000			; 0E: F000:EF57
	dw	0xe6dd, 0xf000			; 0F: F000:E6DD
	dw	0xf065, 0xf000			; 10: F000:F065
	dw	0xf84d, 0xf000			; 11: F000:F84D
	dw	0xf841, 0xf000			; 12: F000:F841
	dw	int_13, 0x0000			; 13: F000:EC59
	dw	0xe739, 0xf000			; 14: F000:E739
	dw	int_15, 0x0000			; 15: F000:F859
	dw	0xe82e, 0xf000			; 16: F000:E82E
	dw	0xefd2, 0xf000			; 17: F000:EFD2
	dw	0x0000, 0xf600			; 18: F600:0000
	dw	int_19, 0x0000			; 19: F000:E6F2
	dw	int_1a, 0x0000			; 1A: F000:FE6E
	dw	int_1b, 0x0000			; 1B: F000:FF53
	dw	0xff53, 0xf000			; 1C: F000:FF53
	dw	0xf0a4, 0xf000			; 1D: F000:F0A4
	dw	0xefc7, 0xf000			; 1E: F000:EFC7
	dw	int_1f, 0x0000			; 1F: F000:0000


int_default:
	iret


;-----------------------------------------------------------------------------

int_00:
	pceh	PCEH_INT, 0x00
	iret

int_01:
	pceh	PCEH_INT, 0x01
	iret

int_02:
	pceh	PCEH_INT, 0x02
	iret

int_03:
	pceh	PCEH_INT, 0x03
	iret

int_04:
	pceh	PCEH_INT, 0x04
	iret

int_05:
	pceh	PCEH_INT, 0x05
	iret

int_06:
	pceh	PCEH_INT, 0x06
	iret

int_07:
	pceh	PCEH_INT, 0x07
	iret

int_09:
	pceh	PCEH_INT, 0x09
	iret

int_0a:
	pceh	PCEH_INT, 0x0a
	iret

int_0b:
	pceh	PCEH_INT, 0x0b
	iret

int_0c:
	pceh	PCEH_INT, 0x0c
	iret

int_0d:
	pceh	PCEH_INT, 0x0d
	iret

int_0e:
	pceh	PCEH_INT, 0x0e
	iret

int_0f:
	pceh	PCEH_INT, 0x0f
	iret

int_10:
	pceh	PCEH_INT, 0x10
	iret

int_11:
	pceh	PCEH_INT, 0x11
	iret

int_12:
	pceh	PCEH_INT, 0x12
	iret

int_13:
	push	ax
	pceh	PCEH_CHECK_INT, 0x13
	or	ax, ax
	jz	.skip
	pop	ax

	sti
	pceh	PCEH_INT, 0x13
	retf	2

.skip:
	pop	ax
	jmp	0xf000:0xec59

int_14:
	pceh	PCEH_INT, 0x14
	iret

int_15:
	cmp	ah, 3
	jbe	.cassette
	sti
	pceh	PCEH_INT, 0x15
	retf	2
.cassette:
	jmp	0xf000:0xf859			; bios int 15

int_17:
	pceh	PCEH_INT, 0x17
	iret

int_1b:
	pceh	PCEH_INT, 0x1b
	iret

int_1d:
	pceh	PCEH_INT, 0x1d
	iret

int_1e:
	pceh	PCEH_INT, 0x1e
	iret

int_1f:
	pceh	PCEH_INT, 0x1f
	iret


;-----------------------------------------------------------------------------


int_19:
	xor	ax, ax
	xor	dx, dx
	int	0x13

	xor	ax, ax
	mov	ds, ax
	mov	es, ax

	mov	word [4 * 0x13 + 0], int_13
	mov	word [4 * 0x13 + 2], cs

	mov	word [4 * 0x1a + 0], int_1a
	mov	word [4 * 0x1a + 2], cs

	pceh	PCEH_GET_BOOT			; get boot drive in AL
	mov	dl, al

	mov	ax, 0x0201
	mov	bx, 0x7c00
	mov	cx, 0x0001
	mov	dh, 0x00
	int	0x13
	jc	.fail

	;cmp	[bx + 0x1fe], word 0xaa55
	;jne	.fail

	jmp	0x0000:0x7c00

.fail:
	int	0x18

.halt:
	jmp	.halt


;-----------------------------------------------------------------------------

int_1a:
	cmp	ah, 2
	jae	.hook

	jmp	0xf000:0xfe6e

.hook:
	push	bp
	mov	bp, sp
	push	word [bp + 6]			; flags
	popf
	pop	bp

	pceh	PCEH_INT, 0x1a

	retf	2


;-----------------------------------------------------------------------------

%ifndef NOFILL
	set_pos	(0x6000)
%endif
