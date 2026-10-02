;-----------------------------------------------------------------------------
; pce
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; File name:    src/arch/ibmpc/bios/pcex.asm
; Created:      2003-04-14 by Hampa Hug <hampa@hampa.ch>
; Copyright:    (C) 2003-2022 Hampa Hug <hampa@hampa.ch>
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; This program is free software. You can redistribute it and / or modify it
; under the terms of the GNU General Public License version 2 as  published
; by the Free Software Foundation.
;
; This program is distributed in the hope  that  it  will  be  useful,  but
; WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
; Public License for more details.
;-----------------------------------------------------------------------------


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

msg_memsize	db "Memory size:    ", 0
msg_serial	db "Serial ports:   ", 0
msg_parallel	db "Parallel ports: ", 0

msg_kib		db " KiB", 13, 10, 0


start:
	cli
	mov	ax, 0x0040
	mov	ss, ax
	mov	sp, 4096
	mov	ds, ax

	cld

	pce	PCE_HOOK_CHECK
	sub	ax, 0x0fce		; check if we are running under pce
.stop:
	jne	.stop

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
	mov	ax, 0x0030
	mov	ss, ax
	mov	sp, 0x0100
	sti

	int	0x19

done:
	jmp	done


;-----------------------------------------------------------------------------
; Clear the first 32K of RAM
;-----------------------------------------------------------------------------
init_data:
	pop	bp

	xor	di, di
	mov	es, di
	xor	ax, ax
	mov	cx, 32768 / 2
	rep	stosw			; clear the first 32K of RAM

	jmp	bp


;-----------------------------------------------------------------------------
; Set up some extra BIOS data
;-----------------------------------------------------------------------------
init_biosdata:
	push	ax

	mov	byte [0x0090], 0x94	; drive 0 media state
	mov	byte [0x0091], 0x94	; drive 1 media state

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
; Initialize the 8255 PPI
;-----------------------------------------------------------------------------
init_ppi:
	push	ax
	push	cx
	push	es

	mov	al, 0x99
	out	0x63, al			; set up ppi ports

	pce	PCE_HOOK_GET_MODEL
	jc	.nohook

	test	al, 0x01			; check if pc
	jnz	.pc
	test	al, 0x02			; check if xt
	jnz	.xt

.nohook:
	mov	ax, 0xf000
	mov	es, ax
	mov	al, [es:0xfffe]			; get bios model byte
	cmp	al, 0xfe
	je	.xt
	cmp	al, 0xfb
	je	.xt

.pc:
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

	mov	[0x0010], ax			; equipment word

.done:
	pop	es
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

	mov	al, 0x58
	out	0x0b, al			; mode channel 0

	mov	al, 0x41
	out	0x0b, al			; mode channel 1

	inc	ax
	out	0x0b, al			; mode channel 2

	inc	ax
	out	0x0b, al			; mode channel 3

	xor	al, al
	out	0x0a, al			; unmask channel 0

	pop	ax
	ret


;-----------------------------------------------------------------------------
; Initialize the video mode
;-----------------------------------------------------------------------------
init_video:
	push	ax
	push	si

	mov	ax, [0x0010]			; configuration
	and	ax, 0x30			; initial video mode

	sub	al, 0x10
	jc	.done				; 00
	jz	.cga40				; 01

	sub	al, 0x10
	jz	.cga80				; 02

.mda:
	mov	al, 0x07
	jmp	.set

.cga40:
	mov	al, 0x01
	jmp	.set

.cga80:
	mov	al, 0x03
	;jmp	.set

.set:
	int	0x10				; set video mode

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

	in	al, 0x21
	and	al, 0xfc
	out	0x21, al

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


;-----------------------------------------------------------------------------
; Initialize the RAM size
;-----------------------------------------------------------------------------
init_mem:
	push	ax
	push	cx
	push	bx
	push	si

	mov	cx, 16				; start at 16 KiB
	mov	bx, 0x0400
	mov	ax, 0xaa55
	xor	si, si

	push	ds

.next1:
	mov	ds, bx

.next2:
	xchg	[si], al
	xchg	[si], al
	cmp	al, 0x55
	jne	.done

	xchg	[si], ah
	xchg	[si], ah
	cmp	ah, 0xaa
	jne	.done

	xor	si, 1023
	jnz	.next2

	inc	cx
	add	bx, 1024 / 16

	cmp	cx, 640
	jb	.next1

.done:
	pop	ds

	mov	[0x0013], cx			; ram size

	mov	si, msg_memsize
	call	prt_string
	mov	ax, cx
	call	prt_uint16
	mov	si, msg_kib
	call	prt_string

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


;-----------------------------------------------------------------------------
; Initialize COM port addresses
;-----------------------------------------------------------------------------
init_serport:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si

	pce	PCE_HOOK_GET_COM		; get com ports
	jnc	.ok

	xor	ax, ax
	xor	bx, bx
	xor	cx, cx
	xor	dx, dx

.ok:
	mov	[0x0000], ax
	mov	[0x0002], bx
	mov	[0x0004], cx
	mov	[0x0006], dx

	sub	ax, 1
	mov	ax, 4
	sbb	al, 0
	sub	bx, 1
	sbb	al, 0
	sub	cx, 1
	sbb	al, 0
	sub	dx, 1
	sbb	al, 0

	mov	si, msg_serial
	call	prt_string
	call	prt_uint16
	call	prt_nl

	mov	cl, 9
	shl	ax, cl

	mov	dx, [0x0010]
	and	dx, ~0x0e00
	or	dx, ax
	mov	[0x0010], dx

	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Initialize LPT port addresses
;-----------------------------------------------------------------------------
init_parport:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si

	pce	PCE_HOOK_GET_LPT		; get lpt ports
	jnc	.ok

	xor	ax, ax
	xor	bx, bx
	xor	cx, cx
	xor	dx, dx

.ok:
	mov	[0x0008], ax
	mov	[0x000a], bx
	mov	[0x000c], cx
	mov	[0x000e], dx

	sub	ax, 1
	mov	ax, 4
	sbb	al, 0
	sub	bx, 1
	sbb	al, 0
	sub	cx, 1
	sbb	al, 0
	sub	dx, 1
	sbb	al, 0

	mov	si, msg_parallel
	call	prt_string
	call	prt_uint16
	call	prt_nl

	mov	cl, 14
	shl	ax, cl

	and	byte [0x0011], 0x3f
	or	word [0x0010], ax

	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Initialize BIOS time
;-----------------------------------------------------------------------------
init_time:
	push	ax
	push	dx

	pce	PCE_HOOK_GET_TIME_BIOS
	jnc	.ok

	xor	ax, ax
	xor	dx, dx

.ok:
	mov	[0x006c], ax
	mov	[0x006e], dx
	mov	[0x0070], byte 0

	pop	dx
	pop	ax
	ret


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
	dw	int_nn, 0x0000			; 00: F000:FF47
	dw	int_nn, 0x0000			; 01: F000:FF47
	dw	0xe2c3, 0xf000			; 02: F000:E2C3
	dw	int_nn, 0x0000			; 03: F000:FF47
	dw	int_nn, 0x0000			; 04: F000:FF47
	dw	0xff54, 0xf000			; 05: F000:FF54
	dw	int_nn, 0x0000			; 06: F000:FF47
	dw	int_nn, 0x0000			; 07: F000:FF47
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
	dw	0xff53, 0xf000			; 1B: F000:FF53
	dw	0xff53, 0xf000			; 1C: F000:FF53
	dw	0xf0a4, 0xf000			; 1D: F000:F0A4
	dw	0xefc7, 0xf000			; 1E: F000:EFC7
	dw	0x0000, 0xf000			; 1F: F000:0000


;-----------------------------------------------------------------------------

int_default:
int_nn:
	iret

;-----------------------------------------------------------------------------

int_13:
	pceint	0x13, .skip
	sti
	retf	2
.skip:
	jmp	0xf000:0xec59			; bios int 13

;-----------------------------------------------------------------------------

int_15:
	pceint	0x15, .skip
	sti
	retf	2
.skip:
	jmp	0xf000:0xf859			; bios int 15

;-----------------------------------------------------------------------------

int_19:
	xor	ax, ax
	xor	dx, dx
	int	0x13

	xor	ax, ax
	mov	ds, ax
	mov	es, ax

	cmp	word [4 * 0x13 + 0], 0xec59
	jne	.noint13
	cmp	word [4 * 0x13 + 2], 0xf000
	jne	.noint13

	mov	word [4 * 0x13 + 0], int_13
	mov	word [4 * 0x13 + 2], cs

.noint13:
	mov	word [4 * 0x1a + 0], int_1a
	mov	word [4 * 0x1a + 2], cs

	pce	PCE_HOOK_GET_BOOT		; get boot drive in AL
	jc	.fail

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
	push	bp
	mov	bp, sp
	push	word [bp + 6]			; flags
	popf
	pop	bp

	pceint	0x1a, .skip
	retf	2
.skip:
	jmp	0xf000:0xfe6e			; bios int 1a


;-----------------------------------------------------------------------------

%ifndef NOFILL
	set_pos	(0x6000)
%endif
