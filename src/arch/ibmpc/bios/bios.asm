;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   src/arch/ibmpc/bios/bios.asm                                 *
;* Created:     2003-04-14 by Hampa Hug <hampa@hampa.ch>                     *
;*****************************************************************************

; $Id$


; Define this to include PCE specific patches
%define PATCH 1


CPU 8086

%macro set_pos 1
	times %1 - ($ - $$) db 0
%endmacro


section .text

	org	0xe000

L_E0000:
	db      "1501476 COPR. IBM 1981"


L_E016:
	dw	L_E0D1				; Return address


;-----------------------------------------------------------------------------
; check CX bytes of RAM at ES:0000 (DS = ES)
;-----------------------------------------------------------------------------

L_E018:
	mov	cx, 0x4000

L_E01B:
	cld
	mov	bx, cx

	mov	ax, 0xaaaa			; first pattern
	mov	dx, 0xff55			; second and third pattern
	sub	di, di
	rep	stosb				; clear 16KB at ES:0

L_E028:
	dec	di
	std					; reverse direction

L_E02A:
	mov	si, di
	mov	cx, bx				; byte count

L_E02E:
	lodsb
	xor	al, ah				; check if readback ok
	jnz	L_E058

	mov	al, dl				; store next pattern
	stosb

	loop	L_E02E

	and	ah, ah				; check if done
	jz	L_E052

	mov	ah, al				; get next pattern
	xchg	dh, dl

	and	ah, ah
	jnz	L_E048

	mov	dl, ah
	jmp	short L_E028

L_E048:
	cld
	inc	di
	jz	L_E02A

	dec	di
	mov	dx, 0x01
	jmp	short L_E028

L_E052:
	in	al, 0x62
	and	al, 0xc0			; check if NMI occured
	mov	al, 0x00

L_E058:
	cld
	ret


	db	0xff				; E05A


;-----------------------------------------------------------------------------
; cold boot entry
;-----------------------------------------------------------------------------

start:						; E05B
	cli


;-----------------------------------------------------------------------------
; check 8088 register, flags and conditional jumps
;-----------------------------------------------------------------------------

	mov	ah, 0xd5
	sahf					; initialize the flags

	jnc	halt_cpu			; test the flags
	jnz	halt_cpu
	jpo	halt_cpu
	jns	halt_cpu

	lahf
	mov	cl, 5
	shr	ah, cl
	jnc	halt_cpu

	mov	al, 0x40
	shl	al, 1				; set the overflow flag
	jno	halt_cpu

	xor	ah, ah
	sahf					; clear all flags

	jna	halt_cpu			; test the flags
	js	halt_cpu
	jpe	halt_cpu

	lahf
	mov	cl, 5
	shr	ah, cl
	jc	halt_cpu

	shl	ah, 1
	jo	halt_cpu

	mov	ax, 0xffff			; first test value
	stc					; first pass

L_E08C:
	mov	ds, ax				; test registers
	mov	bx, ds
	mov	es, bx
	mov	cx, es
	mov	ss, cx
	mov	dx, ss
	mov	sp, dx
	mov	bp, sp
	mov	si, bp
	mov	di, si
	jnc	L_E0A9

	xor	ax, di				; check value 1
	jnz	halt_cpu

	clc					; second pass
	jmp	L_E08C

L_E0A9:
	or	ax, di				; check value 2
	jz	L_E0AE

halt_cpu:					; E0AD
L_E0AD:
	hlt


;-----------------------------------------------------------------------------
; verify bios checksum
;-----------------------------------------------------------------------------

L_E0AE:
	; AX = 0x0000
	out	0xa0, al
	out	0x83, al

	mov	dx, 0x3d8
	out	dx, al

	inc	al
	mov	dl, 0xb8
	out	dx, al

	; 99 = 10011001
	; group A: mode 0, group B: mode 0
	; port A[60]: input, port B[61]: output, port C[62]: input
	mov	al, 0x99
	out	0x63, al			; set up PPI 8255

	; FC = 11111100
	mov	al, 0xfc
	out	0x61, al			; set up port B


	mov	ax, cs
	mov	ss, ax
	mov	ds, ax

	mov	bh, 0xe0			; BX = E000
	mov	sp, L_E016

	; This call returns to E0D1
	jmp	L_EC4C				; add up bytes from DS:E000 to DS:FFFF

L_E0D1:
%ifdef PATCH
	; #### patch #### (bios is modifed -> incorrect checksum)
	nop
	nop
%else
	jnz	halt_cpu			; Verify checksum
%endif


;-----------------------------------------------------------------------------
; test the DMAC and timer channel 1 and set them up for ram refresh
;-----------------------------------------------------------------------------

	mov	al, 0x04
	out	0x08, al			; deactivate DMAC

	mov	al, 0x54			; 01010100b
	out	0x43, al			; Set up counter 1

	mov	al, cl				; CX = 0000
	out	0x41, al			; set counter low byte

	; this is a quick test of PIT counter 1
L_E0DF:
	mov	al, 0x40
	out	0x43, al			; counter latch for counter 1

	cmp	bl, 0xff
	je	L_E0EF

	in	al, 0x41			; get counter low byte
	or	bl, al

	loop	L_E0DF

	hlt

L_E0EF:
	mov	al, bl				; AL = FF
	sub	cx, cx

	out	0x41, al			; set counter low byte

L_E0F5:
	mov	al, 0x40
	out	0x43, al			; counter latch for counter 1
	nop
	nop
	in	al, 0x41			; get counter low byte

	and	bl, al
	jz	L_E104

	loop	L_E0F5

	hlt


L_E104:
	; Set up counter 1 for RAM refresh
	mov	al, 0x12
	out	0x41, al			; set counter 1 low byte

	; DMAC
	out	0x0d, al			; DMAC master clear

	mov	al, 0xff

L_E10C:
	mov	bl, al
	mov	bh, al
	mov	cx, 8				; 4 DMA channels
	sub	dx, dx

L_E115:
	out	dx, al				; set address or word count to FFFF
	push	ax
	out	dx, al
	mov	ax, 0x0101
	in	al, dx
	mov	ah, al
	in	al, dx
	cmp	bx, ax				; check if readback ok
	je	L_E124

	hlt

L_E124:
	inc	dx
	loop	L_E115

	inc	al				; do the same with a value of 0
	jz	L_E10C


	mov	ds, bx				; DS = 0
	mov	es, bx

	mov	al, 0xff
	out	0x01, al			; DMA channel 0 word count
	push	ax
	out	0x01, al

	mov	dl, 0x0b
	mov	al, 0x58			; read mode, autoinit, single transfer
	out	dx, al				; DMA mode register

	mov	al, 0x00
	out	0x08, al			; enable DMAC

	push	ax
	out	0x0a, al			; DMA mask register

	mov	cl, 0x03
	mov	al, 0x41			; write mode, demand transfer

L_E146:
	out	dx, al
	inc	al
	loop	L_E146


	mov	dx, 0x0213
	mov	al, 0x01
	out	dx, al


	mov	bp, [0x0472]			; check boot mode
	cmp	bp, 0x1234
	je	L_E165				; skip memory check


	; Set up stack for a call
	mov    sp, D_F041
	nop
	jmp	L_E018				; check first 16KB of memory

L_E162:
	jz	L_E165

	hlt


L_E165:
	sub	di, di

	in	al, 0x60
	and	al, 0x0c			; RAM size (HS0 and HS1)
	add	al, 0x04
	mov	cl, 0x0c
	shl	ax, cl				; RAM size in bytes
	mov	cx, ax
	cld

L_E174:						; clear memory
	stosb
	loop	L_E174

	mov	[0x472], bp

	mov	al, 0xf8
	out	0x61, al			; activate HS6 in port C

	in	al, 0x62
	and	al, 0x01			; get HS6
	mov	cl, 8 + 4
	rol	ax, cl

	mov	al, 0xfc
	out	0x61, al			; activate HS2-HS5 in port C

	in	al, 0x62
	and	al, 0x0f			; get HS2-HS5
	or	al, ah

	mov	bl, al
	mov	ah, 0x20
	mul	ah
	mov	[0x415], ax			; extra memory after 64K

	jz	L_E1B4				; ZF is undefined after MUL!

	mov	dx, 0x1000
	mov	ah, al
	mov	al, 0x00

L_E1A3:						; clear memory, 32K at a time
	mov	es, dx
	mov	cx, 0x8000
	sub	di, di
	rep	stosb

	add	dx, 0x0800
	dec	bl
	jnz	L_E1A3


;-----------------------------------------------------------------------------
; initialize the PIC 8259
;-----------------------------------------------------------------------------

L_E1B4:
	mov	al, 0x13			; ICW1
	out	0x20, al

	mov	al, 0x08			; ICW2 (base)
	out	0x21, al

	mov	al, 0x09			; ICW4
	out	0x21, al


	sub	ax, ax
	mov	es, ax				; ES = 0000

	mov	ax, 0x0030
	mov	ss, ax
	mov	sp, 256				; SS:SP = 0030:0100

	cmp	bp, 0x1234
	je	L_E1F7

	sub	di, di
	mov	ds, di				; DS:DI = 0000:0000

	mov	bx, 4 * 0x09
	;mov	[bx], word L_FF47
	db	0xc7, 0x07
	dw	L_FF47

	inc	bx
	inc	bx
	mov	[bx], cs			; INT 09 address = F000:FF47

	call	L_E643				; reset keyboard ?

	cmp	bl, 0x65			; check if manufacturing test 2
	jne	L_E1F7

	mov	dl, 0xff

L_E1EB:
	call	L_E650				; read in test program
	mov	al, bl
	stosb
	dec	dl
	jnz	L_E1EB

	int	0x3e

L_E1F7:						; initialize interrupt vectors
	mov	cx, 0x20
	sub	di, di

L_E1FC:
	mov	ax, L_FF47			; default interrupt handler
	stosw
	mov	ax, cs
	stosw
	loop	L_E1FC

	mov	word [4 * 0x02], int_02
	mov	word [4 * 0x05], int_05
	mov	word [4 * 0x18 + 2], 0xf600

	mov	dx, 0x0021
	mov	al, 0x00
	out	dx, al
	in	al, dx
	or	al, al
	jnz	L_E237

	mov	al, 0xff
	out	dx, al
	in	al, dx
	add	al, 1
	jnz	L_E237

	xor	ah, ah
	sti
	sub	cx, cx

L_E22F:
	loop	L_E22F

L_E231:
	loop	L_E231

	or	ah, ah				; check if interrupt occurred
	jz	L_E23F

L_E237:
	mov	dx, 0x101
	call	L_E5CF

	cli
	hlt


L_E23F:
	mov	al, 0xfe
	out	dx, al				; interrupt mask register

	mov	al, 0x10
	out	0x43, al			; PIT mode control

						; set pit counter 0 to a small value and wait for the interrupt.
	mov	cx, 0x0016
	mov	al, cl
	out	0x40, al

L_E24D:
	test	ah, 0xff			; check if interrupt occurred
	jnz	L_E256				; interrupt occurred, good
	loop	L_E24D

	jmp	short L_E237

L_E256:
						; set pit counter 0 to a large value and wait for the interrupt.
	mov	cl, 0x12

	mov	al, 0xff
	out	0x40, al

	mov	ax, 0xfe
	out	dx, al

L_E260:
	test	ah, 0xff
	jnz	L_E237				; interrupt occurred, not good
	loop	L_E260

; E267
	push	ds
	mov	di, 0x0040			; int 10 addr
	push	cs
	pop	ds
	mov	si, bios_int_addr_10		; FF03
	nop
	mov	cx, 16

	mov	al, 0xff
	out	dx, al

	mov	al, 0x36			; PIT counter 0 mode 3
	out	0x43, al

	mov	al, 0
	out	0x40, al			; counter value low byte

L_E27F:						; set up offsets int 10-1f
	movsw
	inc	di
	inc	di
	loop	L_E27F

	out	0x40, al			; counter value high byte

	pop	ds

db 0xE8, 0xB9, 0x03                     ; E287 call 0xe643
db 0x80, 0xFB, 0xAA                     ; E28A cmp bl,0xaa
db 0x74, 0x1E                           ; E28D jz 0xe2ad
db 0xB0, 0x3C                           ; E28F mov al,0x3c
db 0xE6, 0x61                           ; E291 out 0x61,al
db 0x90                                 ; E293 nop
db 0x90                                 ; E294 nop
db 0xE4, 0x60                           ; E295 in al,0x60
db 0x24, 0xFF                           ; E297 and al,0xff
db 0x75, 0x0E                           ; E299 jnz 0xe2a9
db 0xFE, 0x06, 0x12, 0x04               ; E29B inc byte [0x412]
db 0xC7, 0x06, 0x20, 0x00, 0x6D, 0xE6   ; E29F mov word [0x20],0xe66d
db 0xB0, 0xFE                           ; E2A5 mov al,0xfe
db 0xE6, 0x21                           ; E2A7 out 0x21,al

	mov	al, 0xcc
	out	0x61, al
	in	al, 0x60
	mov	ah, 0
	mov	[0x0410], ax			; store equipment word

	and	al, 0x30			; display adapter
	jnz	L_E2E1

						; display adapter with own bios
	mov	[0x0040], word int_default
	jmp	L_E363

	db	0xff
	db	0xff

;-----------------------------------------------------------------------------
; int 02
;-----------------------------------------------------------------------------

int_02:						; E2C3
	push	ax
	in	al, 0x62
	test	al, 0xc0
	jz	L_E2DF

	mov	si, L_FFDA			; "PARITY CHECK 2"
	nop

	test	al, 0x40
	jnz	L_E2D6

	mov	si, L_FF23			; "PARITY CHECK 1"
	nop

L_E2D6:
	sub	ax, ax
	int	0x10

	call	L_E6BA

	cli
	hlt

L_E2DF:
	pop	ax
	iret


L_E2E1:
	cmp	al, 0x30			; check if MDA
	je	L_E2ED

	inc	ah

	cmp	al, 0x20
	jne	L_E2ED

	mov	ah, 3

L_E2ED:
	xchg	ah, al

	push	ax
	sub	ah, ah
	int	0x10				; set video mode
	pop	ax

	push	ax

						; values for CGA
	mov	bx, 0xb000
	mov	dx, 0x3b8
	mov	cx, 0x1000
	mov	al, 1

	cmp	ah, 0x30
	je	L_E30E

						; fix values for MDA
	mov	bh, 0xb8
	mov	dl, 0xd8
	mov	ch, 0x40
	dec	al

L_E30E:
	out	dx, al
	cmp	bp, 0x1234
	mov	es, bx
	je	L_E31E

	mov	ds, bx
	call	L_E01B
	jnz	L_E350

L_E31E:
	pop	ax

	push	ax
	mov	ah, 0
	int	0x10

	mov	ax, 0x7020
	sub	di, di
	mov	cx, 40
	rep	stosw
	pop	ax

	push	ax

	cmp	ah, 0x30
	mov	dx, 0x3ba
	je	L_E33A
	mov	dl, 0xda

L_E33A:
	mov	ah, 8

L_E33C:
	sub	cx, cx

L_E33E:
	in	al, dx
	and	al, ah
	jnz	L_E347
	loop	L_E33E

	jmp	short L_E350

L_E347:
	sub	cx, cx

L_E349:
	in	al, dx
	and	al, ah
	jz	L_E358
	loop	L_E349

L_E350:
	mov	dx, 0x0102
	call	L_E5CF
	jmp	short L_E35E

L_E358:
	mov	cl, 3
	shr	ah, cl
	jnz	L_E33C

L_E35E:
	pop	ax

	mov	ah, 0
	int	0x10


; -- initialize bios extensions

L_E363:
	mov	dx, 0xc000

L_E366:
	mov	ds, dx

	sub	bx, bx
	mov	ax, [bx]			; read bios extension id

	push	bx
	pop	bx

	cmp	ax, 0xaa55			; check if valid extension
	jne	L_E378

	call	L_E684				; init bios extension
	jmp	short L_E37C

L_E378:
	add	dx, 0x80

L_E37C:
	cmp	dx, 0xc800
	jl	L_E366

	mov	dx, 0x210			; expansion card port
	mov	ax, 0x5555
	out	dx, al
	mov	al, 0x01
	in	al, dx
	cmp	al, ah
	jne	L_E3C4

	not	ax
	out	dx, al
	mov	al, 0x01
	in	al, dx
	cmp	al, ah
	jne	L_E3C4

L_E39A:
	mov	bx, ax
db 0xBA, 0x14, 0x02                     ; E39C mov dx,0x214
db 0x2E, 0x88, 0x07                     ; E39F mov [cs:bx],al
db 0xEE                                 ; E3A2 out dx,al
db 0x90                                 ; E3A3 nop
db 0xEC                                 ; E3A4 in al,dx
db 0x3A, 0xC7                           ; E3A5 cmp al,bh
db 0x75, 0x14                           ; E3A7 jnz 0xe3bd
db 0x42                                 ; E3A9 inc dx
db 0xEC                                 ; E3AA in al,dx
db 0x3A, 0xC4                           ; E3AB cmp al,ah
db 0x75, 0x0E                           ; E3AD jnz 0xe3bd
db 0x42                                 ; E3AF inc dx
db 0xEC                                 ; E3B0 in al,dx
db 0x3A, 0xC4                           ; E3B1 cmp al,ah
db 0x75, 0x08                           ; E3B3 jnz 0xe3bd
db 0xF7, 0xD0                           ; E3B5 not ax
	cmp	al, 0xaa
	je	L_E3C4
	jmp	short L_E39A

L_E3BD:
	mov	si, L_FEED
	nop
	call	L_E6BA

L_E3C4:
	call	set_bios_ds

	mov	al, [0x0010]			; configuration word
	and	al, 0x0c

	mov	ah, 0x04
	mul	ah				; ram size in KB
	add	al, 16

	mov	dx, ax
	mov	bx, ax

	mov	ax, [0x0015]			; post state 2

	cmp	bx, byte 64
	je	L_E3E0

	sub	ax, ax

L_E3E0:
	add	ax, bx
	mov	[0x0013], ax			; ram size

	cmp	bp, 0x1234
	push	ds
	je	L_E43B

db 0xBB, 0x00, 0x04                     ; E3EC mov bx,0x400
db 0xB9, 0x10, 0x00                     ; E3EF mov cx,0x10
db 0x3B, 0xD1                           ; E3F2 cmp dx,cx
db 0x76, 0x2D                           ; E3F4 jna 0xe423
db 0x8E, 0xDB                           ; E3F6 mov ds,bx
db 0x8E, 0xC3                           ; E3F8 mov es,bx
db 0x83, 0xC1, 0x10                     ; E3FA add cx,byte +0x10
db 0x81, 0xC3, 0x00, 0x04               ; E3FD add bx,0x400
db 0x51                                 ; E401 push cx
db 0x53                                 ; E402 push bx
db 0x52                                 ; E403 push dx
db 0xE8, 0x11, 0xFC                     ; E404 call 0xe018
db 0x5A                                 ; E407 pop dx
db 0x5B                                 ; E408 pop bx
db 0x59                                 ; E409 pop cx
db 0x74, 0xE6                           ; E40A jz 0xe3f2
db 0x8C, 0xDA                           ; E40C mov dx,ds
db 0x8A, 0xE8                           ; E40E mov ch,al
db 0x8A, 0xC6                           ; E410 mov al,dh
db 0xE8, 0x10, 0x02                     ; E412 call 0xe625
db 0x8A, 0xC5                           ; E415 mov al,ch
db 0xE8, 0x0B, 0x02                     ; E417 call 0xe625
	mov	si, L_FA67			; " 201"
	nop
	call	L_E6BA				; print string
	jmp	short L_E43B

db 0x1F                                 ; E423 pop ds
db 0x1E                                 ; E424 push ds
db 0x8B, 0x16, 0x15, 0x00               ; E425 mov dx,[0x15]
db 0x0B, 0xD2                           ; E429 or dx,dx
db 0x74, 0x0E                           ; E42B jz 0xe43b
db 0xB9, 0x00, 0x00                     ; E42D mov cx,0x0
db 0x81, 0xFB, 0x00, 0x10               ; E430 cmp bx,0x1000
db 0x77, 0x05                           ; E434 ja 0xe43b
db 0xBB, 0x00, 0x10                     ; E436 mov bx,0x1000
db 0xEB, 0xB7                           ; E439 jmp short 0xe3f2
L_E43B:
db 0x1F                                 ; E43B pop ds
db 0x80, 0x3E, 0x12, 0x00, 0x01         ; E43C cmp byte [0x12],0x1
db 0x74, 0x2A                           ; E441 jz 0xe46d
db 0xE8, 0xFD, 0x01                     ; E443 call 0xe643
db 0xE3, 0x1E                           ; E446 jcxz 0xe466
db 0xB0, 0x4D                           ; E448 mov al,0x4d
db 0xE6, 0x61                           ; E44A out 0x61,al
db 0x80, 0xFB, 0xAA                     ; E44C cmp bl,0xaa
db 0x75, 0x15                           ; E44F jnz 0xe466
db 0xB0, 0xCC                           ; E451 mov al,0xcc
db 0xE6, 0x61                           ; E453 out 0x61,al
db 0xB0, 0x4C                           ; E455 mov al,0x4c
db 0xE6, 0x61                           ; E457 out 0x61,al
db 0x2B, 0xC9                           ; E459 sub cx,cx
L_E45B:
db 0xE2, 0xFE                           ; E45B loop 0xe45b
db 0xE4, 0x60                           ; E45D in al,0x60
db 0x3C, 0x00                           ; E45F cmp al,0x0
db 0x74, 0x0A                           ; E461 jz 0xe46d
db 0xE8, 0xBF, 0x01                     ; E463 call 0xe625
	mov	si, 0xff33			; " 301"
	nop
	call	L_E6BA				; print string

L_E46D:
	sub	ax, ax
	mov	es, ax

	mov	cx, 8
	push	ds

	push	cs
	pop	ds
	mov	si, bios_int_addr_08		; FEF3
	nop
	mov	di, 0x0020

L_E47E:
	movsw
	inc	di
	inc	di
	loop	L_E47E

	pop	ds

	push	ds
db 0xB0, 0x4D                           ; E485 mov al,0x4d
db 0xE6, 0x61                           ; E487 out 0x61,al
db 0xB0, 0xFF                           ; E489 mov al,0xff
db 0xE6, 0x21                           ; E48B out 0x21,al
db 0xB0, 0xB6                           ; E48D mov al,0xb6
db 0xE6, 0x43                           ; E48F out 0x43,al
db 0xB8, 0xD3, 0x04                     ; E491 mov ax,0x4d3
db 0xE6, 0x42                           ; E494 out 0x42,al
db 0x8A, 0xC4                           ; E496 mov al,ah
db 0xE6, 0x42                           ; E498 out 0x42,al
db 0xE4, 0x62                           ; E49A in al,0x62
db 0x24, 0x10                           ; E49C and al,0x10
db 0xA2, 0x6B, 0x00                     ; E49E mov [0x6b],al
db 0xE8, 0xD5, 0x14                     ; E4A1 call 0xf979
db 0xE8, 0xD2, 0x14                     ; E4A4 call 0xf979
db 0xE3, 0x0C                           ; E4A7 jcxz 0xe4b5
db 0x81, 0xFB, 0x40, 0x05               ; E4A9 cmp bx,0x540
db 0x73, 0x06                           ; E4AD jnc 0xe4b5
db 0x81, 0xFB, 0x10, 0x04               ; E4AF cmp bx,0x410
db 0x73, 0x07                           ; E4B3 jnc 0xe4bc
db 0xBE, 0x39, 0xFF                     ; E4B5 mov si,0xff39
db 0x90                                 ; E4B8 nop
db 0xE8, 0xFE, 0x01                     ; E4B9 call 0xe6ba
db 0xBA, 0x00, 0xC8                     ; E4BC mov dx,0xc800
db 0x8E, 0xDA                           ; E4BF mov ds,dx
db 0x2B, 0xDB                           ; E4C1 sub bx,bx
db 0x8B, 0x07                           ; E4C3 mov ax,[bx]
db 0x3D, 0x55, 0xAA                     ; E4C5 cmp ax,0xaa55
db 0x75, 0x05                           ; E4C8 jnz 0xe4cf
db 0xE8, 0xB7, 0x01                     ; E4CA call 0xe684
db 0xEB, 0x04                           ; E4CD jmp short 0xe4d3
db 0x81, 0xC2, 0x80, 0x00               ; E4CF add dx,0x80
db 0x81, 0xFA, 0x00, 0xF6               ; E4D3 cmp dx,0xf600
db 0x7C, 0xE6                           ; E4D7 jl 0xe4bf
db 0xEB, 0x01                           ; E4D9 jmp short 0xe4dc
db 0x90                                 ; E4DB nop
db 0x2B, 0xDB                           ; E4DC sub bx,bx
db 0x8E, 0xDA                           ; E4DE mov ds,dx
db 0xE8, 0x69, 0x07                     ; E4E0 call 0xec4c
db 0x74, 0x03                           ; E4E3 jz 0xe4e8
db 0xE8, 0x21, 0x03                     ; E4E5 call 0xe809
db 0x80, 0xC6, 0x02                     ; E4E8 add dh,0x2
db 0x80, 0xFE, 0xFE                     ; E4EB cmp dh,0xfe
db 0x75, 0xEC                           ; E4EE jnz 0xe4dc
db 0x1F                                 ; E4F0 pop ds
db 0xA0, 0x10, 0x00                     ; E4F1 mov al,[0x10]
db 0xA8, 0x01                           ; E4F4 test al,0x1
db 0x75, 0x0A                           ; E4F6 jnz 0xe502
db 0x80, 0x3E, 0x12, 0x00, 0x01         ; E4F8 cmp byte [0x12],0x1
db 0x75, 0x3D                           ; E4FD jnz 0xe53c
	jmp	start
db 0xE4, 0x21                           ; E502 in al,0x21
db 0x24, 0xBF                           ; E504 and al,0xbf
db 0xE6, 0x21                           ; E506 out 0x21,al
	mov	ah, 0x00			; E508
	mov	dl, ah
	int	0x13				; reset fdc
db 0x72, 0x21                           ; E50E jc 0xe531
db 0xBA, 0xF2, 0x03                     ; E510 mov dx,0x3f2
db 0x52                                 ; E513 push dx
db 0xB0, 0x1C                           ; E514 mov al,0x1c
db 0xEE                                 ; E516 out dx,al
db 0x2B, 0xC9                           ; E517 sub cx,cx
db 0xE2, 0xFE                           ; E519 loop 0xe519
db 0xE2, 0xFE                           ; E51B loop 0xe51b
db 0x33, 0xD2                           ; E51D xor dx,dx
db 0xB5, 0x01                           ; E51F mov ch,0x1
db 0x88, 0x16, 0x3E, 0x00               ; E521 mov [0x3e],dl
db 0xE8, 0x55, 0x09                     ; E525 call 0xee7d
db 0x72, 0x07                           ; E528 jc 0xe531
db 0xB5, 0x22                           ; E52A mov ch,0x22
db 0xE8, 0x4E, 0x09                     ; E52C call 0xee7d
db 0x73, 0x07                           ; E52F jnc 0xe538
db 0xBE, 0xEA, 0xFF                     ; E531 mov si,0xffea
db 0x90                                 ; E534 nop
db 0xE8, 0x82, 0x01                     ; E535 call 0xe6ba
db 0xB0, 0x0C                           ; E538 mov al,0xc
db 0x5A                                 ; E53A pop dx
db 0xEE                                 ; E53B out dx,al
db 0xBE, 0x1E, 0x00                     ; E53C mov si,0x1e
db 0x89, 0x36, 0x1A, 0x00               ; E53F mov [0x1a],si
db 0x89, 0x36, 0x1C, 0x00               ; E543 mov [0x1c],si
db 0x89, 0x36, 0x80, 0x00               ; E547 mov [0x80],si
db 0x83, 0xC6, 0x20                     ; E54B add si,byte +0x20
db 0x89, 0x36, 0x82, 0x00               ; E54E mov [0x82],si
db 0xE4, 0x21                           ; E552 in al,0x21
db 0x24, 0xFC                           ; E554 and al,0xfc
db 0xE6, 0x21                           ; E556 out 0x21,al
	mov	bp, L_E63D
	nop
db 0x2B, 0xF6                           ; E55C sub si,si
L_E55E:
db 0x2E, 0x8B, 0x56, 0x00               ; E55E mov dx,[cs:bp+0x0]
db 0xB0, 0xAA                           ; E562 mov al,0xaa
db 0xEE                                 ; E564 out dx,al
db 0x52                                 ; E565 push dx
db 0xEC                                 ; E566 in al,dx
db 0x5A                                 ; E567 pop dx
db 0x3C, 0xAA                           ; E568 cmp al,0xaa
db 0x75, 0x05                           ; E56A jnz 0xe571
db 0x89, 0x54, 0x08                     ; E56C mov [si+0x8],dx
db 0x46                                 ; E56F inc si
db 0x46                                 ; E570 inc si
db 0x45                                 ; E571 inc bp
db 0x45                                 ; E572 inc bp
	cmp	bp, L_E63D_end			; E643
	jne	L_E55E
db 0x2B, 0xDB                           ; E579 sub bx,bx
db 0xBA, 0xFA, 0x03                     ; E57B mov dx,0x3fa
db 0xEC                                 ; E57E in al,dx
db 0xA8, 0xF8                           ; E57F test al,0xf8
db 0x75, 0x06                           ; E581 jnz 0xe589
db 0xC7, 0x07, 0xF8, 0x03               ; E583 mov word [bx],0x3f8
db 0x43                                 ; E587 inc bx
db 0x43                                 ; E588 inc bx
db 0xB6, 0x02                           ; E589 mov dh,0x2
db 0xEC                                 ; E58B in al,dx
db 0xA8, 0xF8                           ; E58C test al,0xf8
db 0x75, 0x06                           ; E58E jnz 0xe596
db 0xC7, 0x07, 0xF8, 0x02               ; E590 mov word [bx],0x2f8
db 0x43                                 ; E594 inc bx
db 0x43                                 ; E595 inc bx
db 0x8B, 0xC6                           ; E596 mov ax,si
db 0xB1, 0x03                           ; E598 mov cl,0x3
db 0xD2, 0xC8                           ; E59A ror al,cl
db 0x0A, 0xC3                           ; E59C or al,bl
db 0xA2, 0x11, 0x00                     ; E59E mov [0x11],al
db 0xB2, 0x01                           ; E5A1 mov dl,0x1
db 0xEC                                 ; E5A3 in al,dx
db 0xA8, 0x0F                           ; E5A4 test al,0xf
db 0x75, 0x05                           ; E5A6 jnz 0xe5ad
db 0x80, 0x0E, 0x11, 0x00, 0x10         ; E5A8 or byte [0x11],0x10
db 0x1E                                 ; E5AD push ds
db 0x07                                 ; E5AE pop es
db 0xBF, 0x78, 0x00                     ; E5AF mov di,0x78
db 0xB8, 0x14, 0x14                     ; E5B2 mov ax,0x1414
db 0xAB                                 ; E5B5 stosw
db 0xAB                                 ; E5B6 stosw
db 0xB8, 0x01, 0x01                     ; E5B7 mov ax,0x101
db 0xAB                                 ; E5BA stosw
db 0xAB                                 ; E5BB stosw
db 0xB0, 0x80                           ; E5BC mov al,0x80
db 0xE6, 0xA0                           ; E5BE out 0xa0,al
db 0x80, 0x3E, 0x12, 0x00, 0x01         ; E5C0 cmp byte [0x12],0x1
db 0x74, 0x06                           ; E5C5 jz 0xe5cd
db 0xBA, 0x01, 0x00                     ; E5C7 mov dx,0x1
db 0xE8, 0x02, 0x00                     ; E5CA call 0xe5cf
db 0xCD, 0x19                           ; E5CD int 0x19

L_E5CF:
db 0x9C                                 ; E5CF pushf
db 0xFA                                 ; E5D0 cli
db 0x1E                                 ; E5D1 push ds
db 0xE8, 0x69, 0x19                     ; E5D2 call 0xff3e
db 0x0A, 0xF6                           ; E5D5 or dh,dh
db 0x74, 0x18                           ; E5D7 jz 0xe5f1
L_E5D9:
	mov	bl, 0x06
	call	beep

L_E5DE:
	loop	L_E5DE

	dec	dh
	jnz	L_E5D9

db 0x80, 0x3E, 0x12, 0x00, 0x01         ; E5E4 cmp byte [0x12],0x1
db 0x75, 0x06                           ; E5E9 jnz 0xe5f1
db 0xB0, 0xCD                           ; E5EB mov al,0xcd
db 0xE6, 0x61                           ; E5ED out 0x61,al
db 0xEB, 0xE8                           ; E5EF jmp short 0xe5d9
L_E5F1:
db 0xB3, 0x01                           ; E5F1 mov bl,0x1
	call	beep
db 0xE2, 0xFE                           ; E5F6 loop 0xe5f6
db 0xFE, 0xCA                           ; E5F8 dec dl
db 0x75, 0xF5                           ; E5FA jnz 0xe5f1
db 0xE2, 0xFE                           ; E5FC loop 0xe5fc
db 0xE2, 0xFE                           ; E5FE loop 0xe5fe
db 0x1F                                 ; E600 pop ds
db 0x9D                                 ; E601 popf
	ret



;-----------------------------------------------------------------------------
; beep
; inp: BL = length
;-----------------------------------------------------------------------------

beep:						; E603
	mov	al, 0xb6
	out	0x43, al

	mov	ax, 0x0533
	out	0x42, al

	mov	al, ah
	out	0x42, al

	in	al, 0x61
	mov	ah, al
	or	al, 3
	out	0x61, al

	sub	cx, cx

L_E61A:
	loop	L_E61A

	dec	bl
	jnz	L_E61A

	mov	al, ah
	out	0x61, al
	ret


prt_hex_al:					; E625
	push	ax
	mov	cl, 4
	shr	al, cl
	call	prt_nibble
	pop	ax

	and	al, 0x0f

; print nibble in AL
prt_nibble:					; E630
	add	al, 0x90
	daa
	adc	al, 0x40
	daa

prt_char:					; E636
	mov	ah, 0x0e
	mov	bh, 0
	int	0x10
	ret

L_E63D:
	dw	0x03bc				; parallel port addresses
	dw	0x0378
	dw	0x0278
L_E63D_end:

L_E643:
	mov	al, 0x0c
	out	0x61, al
	mov	cx, 0x2956

L_E64A:
	loop	L_E64A

	mov	al, 0xcc
	out	0x61, al

L_E650:
	mov	al, 0x4c
	out	0x61, al

	mov	al, 0xfd
	out	0x21, al

	sti

	mov	ah, 0
	sub	cx, cx

L_E65D:
	test	ah, 0xff
	jnz	L_E664
	loop	L_E65D

L_E664:
	in	al, 0x60
	mov	bl, al

	mov	al, 0xcc
	out	0x61, al
	ret

db 0xFB                                 ; E66D sti
db 0x50                                 ; E66E push ax
db 0xE4, 0x61                           ; E66F in al,0x61
db 0x8A, 0xE0                           ; E671 mov ah,al
db 0xF6, 0xD0                           ; E673 not al
db 0x24, 0x40                           ; E675 and al,0x40
db 0x80, 0xE4, 0xBF                     ; E677 and ah,0xbf
db 0x0A, 0xC4                           ; E67A or al,ah
db 0xE6, 0x61                           ; E67C out 0x61,al
db 0xB0, 0x20                           ; E67E mov al,0x20
db 0xE6, 0x20                           ; E680 out 0x20,al
db 0x58                                 ; E682 pop ax
db 0xCF                                 ; E683 iret

; found bios extension in [c000-c800]
L_E684:
	mov	ax, 0x40
	mov	es, ax

	sub	ah, ah
	mov	al, [bx + 2]			; get bios extension size
	mov	cl, 9
	shl	ax, cl
	mov	cx, ax				; size in bytes

	push	cx
	mov	cl, 4
	shr	ax, cl				; size in paragraphs
	add	dx, ax				; skip extension
	pop	cx

	call	L_EC4F				; Add up all bytes
	jz	.chksumok

	call	L_E809
	jmp	short L_E6B9

.chksumok:					; E6A6
	push	dx
	mov	[es:0x100], word 3
	mov	[es:0x102], ds

	call	far [es:0x100]
	pop	dx

L_E6B9:
	ret


L_E6BA:
	call	set_bios_ds

	cmp	[0x0012], byte 0x01
	jne	L_E6C9

	mov	dh, 0x01
	jmp	L_E5CF

L_E6C9:
	mov	al, [cs:si]
	inc	si

	push	ax
	call	prt_char
	pop	ax

	cmp	al, 0x0a
	jne	L_E6C9
	ret

L_E6D7:
	db	" ROM", 0x0d, 0x0a


;-----------------------------------------------------------------------------
; default hardware interrupt handler
;-----------------------------------------------------------------------------

L_E6DD:
	push	ax
	mov	al, 0x20
	out	0x20, al			; end of interrupt
	pop	ax
	iret


L_E6E4:
	jmp	0x0000:0x7c00

	db	0xff, 0xff, 0xff, 0xff
	db	0xff, 0xff, 0xff, 0xff
	db	0xff


;-----------------------------------------------------------------------------
; int 19 - boot os
;-----------------------------------------------------------------------------

int_19:						; E6F2
	sti

	sub	ax, ax
	mov	ds, ax

	mov	[4 * 0x1e], word int_1e
	mov	[4 * 0x1e + 2], cs

	mov	ax, [0x400 + 0x10]		; equipment word
	test	al, 0x01			; check if floppy present
	jz	L_E726

	mov	cx, 4

L_E70B:
	push	cx
	mov	ah, 0
	int	0x13				; reset fdc
	jc	L_E721

	mov	ax, 0x0201
	sub	dx, dx
	mov	es, dx
	mov	bx, 0x7c00
	mov	cx, 0x0001
	int	0x13				; read boot sector

L_E721:
	pop	cx
	jnc	L_E6E4				; jump to boot sector
	loop	L_E70B

L_E726:
	int	0x18				; start rom basic


db 0xFF


L_E729:
	dw	0x0417				; 110 bps
	dw	0x0300				; 150 bps
	dw	0x0180				; 300 bps
	dw	0x00c0				; 600 bps
	dw	0x0060				; 1200 bps
	dw	0x0030				; 2400 bps
	dw	0x0018				; 4800 bps
	dw	0x000c				; 9600 bps


;-----------------------------------------------------------------------------
; int 14 - serial port
;-----------------------------------------------------------------------------

int_14:						; E739
	sti
	push	ds
	push	dx
	push	si
	push	di
	push	cx
	push	bx

	mov	si, dx
	mov	di, dx

	shl	si, 1

	call	set_bios_ds

	mov	dx, [si]			; get port base address
	or	dx, dx
	jz	L_E762

	or	ah, ah
	jz	int_14_00

	dec	ah
	jz	int_14_01

	dec	ah
	jz	int_14_02

	dec	ah
	jnz	L_E762

	jmp	int_14_03

L_E762:
	pop	bx
	pop	cx
	pop	di
	pop	si
	pop	dx
	pop	ds
	iret


;-----------------------------------------------------------------------------
; int 14 func 00 - initialize port
; inp: AL = parameters
;      DX = port number
; out: AL = modem status
;
; bits  description
; 5-7   data rate (110, 150, 300, 600, 1200, 2400, 4800, 9600)
; 3-4   parity (00 or 10 = none, 01 = odd, 11 = even)
; 2     stop bits (0 = 1 bit, 1 = 2 bits)
; 0-1   data bits (00 = 5, 01 = 6, 10 = 7, 11 = 8)
;-----------------------------------------------------------------------------

int_14_00:					; E769
	mov	ah, al

	add	dx, byte 0x03			; LCR
	mov	al, 0x80			; DLAB
	out	dx, al

	mov	dl, ah
	mov	cl, 0x04			; get rate in low bits
	rol	dl, cl
	and	dx, 0x000e
	mov	di, L_E729
	add	di, dx

	mov	dx, [si]			; port base address
	inc	dx
	mov	al, [cs:di + 0x01]
	out	dx, al				; divisor high

	dec	dx
	mov	al, [cs:di]
	out	dx, al				; divisor low

	add	dx, byte 0x03			; LCR
	mov	al, ah
	and	al, 0x1f
	out	dx, al				; parity, stop bits, data bits

	dec	dx
	dec	dx				; IER
	mov	al, 0x00
	out	dx, al				; disable interrupt

	jmp	short int_14_03


;-----------------------------------------------------------------------------
; int 14 func 01 - send character
; inp: AL = character
;      DX = port number
; out: AH = status
;-----------------------------------------------------------------------------

int_14_01:					; E79C
	push	ax

	add	dx, byte 0x04			; MCR
	mov	al, 0x03
	out	dx, al				; set DTR and RTS

	inc	dx
	inc	dx				; MSR
	mov	bh, 0x30			; DSR / CTS
	call	L_E7F2
	jz	L_E7B4

L_E7AC:
						; timeout
	pop	cx
	mov	al, cl

L_E7AF:
	or	ah, 0x80			; indicate error
	jmp	short L_E762

L_E7B4:
	dec	dx				; LSR
	mov	bh, 0x20			; TBE
	call	L_E7F2
	jnz	L_E7AC

	sub	dx, byte 0x05			; TXR
	pop	cx
	mov	al, cl				; the character
	out	dx, al

	jmp	short L_E762


;-----------------------------------------------------------------------------
; int 14 func 02 - receive character
; inp: DX = port number
; out: AH = status
;      AL = character
;-----------------------------------------------------------------------------

int_14_02:					; E7C5
	add	dx, byte 0x04			; MCR
	mov	al, 0x01			; DTR
	out	dx, al

	inc	dx
	inc	dx				; MSR
	mov	bh, 0x20			; DSR
	call	L_E7F2
	jnz	L_E7AF

	dec	dx				; LSR
	mov	bh, 0x01			; RRD
	call	L_E7F2
	jnz	L_E7AF

	and	ah, 0x1e
	mov	dx, [si]			; RXD
	in	al, dx				; read the character

	jmp	L_E762


int_14_03:                              ; E7E5
db 0x8B, 0x14                           ; E7E5 mov dx,[si]
db 0x83, 0xC2, 0x05                     ; E7E7 add dx,byte +0x5
db 0xEC                                 ; E7EA in al,dx
db 0x8A, 0xE0                           ; E7EB mov ah,al
db 0x42                                 ; E7ED inc dx
db 0xEC                                 ; E7EE in al,dx
db 0xE9, 0x70, 0xFF                     ; E7EF jmp 0xe762


L_E7F2:
	mov	bl, [di + 0x007c]		; port timeout value

L_E7F5:
	sub	cx, cx				; counter

L_E7F7:
	in	al, dx
	mov	ah, al
	and	al, bh
	cmp	al, bh
	je	L_E808
	loop	L_E7F7

	dec	bl
	jnz	L_E7F5

	or	bh, bh				; clear zero flag

L_E808:
	ret


L_E809:
	push	dx
	push	ax

	mov	dx, ds
	cmp	dx, 0xc800
	jng	L_E826

	mov	al, dh
	call	prt_hex_al

	mov	al, dl
	call	prt_hex_al

	mov	si, L_E6D7
	call	L_E6BA				; print " ROM"

L_E823:
	pop	ax
	pop	dx
	ret

L_E826:
	mov	dx, 0x0102
	call	L_E5CF
	jmp	short L_E823


;-----------------------------------------------------------------------------
; int 16 handler
;-----------------------------------------------------------------------------

int_16:						; E82E
	sti
	push	ds
	push	bx
	call	set_bios_ds

	or	ah, ah
	jz	int_16_00

	dec	ah
	jz	int_16_01

	dec	ah
	jz	int_16_02

	jmp	short int_16_done

;-----------------------------------------------------------------------------
; int 16 func 00
;-----------------------------------------------------------------------------

int_16_00:					; E842
	sti
	nop
	cli

	mov	bx, [0x001a]			; keyboard buffer offset
	cmp	bx, [0x001c]			; buffer end
	je	int_16_00

	mov	ax, [bx]

	call	L_E871
	mov	[0x001a], bx

	jmp	short int_16_done


;-----------------------------------------------------------------------------
; int 16 func 01
;-----------------------------------------------------------------------------

int_16_01:					; E85A
	cli

	mov	bx, [0x001a]
	cmp	bx, [0x001c]			; check if buffer empty
	mov	ax, [bx]

	sti

	pop	bx
	pop	ds

	retf	2


;-----------------------------------------------------------------------------
; int 16 func 02
;-----------------------------------------------------------------------------

int_16_02:
	mov	al, [0x0017]			; keyboard status


int_16_done:					; E86E
	pop	bx
	pop	ds
	iret


L_E871:
	inc	bx
	inc	bx
	cmp	bx, [0x0082]			; end of keyboard buffer
	jne	L_E87D

	mov	bx, [0x0080]			; start of keyboard buffer

L_E87D:
	ret


L_E87E:
	db	0x52, 0x3a, 0x45, 0x46		; these are key codes
	db	0x38, 0x1d, 0x2a, 0x36

L_E886:
	db	0x80, 0x40, 0x20, 0x10
	db	0x08, 0x04, 0x02, 0x01

db 0x1B, 0xFF                           ; E88E sbb di,di
db 0x00, 0xFF                           ; E890 add bh,bh
db 0xFF                                 ; E892 db 0xFF
db 0xFF, 0x1E, 0xFF, 0xFF               ; E893 call far [0xffff]
db 0xFF                                 ; E897 db 0xFF
db 0xFF, 0x1F                           ; E898 call far [bx]
db 0xFF                                 ; E89A db 0xFF
db 0x7F, 0xFF                           ; E89B jg 0xe89c
db 0x11, 0x17                           ; E89D adc [bx],dx
db 0x05, 0x12, 0x14                     ; E89F add ax,0x1412
db 0x19, 0x15                           ; E8A2 sbb [di],dx
db 0x09, 0x0F                           ; E8A4 or [bx],cx
db 0x10, 0x1B                           ; E8A6 adc [bp+di],bl
db 0x1D, 0x0A, 0xFF                     ; E8A8 sbb ax,0xff0a
db 0x01, 0x13                           ; E8AB add [bp+di],dx
db 0x04, 0x06                           ; E8AD add al,0x6
db 0x07                                 ; E8AF pop es
db 0x08, 0x0A                           ; E8B0 or [bp+si],cl
db 0x0B, 0x0C                           ; E8B2 or cx,[si]
db 0xFF                                 ; E8B4 db 0xFF
db 0xFF                                 ; E8B5 db 0xFF
db 0xFF                                 ; E8B6 db 0xFF
db 0xFF, 0x1C                           ; E8B7 call far [si]
db 0x1A, 0x18                           ; E8B9 sbb bl,[bx+si]
db 0x03, 0x16, 0x02, 0x0E               ; E8BB add dx,[0xe02]
db 0x0D, 0xFF, 0xFF                     ; E8BF or ax,0xffff
db 0xFF                                 ; E8C2 db 0xFF
db 0xFF                                 ; E8C3 db 0xFF
db 0xFF                                 ; E8C4 db 0xFF
db 0xFF, 0x20                           ; E8C5 jmp near [bx+si]
db 0xFF, 0x5E, 0x5F                     ; E8C7 call far [bp+0x5f]
db 0x60                                 ; E8CA pusha
db 0x61                                 ; E8CB popa
db 0x62, 0x63, 0x64                     ; E8CC bound sp,[bp+di+0x64]
db 0x65                                 ; E8CF db 0x65
db 0x66                                 ; E8D0 db 0x66
db 0x67                                 ; E8D1 db 0x67
db 0xFF                                 ; E8D2 db 0xFF
db 0xFF, 0x77, 0xFF                     ; E8D3 push word [bx-0x1]
db 0x84, 0xFF                           ; E8D6 test bh,bh
db 0x73, 0xFF                           ; E8D8 jnc 0xe8d9
db 0x74, 0xFF                           ; E8DA jz 0xe8db
db 0x75, 0xFF                           ; E8DC jnz 0xe8dd
db 0x76, 0xFF                           ; E8DE jna 0xe8df
db 0xFF, 0x1B                           ; E8E0 call far [bp+di]
db 0x31, 0x32                           ; E8E2 xor [bp+si],si
db 0x33, 0x34                           ; E8E4 xor si,[si]
db 0x35, 0x36, 0x37                     ; E8E6 xor ax,0x3736
db 0x38, 0x39                           ; E8E9 cmp [bx+di],bh
db 0x30, 0x2D                           ; E8EB xor [di],ch
db 0x3D, 0x08, 0x09                     ; E8ED cmp ax,0x908
db 0x71, 0x77                           ; E8F0 jno 0xe969
db 0x65, 0x72, 0x74                     ; E8F2 gs jc 0xe969
db 0x79, 0x75                           ; E8F5 jns 0xe96c
db 0x69, 0x6F, 0x70, 0x5B, 0x5D         ; E8F7 imul bp,[bx+0x70],0x5d5b
db 0x0D, 0xFF, 0x61                     ; E8FC or ax,0x61ff
db 0x73, 0x64                           ; E8FF jnc 0xe965
db 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3B     ; E901 a32 push dword 0x3b6c6b6a
db 0x27                                 ; E908 daa
db 0x60                                 ; E909 pusha
db 0xFF, 0x5C, 0x7A                     ; E90A call far [si+0x7a]
db 0x78, 0x63                           ; E90D js 0xe972
db 0x76, 0x62                           ; E90F jna 0xe973
db 0x6E                                 ; E911 outsb
db 0x6D                                 ; E912 insw
db 0x2C, 0x2E                           ; E913 sub al,0x2e
db 0x2F                                 ; E915 das
db 0xFF, 0x2A                           ; E916 jmp far [bp+si]
db 0xFF, 0x20                           ; E918 jmp near [bx+si]
db 0xFF, 0x1B                           ; E91A call far [bp+di]
db 0x21, 0x40, 0x23                     ; E91C and [bx+si+0x23],ax
db 0x24, 0x25                           ; E91F and al,0x25
db 0x5E                                 ; E921 pop si
db 0x26, 0x2A, 0x28                     ; E922 sub ch,[es:bx+si]
db 0x29, 0x5F, 0x2B                     ; E925 sub [bx+0x2b],bx
db 0x08, 0x00                           ; E928 or [bx+si],al
db 0x51                                 ; E92A push cx
db 0x57                                 ; E92B push di
db 0x45                                 ; E92C inc bp
db 0x52                                 ; E92D push dx
db 0x54                                 ; E92E push sp
db 0x59                                 ; E92F pop cx
db 0x55                                 ; E930 push bp
db 0x49                                 ; E931 dec cx
db 0x4F                                 ; E932 dec di
db 0x50                                 ; E933 push ax
db 0x7B, 0x7D                           ; E934 jpo 0xe9b3
db 0x0D, 0xFF, 0x41                     ; E936 or ax,0x41ff
db 0x53                                 ; E939 push bx
db 0x44                                 ; E93A inc sp
db 0x46                                 ; E93B inc si
db 0x47                                 ; E93C inc di
db 0x48                                 ; E93D dec ax
db 0x4A                                 ; E93E dec dx
db 0x4B                                 ; E93F dec bx
db 0x4C                                 ; E940 dec sp
db 0x3A, 0x22                           ; E941 cmp ah,[bp+si]
db 0x7E, 0xFF                           ; E943 jng 0xe944
db 0x7C, 0x5A                           ; E945 jl 0xe9a1
db 0x58                                 ; E947 pop ax
db 0x43                                 ; E948 inc bx
db 0x56                                 ; E949 push si
db 0x42                                 ; E94A inc dx
db 0x4E                                 ; E94B dec si
db 0x4D                                 ; E94C dec bp
db 0x3C, 0x3E                           ; E94D cmp al,0x3e
db 0x3F                                 ; E94F aas
db 0xFF, 0x00                           ; E950 inc word [bx+si]
db 0xFF, 0x20                           ; E952 jmp near [bx+si]
db 0xFF, 0x54, 0x55                     ; E954 call near [si+0x55]
db 0x56                                 ; E957 push si
db 0x57                                 ; E958 push di
db 0x58                                 ; E959 pop ax
db 0x59                                 ; E95A pop cx
db 0x5A                                 ; E95B pop dx
db 0x5B                                 ; E95C pop bx
db 0x5C                                 ; E95D pop sp
db 0x5D                                 ; E95E pop bp
db 0x68, 0x69, 0x6A                     ; E95F push word 0x6a69
db 0x6B, 0x6C, 0x6D, 0x6E               ; E962 imul bp,[si+0x6d],byte +0x6e
db 0x6F                                 ; E966 outsw
db 0x70, 0x71                           ; E967 jo 0xe9da
db 0x37                                 ; E969 aaa
db 0x38, 0x39                           ; E96A cmp [bx+di],bh
db 0x2D, 0x34, 0x35                     ; E96C sub ax,0x3534
db 0x36, 0x2B, 0x31                     ; E96F sub si,[ss:bx+di]
db 0x32, 0x33                           ; E972 xor dh,[bp+di]
db 0x30, 0x2E, 0x47, 0x48               ; E974 xor [0x4847],ch
db 0x49                                 ; E978 dec cx
db 0xFF, 0x4B, 0xFF                     ; E979 dec word [bp+di-0x1]
db 0x4D                                 ; E97C dec bp
db 0xFF, 0x4F, 0x50                     ; E97D dec word [bx+0x50]
db 0x51                                 ; E980 push cx
db 0x52                                 ; E981 push dx
db 0x53                                 ; E982 push bx
db 0xFF                                 ; E983 db 0xFF
db 0xFF                                 ; E984 db 0xFF
db 0xFF                                 ; E985 db 0xFF
db 0xFF                                 ; E986 db 0xFF


;-----------------------------------------------------------------------------
; int 09 handler
;-----------------------------------------------------------------------------

int_09:						; E987
	sti

	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es

	cld
	call	set_bios_ds

	in	al, 0x60
	push	ax

	in	al, 0x61
	mov	ah, al
	or	al, 0x80
	out	0x61, al
	xchg	ah, al
	out	0x61, al

	pop	ax
	mov	ah, al

	cmp	al, 0xff
	jne	L_E9AD
	jmp	L_EC27

L_E9AD:
	and	al, 0x7f

	push	cs
	pop	es
	mov	di, L_E87E
	mov	cx, 8
	repne	scasb				; check if special key (shift, ...)

	mov	al, ah
	je	L_E9C0
	jmp	L_EA45

L_E9C0:
	sub	di, L_E87E + 1
	mov	ah, [cs:di + L_E886]		; convert to bit mask

	test	al, 0x80			; check if break code
	jnz	L_EA1E

	cmp	ah, 0x10
	jae	L_E9D9

						; shift, alt, ctrl
	or	[0x0017], ah			; keyboard status
	jmp	L_EA59

L_E9D9:
						; caps lock, scroll lock, insert

	test	[0x0017], byte 0x04		; ctrl
	jnz	L_EA45

	cmp	al, 82				; insert
	jne	L_EA06

	test	[0x0017], byte 0x08		; alt
	jnz	L_EA45

	test	[0x0017], byte 0x20		; num lock
	jnz	L_E9FF

	test	[0x0017], byte 0x03		; either shift
	je	L_EA06

L_E9F9:
	mov	ax, 0x5230
	jmp	L_EBD5

L_E9FF:
	test	[0x0017], byte 0x03		; either shift
	jz	L_E9F9

L_EA06:
db 0x84, 0x26, 0x18, 0x00               ; EA06 test [0x18],ah
db 0x75, 0x4D                           ; EA0A jnz 0xea59
db 0x08, 0x26, 0x18, 0x00               ; EA0C or [0x18],ah
db 0x30, 0x26, 0x17, 0x00               ; EA10 xor [0x17],ah
db 0x3C, 0x52                           ; EA14 cmp al,0x52
db 0x75, 0x41                           ; EA16 jnz 0xea59
db 0xB8, 0x00, 0x52                     ; EA18 mov ax,0x5200
db 0xE9, 0xB7, 0x01                     ; EA1B jmp 0xebd5
L_EA1E:
db 0x80, 0xFC, 0x10                     ; EA1E cmp ah,0x10
db 0x73, 0x1A                           ; EA21 jnc 0xea3d
db 0xF6, 0xD4                           ; EA23 not ah
db 0x20, 0x26, 0x17, 0x00               ; EA25 and [0x17],ah
db 0x3C, 0xB8                           ; EA29 cmp al,0xb8
db 0x75, 0x2C                           ; EA2B jnz 0xea59
db 0xA0, 0x19, 0x00                     ; EA2D mov al,[0x19]
db 0xB4, 0x00                           ; EA30 mov ah,0x0
db 0x88, 0x26, 0x19, 0x00               ; EA32 mov [0x19],ah
db 0x3C, 0x00                           ; EA36 cmp al,0x0
db 0x74, 0x1F                           ; EA38 jz 0xea59
db 0xE9, 0xA1, 0x01                     ; EA3A jmp 0xebde
db 0xF6, 0xD4                           ; EA3D not ah
db 0x20, 0x26, 0x18, 0x00               ; EA3F and [0x18],ah
db 0xEB, 0x14                           ; EA43 jmp short 0xea59
L_EA45:
db 0x3C, 0x80                           ; EA45 cmp al,0x80
db 0x73, 0x10                           ; EA47 jnc 0xea59
db 0xF6, 0x06, 0x18, 0x00, 0x08         ; EA49 test byte [0x18],0x8
db 0x74, 0x17                           ; EA4E jz 0xea67
db 0x3C, 0x45                           ; EA50 cmp al,0x45
db 0x74, 0x05                           ; EA52 jz 0xea59
db 0x80, 0x26, 0x18, 0x00, 0xF7         ; EA54 and byte [0x18],0xf7
L_EA59:
db 0xFA                                 ; EA59 cli
db 0xB0, 0x20                           ; EA5A mov al,0x20
db 0xE6, 0x20                           ; EA5C out 0x20,al
db 0x07                                 ; EA5E pop es
db 0x1F                                 ; EA5F pop ds
db 0x5F                                 ; EA60 pop di
db 0x5E                                 ; EA61 pop si
db 0x5A                                 ; EA62 pop dx
db 0x59                                 ; EA63 pop cx
db 0x5B                                 ; EA64 pop bx
db 0x58                                 ; EA65 pop ax
db 0xCF                                 ; EA66 iret

L_EA67:
	test	byte [0x0017], 0x08		; alt
	jnz	L_EA71
	jmp	L_EB02

L_EA71:
	test	byte [0x0017], 0x04		; ctrl
	jz	L_EAAB

	cmp	al, 0x53			; del
	jnz	L_EAAB

						; Alt-Ctrl-Del was pressed
	mov	word [0x0072], 0x1234

%ifdef PATCH
	; #### patch #### (new entry address)
	jmp	0xf000:0xfff0
%else
	jmp	0xf000:start
%endif

db 0x52                                 ; EA87 push dx
db 0x4F                                 ; EA88 dec di
db 0x50                                 ; EA89 push ax
db 0x51                                 ; EA8A push cx
db 0x4B                                 ; EA8B dec bx
db 0x4C                                 ; EA8C dec sp
db 0x4D                                 ; EA8D dec bp
db 0x47                                 ; EA8E inc di
db 0x48                                 ; EA8F dec ax
db 0x49                                 ; EA90 dec cx
db 0x10, 0x11                           ; EA91 adc [bx+di],dl
db 0x12, 0x13                           ; EA93 adc dl,[bp+di]
db 0x14, 0x15                           ; EA95 adc al,0x15
db 0x16                                 ; EA97 push ss
db 0x17                                 ; EA98 pop ss
db 0x18, 0x19                           ; EA99 sbb [bx+di],bl
db 0x1E                                 ; EA9B push ds
db 0x1F                                 ; EA9C pop ds
db 0x20, 0x21                           ; EA9D and [bx+di],ah
db 0x22, 0x23                           ; EA9F and ah,[bp+di]
db 0x24, 0x25                           ; EAA1 and al,0x25
db 0x26, 0x2C, 0x2D                     ; EAA3 es sub al,0x2d
db 0x2E, 0x2F                           ; EAA6 cs das
db 0x30, 0x31                           ; EAA8 xor [bx+di],dh
db 0x32
L_EAAB:
	cmp	al, 0x39
	jnz	L_EAB4
db 0xB0, 0x20                           ; EAAF mov al,0x20
db 0xE9, 0x21, 0x01                     ; EAB1 jmp 0xebd5
L_EAB4:
db 0xBF, 0x87, 0xEA                     ; EAB4 mov di,0xea87
db 0xB9, 0x0A, 0x00                     ; EAB7 mov cx,0xa
db 0xF2, 0xAE                           ; EABA repne scasb
db 0x75, 0x12                           ; EABC jnz 0xead0
db 0x81, 0xEF, 0x88, 0xEA               ; EABE sub di,0xea88
db 0xA0, 0x19, 0x00                     ; EAC2 mov al,[0x19]
db 0xB4, 0x0A                           ; EAC5 mov ah,0xa
db 0xF6, 0xE4                           ; EAC7 mul ah
db 0x03, 0xC7                           ; EAC9 add ax,di
db 0xA2, 0x19, 0x00                     ; EACB mov [0x19],al
db 0xEB, 0x89                           ; EACE jmp short 0xea59
db 0xC6, 0x06, 0x19, 0x00, 0x00         ; EAD0 mov byte [0x19],0x0
db 0xB9, 0x1A, 0x00                     ; EAD5 mov cx,0x1a
db 0xF2, 0xAE                           ; EAD8 repne scasb
db 0x75, 0x05                           ; EADA jnz 0xeae1
db 0xB0, 0x00                           ; EADC mov al,0x0
db 0xE9, 0xF4, 0x00                     ; EADE jmp 0xebd5
db 0x3C, 0x02                           ; EAE1 cmp al,0x2
db 0x72, 0x0C                           ; EAE3 jc 0xeaf1
db 0x3C, 0x0E                           ; EAE5 cmp al,0xe
db 0x73, 0x08                           ; EAE7 jnc 0xeaf1
db 0x80, 0xC4, 0x76                     ; EAE9 add ah,0x76
db 0xB0, 0x00                           ; EAEC mov al,0x0
db 0xE9, 0xE4, 0x00                     ; EAEE jmp 0xebd5
db 0x3C, 0x3B                           ; EAF1 cmp al,0x3b
db 0x73, 0x03                           ; EAF3 jnc 0xeaf8
db 0xE9, 0x61, 0xFF                     ; EAF5 jmp 0xea59
db 0x3C, 0x47                           ; EAF8 cmp al,0x47
db 0x73, 0xF9                           ; EAFA jnc 0xeaf5
db 0xBB, 0x5F, 0xE9                     ; EAFC mov bx,0xe95f
db 0xE9, 0x1B, 0x01                     ; EAFF jmp 0xec1d
L_EB02:
db 0xF6, 0x06, 0x17, 0x00, 0x04         ; EB02 test byte [0x17],0x4
db 0x74, 0x58                           ; EB07 jz 0xeb61
db 0x3C, 0x46                           ; EB09 cmp al,0x46
db 0x75, 0x18                           ; EB0B jnz 0xeb25
db 0x8B, 0x1E, 0x80, 0x00               ; EB0D mov bx,[0x80]
db 0x89, 0x1E, 0x1A, 0x00               ; EB11 mov [0x1a],bx
db 0x89, 0x1E, 0x1C, 0x00               ; EB15 mov [0x1c],bx
db 0xC6, 0x06, 0x71, 0x00, 0x80         ; EB19 mov byte [0x71],0x80
db 0xCD, 0x1B                           ; EB1E int 0x1b
db 0x2B, 0xC0                           ; EB20 sub ax,ax
db 0xE9, 0xB0, 0x00                     ; EB22 jmp 0xebd5
db 0x3C, 0x45                           ; EB25 cmp al,0x45
db 0x75, 0x21                           ; EB27 jnz 0xeb4a
db 0x80, 0x0E, 0x18, 0x00, 0x08         ; EB29 or byte [0x18],0x8
db 0xB0, 0x20                           ; EB2E mov al,0x20
db 0xE6, 0x20                           ; EB30 out 0x20,al
db 0x80, 0x3E, 0x49, 0x00, 0x07         ; EB32 cmp byte [0x49],0x7
db 0x74, 0x07                           ; EB37 jz 0xeb40
db 0xBA, 0xD8, 0x03                     ; EB39 mov dx,0x3d8
db 0xA0, 0x65, 0x00                     ; EB3C mov al,[0x65]
db 0xEE                                 ; EB3F out dx,al
db 0xF6, 0x06, 0x18, 0x00, 0x08         ; EB40 test byte [0x18],0x8
db 0x75, 0xF9                           ; EB45 jnz 0xeb40
db 0xE9, 0x14, 0xFF                     ; EB47 jmp 0xea5e
db 0x3C, 0x37                           ; EB4A cmp al,0x37
db 0x75, 0x06                           ; EB4C jnz 0xeb54
db 0xB8, 0x00, 0x72                     ; EB4E mov ax,0x7200
db 0xE9, 0x81, 0x00                     ; EB51 jmp 0xebd5
db 0xBB, 0x8E, 0xE8                     ; EB54 mov bx,0xe88e
db 0x3C, 0x3B                           ; EB57 cmp al,0x3b
db 0x72, 0x76                           ; EB59 jc 0xebd1
db 0xBB, 0xC8, 0xE8                     ; EB5B mov bx,0xe8c8
db 0xE9, 0xBC, 0x00                     ; EB5E jmp 0xec1d
db 0x3C, 0x47                           ; EB61 cmp al,0x47
db 0x73, 0x2C                           ; EB63 jnc 0xeb91
db 0xF6, 0x06, 0x17, 0x00, 0x03         ; EB65 test byte [0x17],0x3
db 0x74, 0x5A                           ; EB6A jz 0xebc6
db 0x3C, 0x0F                           ; EB6C cmp al,0xf
db 0x75, 0x05                           ; EB6E jnz 0xeb75
db 0xB8, 0x00, 0x0F                     ; EB70 mov ax,0xf00
db 0xEB, 0x60                           ; EB73 jmp short 0xebd5
db 0x3C, 0x37                           ; EB75 cmp al,0x37
db 0x75, 0x09                           ; EB77 jnz 0xeb82
db 0xB0, 0x20                           ; EB79 mov al,0x20
db 0xE6, 0x20                           ; EB7B out 0x20,al
db 0xCD, 0x05                           ; EB7D int 0x5
db 0xE9, 0xDC, 0xFE                     ; EB7F jmp 0xea5e
db 0x3C, 0x3B                           ; EB82 cmp al,0x3b
db 0x72, 0x06                           ; EB84 jc 0xeb8c
db 0xBB, 0x55, 0xE9                     ; EB86 mov bx,0xe955
db 0xE9, 0x91, 0x00                     ; EB89 jmp 0xec1d
db 0xBB, 0x1B, 0xE9                     ; EB8C mov bx,0xe91b
db 0xEB, 0x40                           ; EB8F jmp short 0xebd1
db 0xF6, 0x06, 0x17, 0x00, 0x20         ; EB91 test byte [0x17],0x20
db 0x75, 0x20                           ; EB96 jnz 0xebb8
db 0xF6, 0x06, 0x17, 0x00, 0x03         ; EB98 test byte [0x17],0x3
db 0x75, 0x20                           ; EB9D jnz 0xebbf
db 0x3C, 0x4A                           ; EB9F cmp al,0x4a
db 0x74, 0x0B                           ; EBA1 jz 0xebae
db 0x3C, 0x4E                           ; EBA3 cmp al,0x4e
db 0x74, 0x0C                           ; EBA5 jz 0xebb3
db 0x2C, 0x47                           ; EBA7 sub al,0x47
db 0xBB, 0x76, 0xE9                     ; EBA9 mov bx,0xe976
db 0xEB, 0x71                           ; EBAC jmp short 0xec1f
db 0xB8, 0x2D, 0x4A                     ; EBAE mov ax,0x4a2d
db 0xEB, 0x22                           ; EBB1 jmp short 0xebd5
db 0xB8, 0x2B, 0x4E                     ; EBB3 mov ax,0x4e2b
db 0xEB, 0x1D                           ; EBB6 jmp short 0xebd5
db 0xF6, 0x06, 0x17, 0x00, 0x03         ; EBB8 test byte [0x17],0x3
db 0x75, 0xE0                           ; EBBD jnz 0xeb9f
db 0x2C, 0x46                           ; EBBF sub al,0x46
db 0xBB, 0x69, 0xE9                     ; EBC1 mov bx,0xe969
db 0xEB, 0x0B                           ; EBC4 jmp short 0xebd1
db 0x3C, 0x3B                           ; EBC6 cmp al,0x3b
db 0x72, 0x04                           ; EBC8 jc 0xebce
db 0xB0, 0x00                           ; EBCA mov al,0x0
db 0xEB, 0x07                           ; EBCC jmp short 0xebd5
db 0xBB, 0xE1, 0xE8                     ; EBCE mov bx,0xe8e1
db 0xFE, 0xC8                           ; EBD1 dec al
db 0x2E, 0xD7                           ; EBD3 cs xlatb
L_EBD5:
db 0x3C, 0xFF                           ; EBD5 cmp al,0xff
db 0x74, 0x1F                           ; EBD7 jz 0xebf8
db 0x80, 0xFC, 0xFF                     ; EBD9 cmp ah,0xff
db 0x74, 0x1A                           ; EBDC jz 0xebf8
db 0xF6, 0x06, 0x17, 0x00, 0x40         ; EBDE test byte [0x17],0x40
db 0x74, 0x20                           ; EBE3 jz 0xec05
db 0xF6, 0x06, 0x17, 0x00, 0x03         ; EBE5 test byte [0x17],0x3
db 0x74, 0x0F                           ; EBEA jz 0xebfb
db 0x3C, 0x41                           ; EBEC cmp al,0x41
db 0x72, 0x15                           ; EBEE jc 0xec05
db 0x3C, 0x5A                           ; EBF0 cmp al,0x5a
db 0x77, 0x11                           ; EBF2 ja 0xec05
db 0x04, 0x20                           ; EBF4 add al,0x20
db 0xEB, 0x0D                           ; EBF6 jmp short 0xec05
db 0xE9, 0x5E, 0xFE                     ; EBF8 jmp 0xea59
db 0x3C, 0x61                           ; EBFB cmp al,0x61
db 0x72, 0x06                           ; EBFD jc 0xec05
db 0x3C, 0x7A                           ; EBFF cmp al,0x7a
db 0x77, 0x02                           ; EC01 ja 0xec05
db 0x2C, 0x20                           ; EC03 sub al,0x20
db 0x8B, 0x1E, 0x1C, 0x00               ; EC05 mov bx,[0x1c]
db 0x8B, 0xF3                           ; EC09 mov si,bx
db 0xE8, 0x63, 0xFC                     ; EC0B call 0xe871
db 0x3B, 0x1E, 0x1A, 0x00               ; EC0E cmp bx,[0x1a]
db 0x74, 0x13                           ; EC12 jz 0xec27
db 0x89, 0x04                           ; EC14 mov [si],ax
db 0x89, 0x1E, 0x1C, 0x00               ; EC16 mov [0x1c],bx
db 0xE9, 0x3C, 0xFE                     ; EC1A jmp 0xea59
db 0x2C, 0x3B                           ; EC1D sub al,0x3b
db 0x2E, 0xD7                           ; EC1F cs xlatb
db 0x8A, 0xE0                           ; EC21 mov ah,al
db 0xB0, 0x00                           ; EC23 mov al,0x0
db 0xEB, 0xAE                           ; EC25 jmp short 0xebd5
L_EC27:
db 0xB0, 0x20                           ; EC27 mov al,0x20
db 0xE6, 0x20                           ; EC29 out 0x20,al
db 0xBB, 0x80, 0x00                     ; EC2B mov bx,0x80
db 0xE4, 0x61                           ; EC2E in al,0x61
db 0x50                                 ; EC30 push ax
db 0x24, 0xFC                           ; EC31 and al,0xfc
db 0xE6, 0x61                           ; EC33 out 0x61,al
db 0xB9, 0x48, 0x00                     ; EC35 mov cx,0x48
db 0xE2, 0xFE                           ; EC38 loop 0xec38
db 0x0C, 0x02                           ; EC3A or al,0x2
db 0xE6, 0x61                           ; EC3C out 0x61,al
db 0xB9, 0x48, 0x00                     ; EC3E mov cx,0x48
db 0xE2, 0xFE                           ; EC41 loop 0xec41
db 0x4B                                 ; EC43 dec bx
db 0x75, 0xEB                           ; EC44 jnz 0xec31
db 0x58                                 ; EC46 pop ax
db 0xE6, 0x61                           ; EC47 out 0x61,al
db 0xE9, 0x12, 0xFE                     ; EC49 jmp 0xea5e


; Add up 8K bytes from DS:BX to DS:BX+2000
L_EC4C:
	mov	cx, 0x2000

L_EC4F:
	xor	al, al

.next:						; EC51
	add	al, [bx]
	inc	bx
	loop	.next

	or	al, al
	ret


;-----------------------------------------------------------------------------
; int 13
;-----------------------------------------------------------------------------

int_13:						; EC59
	sti
	push	bx
	push	cx
	push	ds
	push	si
	push	di
	push	bp
	push	dx
	mov	bp, sp

	call	set_bios_ds

	call	L_EC85

	mov	bx, 4
	call	dpt_get_param			; EE6C

	mov	[0x0040], ah
	mov	ah, [0x0041]
	cmp	ah, 1
	cmc					; carry set if AH != 0

	pop	dx
	pop	bp
	pop	di
	pop	si
	pop	ds
	pop	cx
	pop	bx
	retf	2


L_EC85:
	mov	dh, al				; sector count
	and	[0x003f], byte 0x7f		; clear current op is write

	or	ah, ah
	jz	int_13_00

	dec	ah
	jz	int_13_01

	mov	byte [0x0041], 0x00		; last operation status

	cmp	dl, 4				; check drive number
	jae	L_ECB1

	dec	ah
	jz	int_13_02

	dec	ah
	jnz	L_ECA9
	jmp	int_13_03

L_ECA9:
	dec	ah
	jz	int_13_04

	dec	ah
	jz	int_13_05

L_ECB1:
	mov	byte [0x0041], 0x01		; invalid request
	ret


;-----------------------------------------------------------------------------
; int 13 func 00 - reset fdc
;-----------------------------------------------------------------------------

int_13_00:					; ECB7
	mov	dx, 0x03f2			; digital output register
	cli

	mov	al, [0x003f]
	mov     cl, 0x04
	shl	al, cl
	test	al, 0x20			; drive motor 1
	jnz	L_ECD2

	test	al, 0x40			; drive motor 2
	jnz	L_ECD0

	test	al, 0x80			; drive motor 3
	jz	L_ECD4

	inc	al

L_ECD0:
	inc	al

L_ECD2:
	inc	al

L_ECD4:
	or	al, 0x08			; dma enable
	out	dx, al				; reset fdc

	mov	byte [0x003e], 0x00		; seek status
	mov	byte [0x0041], 0x00		; diskette status

	or	al, 0x04			; disable reset
	out	dx, al

	sti

	call	L_EF12				; sense interrupt status

	mov	al, [0x0042]			; FDC ST0
	cmp	al, 0xc0			; abnormal termination
	je	L_ECF5

	or	byte [0x0041], 0x20		; general controller failure
	ret

L_ECF5:
	mov	ah, 0x03			; specify
	call	fdc_send_byte

	mov	bx, (0 << 1) | 1
	call	dpt_get_param

	mov	bx, (1 << 1) | 1
	call	dpt_get_param

	ret


;-----------------------------------------------------------------------------
; int 13 func 01 - get last operation status
;-----------------------------------------------------------------------------

int_13_01:					; ED07
	mov	al, [0x0041]
	ret


;-----------------------------------------------------------------------------
; int 13 func 02 - read
;-----------------------------------------------------------------------------

int_13_02:					; ED0B
	mov	al, 0x46			; dma mode

L_ED0D:
	call	fdc_setup_dma

	mov	ah, 0xe6
	jmp	short L_ED4A


;-----------------------------------------------------------------------------
; int 13 func 04 - verify
;-----------------------------------------------------------------------------

int_13_04:					; ED14
	mov	al, 0x42
	jmp	short L_ED0D


;-----------------------------------------------------------------------------
; int 13 func 05 - format
;-----------------------------------------------------------------------------

int_13_05:					; ED18
	or	byte [0x003f], 0x80

	mov	al, 0x4a
	call	fdc_setup_dma			; EEC8

	mov	ah, 0x4d
	jmp	short L_ED4A


L_ED26:
	mov	bx, 0x0007
	call	dpt_get_param

	mov	bx, 0x0009
	call	dpt_get_param

	mov	bx, 0x000f
	call	dpt_get_param

	mov	bx, 0x0011
	jmp	L_EDE9


;-----------------------------------------------------------------------------
; int 13 func 03 - write
;-----------------------------------------------------------------------------

int_13_03:					; ED3E
	or	byte [0x003f], 0x80
	mov	al, 0x4a
	call	fdc_setup_dma			; EEC8
	mov	ah, 0xc5


; do fdc read/write operation
L_ED4A:
	jnc	L_ED54				; dma setup was ok

	mov	byte [0x0041], 0x09
	mov	al, 0x00
	ret

L_ED54:
	push	ax
	push	cx

	mov	cl, dl
	mov	al, 0x01
	shl	al, cl

	cli

	mov	byte [0x0040], 0xff		; reset motor off timeout

	test	byte [0x003f], al		; check if motor is on
	jnz	L_ED99

	and	byte [0x003f], 0xf0		; clear motor status
	or	byte [0x003f], al		; set motor status to on

	sti

	mov	al, 0x10
	shl	al, cl
	or	al, dl
	or	al, 0x0c

	push	dx
	mov	dx, 0x03f2
	out	dx, al				; turn on motor
	pop	dx

	test	byte [0x003f], 0x80
	jz	L_ED99

	mov	bx, 2 * 0x000a			; motor start time
	call	dpt_get_param			; EE6C
	or	ah, ah

L_ED8F:
	jz	L_ED99

	sub	cx, cx

L_ED93:
	loop	L_ED93

	dec	ah
	jmp	short L_ED8F

L_ED99:
						; motor is on

	sti

	pop	cx

	call	L_EE7D				; seek track

	pop	ax

	mov	bh, ah
	mov	dh, 0x00
	jc	L_EDF0

db 0xBE, 0xF0, 0xED                     ; EDA5 mov si,0xedf0
db 0x90                                 ; EDA8 nop
db 0x56                                 ; EDA9 push si

	call	L_EE41				; send command to fdc

db 0x8A, 0x66, 0x01                     ; EDAD mov ah,[bp+0x1]
db 0xD0, 0xE4                           ; EDB0 shl ah,1
db 0xD0, 0xE4                           ; EDB2 shl ah,1
db 0x80, 0xE4, 0x04                     ; EDB4 and ah,0x4
db 0x0A, 0xE2                           ; EDB7 or ah,dl
db 0xE8, 0x85, 0x00                     ; EDB9 call 0xee41

	cmp	bh, 0x4d			; check if format
	jne	L_EDC4
	jmp	L_ED26

L_EDC4:
db 0x8A, 0xE5                           ; EDC4 mov ah,ch
db 0xE8, 0x78, 0x00                     ; EDC6 call 0xee41
db 0x8A, 0x66, 0x01                     ; EDC9 mov ah,[bp+0x1]
db 0xE8, 0x72, 0x00                     ; EDCC call 0xee41
db 0x8A, 0xE1                           ; EDCF mov ah,cl
db 0xE8, 0x6D, 0x00                     ; EDD1 call 0xee41
db 0xBB, 0x07, 0x00                     ; EDD4 mov bx,0x7
db 0xE8, 0x92, 0x00                     ; EDD7 call 0xee6c
db 0xBB, 0x09, 0x00                     ; EDDA mov bx,0x9
db 0xE8, 0x8C, 0x00                     ; EDDD call 0xee6c
db 0xBB, 0x0B, 0x00                     ; EDE0 mov bx,0xb
db 0xE8, 0x86, 0x00                     ; EDE3 call 0xee6c
db 0xBB, 0x0D, 0x00                     ; EDE6 mov bx,0xd

L_EDE9:
	call	dpt_get_param

	pop	si

	call	L_EF33				; wait for floppy interrupt

L_EDF0:
	jc	L_EE37

	call	fdc_read_result			; read result
	jc	L_EE36

	cld
	mov	si, 0x0042			; fdc result

	lodsb					; ST0
	and	al, 0xc0			; interrupt code
	jz	L_EE3B				; normal termination

	cmp	al, 0x40			; abnormal termination
	jnz	L_EE2D

	lodsb					; ST1

	shl	al, 1				; end of cylinder
	mov	ah, 0x04
	jc	L_EE2F

	shl	al, 1
	shl	al, 1				; data error
	mov	ah, 0x10
	jc	L_EE2F

	shl	al, 1				; over run
	mov	ah, 0x08
	jc	L_EE2F

	shl	al, 1
	shl	al, 1				; no data
	mov	ah, 0x04
	jc	L_EE2F

	shl	al, 1				; not writable
	mov	ah, 0x03
	jc	L_EE2F

	shl	al, 1				; missing address mark
	mov	ah, 0x02
	jc	L_EE2F

L_EE2D:
	mov	ah, 0x20

L_EE2F:
	or	[0x41], ah

	call	L_EFAE

L_EE36:
	ret

L_EE37:
db 0xE8, 0x2F, 0x01                     ; EE37 call 0xef69
db 0xC3                                 ; EE3A ret

L_EE3B:
db 0xE8, 0x70, 0x01                     ; EE3B call 0xefae
db 0x32, 0xE4                           ; EE3E xor ah,ah
db 0xC3                                 ; EE40 ret


;-----------------------------------------------------------------------------
; Send a byte to the FDC
; inp: AH = value
; out: CF set on error
;-----------------------------------------------------------------------------

fdc_send_byte:					; EE41
L_EE41:
	push	dx
	push	cx

	mov	dx, 0x03f4			; status register
	xor	cx, cx

L_EE48:
	in	al, dx
	test	al, 0x40			; direction
	jz	L_EE59
	loop	L_EE48

L_EE4F:
	or	byte [0x0041], 0x80		; timeout

	pop	cx
	pop	dx
	pop	ax
	stc
	ret

L_EE59:
	xor	cx, cx

L_EE5B:
	in	al, dx
	test	al, 0x80			; ready
	jnz	L_EE64
	loop	L_EE5B

	jmp	short L_EE4F			; timeout

L_EE64:
	mov	al, ah
	mov	dl, 0xf5			; data register

	out	dx, al

	pop	cx
	pop	dx
	ret


;-----------------------------------------------------------------------------
; Get a DPT entry
;
; inp: BX[15..1] = DPT offset
;      BX[0] = send to FDC flag
; out: AH = the DPT entry
;
; If BX[0] is set then send the byte to the FDC in addition to returning it
; in AH.
;-----------------------------------------------------------------------------

dpt_get_param:					; EE6C
L_EE6C:
	push	ds

	sub	ax, ax
	mov	ds, ax

	lds	si, [4 * 0x1e]			; get pointer to dpt

	shr	bx, 1
	mov	ah, [bx + si]			; get parameter

	pop	ds
	jc	L_EE41
	ret


; seek track
L_EE7D:
	mov	al, 0x01

	push	cx
	mov	cl, dl
	rol	al, cl
	pop	cx

	test	[0x003e], al			; check recalibrate status
	jnz	L_EE9E

	or	[0x003e], al			; set recalibrate status

	mov	ah, 0x07			; fdc recalibrate
	call	L_EE41				; send to fdc

	mov	ah, dl
	call	L_EE41				; send to fdc

	call	L_EF12				; wait for command completion
	jc	L_EEC7

L_EE9E:
	mov	ah, 0x0f			; fdc seek
	call	fdc_send_byte

	mov	ah, dl
	call	fdc_send_byte

	mov	ah, ch
	call	fdc_send_byte

	call	L_EF12				; wait for command completion

	pushf

	mov	bx, 0x0012
	call	dpt_get_param

	push	cx

L_EEB8:
	mov	cx, 0x0226
	or	ah, ah
	jz	L_EEC5

L_EEBF:
	loop	L_EEBF				; delay

	dec	ah
	jmp	short L_EEB8

L_EEC5:
	pop	cx
	popf

L_EEC7:
	ret


; setup dma channel 2 with mode al, address ES:BX, sector count dh
fdc_setup_dma:					; EEC8
L_EEC8:
	push	cx
	cli

	out	0x000c, al			; dma mode
	push	ax
	pop	ax
	out	0x000b, al			; clear flip-flop

	mov	ax, es
	mov	cl, 0x04
	rol	ax, cl
	mov	ch, al				; dma page
	and	al, 0xf0
	add	ax, bx
	jnc	L_EEE0

	inc	ch

L_EEE0:
						; linear address in CH:AX

	push	ax
	out	0x0004, al			; dma address low byte
	mov	al, ah
	out	0x0004, al			; dma address high byte

	mov	al, ch
	and	al, 0x0f
	out	0x81, al			; dma page

	mov	ah, dh				; sector count
	sub	al, al
	shr	ax, 1				; n << 7

	push	ax
	mov	bx, 2 * 0x0003			; bytes per sector
	call	L_EE6C
	mov	cl, ah
	pop	ax

	shl	ax, cl				; byte count in AX
	dec	ax

	push	ax
	out	0x0005, al			; dma channel 2 word count
	mov	al, ah
	out	0x0005, al

	sti

	pop	cx
	pop	ax

	add	ax, cx
	pop	cx

	mov	al, 0x02
	out	0x000a, al

	ret


; sense interrupt status
L_EF12:
	call	L_EF33				; wait for floppy interrupt
	jc	L_EF2B

	mov	ah, 0x08			; sense interrupt status
	call	L_EE41				; send ah to fdc

	call	fdc_read_result
	jc	L_EF2B

	mov	al, [0x0042]			; fdc st0
	and	al, 0x60
	cmp	al, 0x60			; abnormal termination
	je	L_EF2C

	clc

L_EF2B:
	ret

L_EF2C:
	or	byte [0x0041], 0x40
	stc
	ret


; wait for floppy interrupt
L_EF33:
	sti
	push	bx
	push	cx

	mov	bl, 0x02
	xor	cx, cx

L_EF3A:
	test	byte [0x003e], 0x80		; check for floppy interrupt
	jnz	L_EF4D
	loop	L_EF3A

	dec	bl
	jnz	L_EF3A

	or	byte [0x0041], 0x80		; timeout
	stc

L_EF4D:
	pushf
	and	byte [0x003e], 0x7f		; clear floppy interrupt flag
	popf

	pop	cx
	pop	bx
	ret


;-----------------------------------------------------------------------------
; int 0e - floppy
;-----------------------------------------------------------------------------

int_0e:						; EF57
	sti
	push	ds
	push	ax

	call	set_bios_ds

	or	byte [0x003e], 0x80

	mov	al, 0x20
	out	0x20, al			; end of interrupt

	pop	ax
	pop	ds
	iret


;-----------------------------------------------------------------------------
; Read all result bytes from the FDC and store them at 0040:0042
;
; out: CF set on error
;      DI points behind last result byte
;-----------------------------------------------------------------------------

fdc_read_result:				; EF69
L_EF69:
	cld
	mov	di, 0x0042

	push	cx
	push	dx
	push	bx

	mov	bl, 0x07			; maximum number of bytes

L_EF72:
	xor	cx, cx
	mov	dx, 0x03f4			; FDC MSR

L_EF77:
	in	al, dx
	test	al, 0x80			; data request
	jnz	L_EF88
	loop	L_EF77

	or	byte [0x0041], 0x80		; timeout

L_EF83:
	stc

	pop	bx
	pop	dx
	pop	cx
	ret

L_EF88:
	in	al, dx
	test	al, 0x40			; data direction
	jnz	L_EF94

L_EF8D:
	or	byte [0x0041], 0x20		; wrong direction
	jmp	short L_EF83

L_EF94:
	inc	dx				; FDC data register
	in	al, dx				; read result byte
	mov	[di], al
	inc	di

	mov	cx, 0x000a
.L_EF9C:
	loop	.L_EF9C				; delay

	dec	dx				; FDC MSR
	in	al, dx
	test	al, 0x10			; busy
	jz	L_EFAA

	dec	bl
	jnz	L_EF72

	jmp	short L_EF8D

L_EFAA:
	pop	bx
	pop	dx
	pop	cx
	ret


L_EFAE:
	mov	al, [0x0045]			; cylinder
	cmp	al, ch
	mov	al, [0x0047]			; sector
	je	L_EFC2

	mov	bx, 0x0008
	call	dpt_get_param

	mov	al, ah
	inc	al

L_EFC2:
	sub	al, cl
	ret


	db	0xff, 0xff


;-----------------------------------------------------------------------------
;* int 1e - diskette parameter table
;-----------------------------------------------------------------------------

int_1e:						; EFC7
	db	0xCF				; step rate
	db	0x02				; dma mode
	db	0x25				; motor off delay
	db	0x02				; byte per sector
	db	0x08				; sectors per track
	db	0x2a				; gap 3 writing
	db	0xff
	db	0x50				; gap 3 formatting
	db	0xf6				; fill byte
	db	0x19				; head settle time
	db	0x04				; motor start time


;-----------------------------------------------------------------------------
; int 17 handler
;-----------------------------------------------------------------------------

int_17:						; EFD2
	sti
	push	ds
	push	dx
	push	si
	push	cx
	push	bx

	call	set_bios_ds

	mov	si, dx
	mov	bl, [si + 0x0078]		; port timeout value

	shl	si, 1
	mov	dx, [si + 0x0008]		; base address

	or	dx, dx
	jz	int_17_done

	or	ah, ah
	jz	int_17_00

	dec	ah
	jz	int_17_01

	dec	ah
	jz	int_17_02

int_17_done:					; EFF5
	pop	bx
	pop	cx
	pop	si
	pop	dx
	pop	ds
	iret


;-----------------------------------------------------------------------------
; int 17 func 00 - print character
; inp: AL = character
;      DX = port
;-----------------------------------------------------------------------------
int_17_00:					; EFFB
	push	ax

	out	dx, al				; write data register

	inc	dx

L_EFFE:
	sub	cx, cx

L_F000:
	in	al, dx				; read status register
	mov	ah, al
	test	al, 0x80			; busy
	jnz	L_F015
	loop	L_F000

	dec	bl
	jnz	L_EFFE

	or	ah, 0x01			; timeout
	and	ah, 0xf9
	jmp	short L_F028

L_F015:
	mov	al, 0x0d

	inc	dx
	out	dx, al				; strobe

	mov	al, 0x0c
	out	dx, al				; no strobe

	pop	ax


;-----------------------------------------------------------------------------
; int 17 func 02 - get port status
; inp: DX = port
;-----------------------------------------------------------------------------
int_17_02:					; F01D
	push	ax

L_F01E:
	mov	dx, [si + 0x0008]		; port base address

	inc	dx
	in	al, dx				; read status register

	mov	ah, al
	and	ah, 0xf8

L_F028:
	pop	dx

	mov	al, dl
	xor	ah, 0x48

	jmp	short int_17_done


;-----------------------------------------------------------------------------
; int 17 func 01 - initialize port
; inp: DX = port
;-----------------------------------------------------------------------------
int_17_01:					; F030
	push	ax

	inc	dx
	inc	dx

	mov	al, 0x08
	out	dx, al				; write control register

	mov	ax, 1000

L_F039:
	dec	ax
	jnz	L_F039

	mov	al, 0x0c
	out	dx, al

	jmp	short L_F01E


D_F041:
	dw	L_E162

	db	0xff, 0xff


int_10_func:
	dw	int_10_00			; F0FC
	dw	int_10_01
	dw	int_10_02
	dw	int_10_03
	dw	int_10_04
	dw	int_10_05
	dw	int_10_06
	dw	int_10_07			; F338
	dw	int_10_08			; F374
	dw	int_10_09
	dw	int_10_0a			; F3EC
	dw	int_10_0b			; F24E
	dw	int_10_0c			; F42F
	dw	int_10_0d			; F41E
	dw	int_10_0e
	dw	int_10_0f			; F274


;-----------------------------------------------------------------------------
; int 10 handler
;-----------------------------------------------------------------------------

int_10:						; F065
	sti
	cld

	push	es
	push	ds
	push	dx
	push	cx
	push	bx
	push	si
	push	di
	push	ax

	mov	al, ah
	xor	ah, ah
	shl	ax, 1

	mov	si, ax
	cmp	ax, 2 * 0x10
	jb	L_F080

	pop	ax
	jmp	int_10_done

L_F080:
	call	set_bios_ds

	mov	ax, 0xb800
	mov	di, [0x0010]			; equipment word
	and	di, 0x30			; initial video mode
	cmp	di, byte 0x30			; MDA
	jne	L_F095

	mov	ah, 0xb0

L_F095:
	mov	es, ax				; ES points to video memory

	pop	ax

	mov	ah, [0x0049]			; current video mode

	jmp	near [cs:si + int_10_func]


	db	0xff, 0xff, 0xff


;-----------------------------------------------------------------------------
; int 1d - video parameter table
;-----------------------------------------------------------------------------

int_1d:						; F0A4
	db	0x38, 0x28, 0x2D, 0x0A		; crtc regs modes 00 and 01
	db	0x1F, 0x06, 0x19, 0x1C
	db	0x02, 0x07, 0x06, 0x07
	db	0x00, 0x00, 0x00, 0x00

	db	0x71, 0x50, 0x5A, 0x0A		; crtc regs modes 02 and 03
	db	0x1F, 0x06, 0x19, 0x1C
	db	0x02, 0x07, 0x06, 0x07
	db	0x00, 0x00, 0x00, 0x00

	db	0x38, 0x28, 0x2D, 0x0A		; crtc regs modes 04 and 05
	db	0x7F, 0x06, 0x64, 0x70
	db	0x02, 0x01, 0x06, 0x07
	db	0x00, 0x00, 0x00, 0x00

	db	0x61, 0x50, 0x52, 0x0F		; crtc regs modes 06 and 07
	db	0x19, 0x06, 0x19, 0x19
	db	0x02, 0x0D, 0x0B, 0x0C
	db	0x00, 0x00, 0x00, 0x00

; page sizes
L_F0E4:
	dw	2048				; mode 0 and 1
	dw	4096				; mode 2 and 3
	dw	16384				; mode 4 and 5
	dw	16384				; mode 6 and 7

; number of columns for video modes 0 - 7
L_F0EC:
	db	40
	db	40
	db	80
	db	80
	db	40
	db	40
	db	80
	db	80

; crtc mode control registers for video modes 0 - 7
L_F0F4:
	db	0x2C
	db	0x28
	db	0x2D
	db	0x29
	db	0x2A
	db	0x2E
	db	0x1E
	db	0x29


;-----------------------------------------------------------------------------
; int 10 func 00 - set video mode
; inp: AL = video mode (0 - 7)
;-----------------------------------------------------------------------------
int_10_00:					; F0FC
	mov	dx, 0x03d4
	mov	bl, 0

	cmp	di, byte 0x30			; check if MDA
	jne	L_F10C

	mov	al, 7				; force mode 7
	mov	dl, 0xb4			; 0x03b4
	inc	bl

L_F10C:
	mov	ah, al

	mov	[0x0049], al			; current video mode
	mov	[0x0063], dx			; crtc base

	push	ds
	push	ax

	push	dx
	add	dx, byte 4
	mov	al, bl
	out	dx, al
	pop	dx

	sub	ax, ax
	mov	ds, ax
	lds	bx, [0x0074]			; int 1d address

	pop	ax

	mov	cx, 16
	cmp	ah, 2
	jb	L_F140

	add	bx, cx
	cmp	ah, 4
	jb	L_F140

	add	bx, cx
	cmp	ah, 7
	jb	L_F140

	add	bx, cx

L_F140:
	push	ax

	xor	ah, ah

L_F143:
	mov	al, ah
	out	dx, al

	inc	dx
	inc	ah

	mov	al, [bx]
						; db 0x8a, 0x07
	out	dx, al

	inc	bx
	dec	dx
	loop	L_F143

	pop	ax
	pop	ds

	xor	di, di
	mov	[0x004e], di			; current page offset

	mov	[0x0062], byte 0		; current page

	mov	cx, 0x2000
	cmp	ah, 4
	jb	L_F170

	cmp	ah, 7
	je	L_F16E

	xor	ax, ax
	jmp	short L_F173

L_F16E:
	mov	ch, 0x08

L_F170:
	mov	ax, 0x0720

L_F173:
	rep	stosw				; clear page

	mov	[0x0060], word 0x0607		; cursor size

	mov	al, [0x0049]			; video mode
	xor	ah, ah
	mov	si, ax

	mov	dx, [0x0063]			; crtc base
	add	dx, byte 4
	mov	al, [cs:si + L_F0F4]
	out	dx, al
	mov	[0x0065], al			; crtc mode control register

	mov	al, [cs:si + L_F0EC]
	xor	ah, ah
	mov	[0x004a], ax			; number of columns

	and	si, 0x0e
	mov	cx, [cs:si + L_F0E4]
	mov	[0x004c], cx			; page size

	mov	cx, 8
	mov	di, 0x0050			; cursor position
	push	ds
	pop	es
	xor	ax, ax
	rep	stosw				; clear cursor positions for all pages

	inc	dx
	mov	al, 0x30
	cmp	[0x0049], byte 6
	jne	L_F1C1

	mov	al, 0x3f

L_F1C1:
	out	dx, al
	mov	[0x0066], al			; palette mask

int_10_done:					; F1C5
	pop	di
	pop	si
	pop	bx

int_10_done1:					; F1C8
	pop	cx
	pop	dx
	pop	ds
	pop	es
	iret


;-----------------------------------------------------------------------------
; int 10 func 01 - set cursor size
; inp: CH = top scan line
;      CL = bottom scan line
;-----------------------------------------------------------------------------
int_10_01:					; F1CD
	mov	ah, 0x0a			; crtc register
	mov	[0x60], cx			; cursor size
	call	L_F1D8
	jmp	short int_10_done

; write CH -> crtc[AH], CL -> crtc[AH + 1]
L_F1D8:
	mov	dx, [0x63]			; crtc base
	mov	al, ah
	out	dx, al

	inc	dx
	mov	al, ch
	out	dx, al

	dec	dx
	mov	al, ah
	inc	al
	out	dx, al

	inc	dx
	mov	al, cl
	out	dx, al

	ret


;-----------------------------------------------------------------------------
; int 10 func 02 - set cursor position
; inp: BH = page
;      DH = row
;      DL = column
;-----------------------------------------------------------------------------
int_10_02:					; F1EE
	mov	cl, bh
	xor	ch, ch				; page number in CX
	shl	cx, 1
	mov	si, cx
	mov	[si + 0x50], dx			; store cursor position

	cmp	[0x62], bh			; check if current page
	jne	L_F204

	mov	ax, dx
	call	L_F206				; update crtc

L_F204:
	jmp	short int_10_done


; set cursor position AX
L_F206:
	call	L_F285				; convert row/column to offset

	mov	cx, ax
	add	cx, [0x004e]			; current page offset
	sar	cx, 1

	mov	ah, 0x0e			; crtc register
	call	L_F1D8				; write crtc

	ret


;-----------------------------------------------------------------------------
; int 10 func 05 - set current page
; inp: AL = page
;-----------------------------------------------------------------------------
int_10_05:					; F217
	mov	[0x0062], al			; current page

	mov	cx, [0x004c]			; page size

	cbw
	push	ax

	mul	cx
	mov	[0x004e], ax			; page offset

	mov	cx, ax
	sar	cx, 1
	mov	ah, 0x0c			; crtc register
	call	L_F1D8

	pop	bx

	shl	bx, 1
	mov	ax, [bx + 0x0050]		; cursor position

	call	L_F206				; set cursor position

	jmp	short int_10_done


;-----------------------------------------------------------------------------
; int 10 func 03 - get cursor position and size
; inp: BH = page number
; out: DH = row
;      DL = column
;      CH = top scan line
;      CL = bottom scan line
;-----------------------------------------------------------------------------
int_10_03:					; F239
	mov	bl, bh
	xor	bh, bh
	shl	bx, 1
	mov	dx, [bx + 0x0050]		; cursor position
	mov	cx, [0x0060]			; cursor size

	pop	di
	pop	si
	pop	bx
	pop	ax
	pop	ax
	pop	ds
	pop	es
	iret


;-----------------------------------------------------------------------------
; int 10 func 0b - set background color and color palette
; inp: BH = select background color (0) or color palette (1)
;      BL = background color or color palette
;-----------------------------------------------------------------------------
int_10_0b:					; F24E
	mov	dx, [0x0063]			; crtc base
	add	dx, byte 5

	mov	al, [0x0066]			; color selection register

	or	bh, bh
	jnz	int_10_0b_01

int_10_0b_00:
	and	al, 0xe0			; clear background color
	and	bl, 0x1f			; mask background color
	or	al, bl

L_F263:
	out	dx, al

	mov	[0x0066], al

	jmp	int_10_done

int_10_0b_01:					; F26A
	and	al, 0xdf			; select palette 0
	shr	bl, 1
	jnc	L_F263

	or	al, 0x20			; select palette 1
	jmp	short L_F263


;-----------------------------------------------------------------------------
; int 10 func 0f - get video mode
; out: AH = column count
;      AL = video mode
;      BH = current page
;-----------------------------------------------------------------------------
int_10_0f:					; F274
	mov	ah, [0x004a]			; number of columns
	mov	al, [0x0049]			; video mode
	mov	bh, [0x0062]			; current page

	pop	di
	pop	si
	pop	cx
	jmp	int_10_done1


; convert row/column to offset in video memory
L_F285:
	push	bx
	mov	bx, ax
	mov	al, ah				; row in AL
	mul	byte [0x4a]			; number of columns

	xor	bh, bh
	add	ax, bx				; add column

	shl	ax, 1

	pop	bx
	ret


;-----------------------------------------------------------------------------
; int 10 func 06 - scroll up
; inp: AL = line count (0 to clear)
;      BH = attribute for new lines
;      CH = row upper left corner
;      CL = column upper left corner
;      DH = row lower right corner
;      DL = column lower right corner
;-----------------------------------------------------------------------------
int_10_06:					; F296
	mov	bl, al

	cmp	ah, 4
	jb	L_F2A5

	cmp	ah, 7
	je	L_F2A5

	jmp	L_F495

; cga/mda text modes
L_F2A5:
	push	bx
	mov	ax, cx

	call	L_F2E2
	jz	L_F2DE
db 0x03, 0xF0                           ; F2AD add si,ax
db 0x8A, 0xE6                           ; F2AF mov ah,dh
db 0x2A, 0xE3                           ; F2B1 sub ah,bl
L_F2B3:
db 0xE8, 0x72, 0x00                     ; F2B3 call 0xf328
db 0x03, 0xF5                           ; F2B6 add si,bp
db 0x03, 0xFD                           ; F2B8 add di,bp
db 0xFE, 0xCC                           ; F2BA dec ah
db 0x75, 0xF5                           ; F2BC jnz 0xf2b3
L_F2BE:
db 0x58                                 ; F2BE pop ax
db 0xB0, 0x20                           ; F2BF mov al,0x20
L_F2C1:
db 0xE8, 0x6D, 0x00                     ; F2C1 call 0xf331
db 0x03, 0xFD                           ; F2C4 add di,bp
db 0xFE, 0xCB                           ; F2C6 dec bl
db 0x75, 0xF7                           ; F2C8 jnz 0xf2c1
L_F2CA:
db 0xE8, 0x71, 0x0C                     ; F2CA call 0xff3e
db 0x80, 0x3E, 0x49, 0x00, 0x07         ; F2CD cmp byte [0x49],0x7
db 0x74, 0x07                           ; F2D2 jz 0xf2db
db 0xA0, 0x65, 0x00                     ; F2D4 mov al,[0x65]
db 0xBA, 0xD8, 0x03                     ; F2D7 mov dx,0x3d8
db 0xEE                                 ; F2DA out dx,al
L_F2DB:
db 0xE9, 0xE7, 0xFE                     ; F2DB jmp 0xf1c5
L_F2DE:
db 0x8A, 0xDE                           ; F2DE mov bl,dh
db 0xEB, 0xDC                           ; F2E0 jmp short 0xf2be

L_F2E2:
	cmp	byte [0x0049], 0x02
%ifdef PATCH
	; #### patch #### (don't wait for retrace)
	jmp	short L_F301
%else
	jb	L_F301
%endif

	cmp	byte [0x0049], 0x03
	ja	L_F301

	push	dx
	mov	dx, 0x03da
	push	ax

L_F2F5:
	in	al, dx
	test	al, 0x08			; test if vertical retrace
	jz	L_F2F5

	mov	al, 0x25			; turn off video signal
	mov	dl, 0xd8
	out	dx, al

	pop	ax
	pop	dx

L_F301:
	call	L_F285
db 0x03, 0x06, 0x4E, 0x00               ; F304 add ax,[0x4e]
db 0x8B, 0xF8                           ; F308 mov di,ax
db 0x8B, 0xF0                           ; F30A mov si,ax
db 0x2B, 0xD1                           ; F30C sub dx,cx
db 0xFE, 0xC6                           ; F30E inc dh
db 0xFE, 0xC2                           ; F310 inc dl
db 0x32, 0xED                           ; F312 xor ch,ch
db 0x8B, 0x2E, 0x4A, 0x00               ; F314 mov bp,[0x4a]
db 0x03, 0xED                           ; F318 add bp,bp
db 0x8A, 0xC3                           ; F31A mov al,bl
db 0xF6, 0x26, 0x4A, 0x00               ; F31C mul byte [0x4a]
db 0x03, 0xC0                           ; F320 add ax,ax
db 0x06                                 ; F322 push es
db 0x1F                                 ; F323 pop ds
db 0x80, 0xFB, 0x00                     ; F324 cmp bl,0x0
db 0xC3                                 ; F327 ret

L_F328:
	mov	cl, dl
	push	si
	push	di
	rep	movsw
	pop	di
	pop	si
	ret

L_F331:
	mov	cl, dl
	push	di
	rep	stosw
	pop	di
	ret


;-----------------------------------------------------------------------------
; int 10 func 07 - scroll down
; inp: AL = line count
;      BH = attribute for new lines
;      CH = upper left corner row
;      CL = upper left corner column
;      DH = lower right corner row
;      DL = lower right corner column
;-----------------------------------------------------------------------------
int_10_07:					; F338
	std
	mov	bl, al

	cmp	ah, 0x04
	jb	L_F348

	cmp	ah, 0x07
	je	L_F348

	jmp	L_F4EE

L_F348:
	push	bx

	mov	ax, dx
	call	L_F2E2
	jz	L_F370

	sub	si, ax
	mov	ah, dh
	sub	ah, bl

L_F356:
	call	L_F328

	sub	si, bp
	sub	di, bp
	dec	ah
	jnz	L_F356

L_F361:
	pop	ax
	mov	al, 0x20

L_F364:
	call	L_F331

	sub	di, bp
	dec	bl
	jnz	L_F364

	jmp	L_F2CA

L_F370:
	mov	bl, dh
	jmp	short L_F361


;-----------------------------------------------------------------------------
; int 10 func 08 - get character and attribute
; inp: BH = page
; out: AL = character
;      AH = attribute
;-----------------------------------------------------------------------------

int_10_08:					; F374
	cmp	ah, 4
	jb	L_F381

	cmp	ah, 7
	je	L_F381

	jmp	L_F629

L_F381:
	call	L_F39E				; get character offset

	mov	si, bx

	mov	dx, [0x0063]			; crtc base
	add	dx, byte 6			; crtc status

	push	es
	pop	ds

L_F38F:
%ifdef PATCH
	; #### patch #### (don't wait for horizontal retrace)
	jmp	short L_F39A
	nop
%else
	in	al, dx
	test	al, 0x01			; check if horizontal sync
%endif
	jnz	L_F38F				; wait for end of sync

	cli

L_F395:
	in	al, dx
	test	al, 0x01
	jz	L_F395				; wait for sync

L_F39A:
	lodsw					; read character and attribute

	jmp	int_10_done


; get character offset
L_F39E:
	mov	cl, bh
	xor	ch, ch
	mov	si, cx
	shl	si, 1
	mov	ax, [si + 0x0050]		; get cursor position

	xor	bx, bx
	jcxz	L_F3B3

L_F3AD:
	add	bx, [0x004c]			; page size
	loop	L_F3AD

L_F3B3:
	call	L_F285				; convert row/column to offset

	add	bx, ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 09 - write character and attribute
; inp: AL = character
;      BH = page
;      BL = attribute
;      CX = count
;-----------------------------------------------------------------------------

int_10_09:					; F3B9
	cmp	ah, 4
	jb	L_F3C6

	cmp	ah, 7
	je	L_F3C6

	jmp	L_F578

L_F3C6:
	mov	ah, bl

	push	ax
	push	cx

	call	L_F39E				; get character offset

	mov	di, bx

	pop	cx
	pop	bx

L_F3D1:
%ifdef PATCH
	; #### patch #### (don't wait for horizontal retrace)
	jmp	short L_F3E3
	nop
	nop
%else
	mov	dx, [0x0063]			; crtc base
%endif
	add	dx, byte 6			; crtc status register

L_F3D8:
	in	al, dx
	test	al, 0x01			; check if horizontal sync
	jnz	L_F3D8				; wait for end of sync

	cli

L_F3DE:
	in	al, dx
	test	al, 0x01
	jz	L_F3DE				; wait for start of sync

L_F3E3:
	mov	ax, bx
	stosw					; write character and attribute

	sti

	loop	L_F3D1

	jmp	int_10_done


;-----------------------------------------------------------------------------
; int 10 func 0a - write character without attribute
; inp: AL = character
;      BH = page
;      CX = count
;-----------------------------------------------------------------------------

int_10_0a:					; F3EC
	cmp	ah, 4
	jb	L_F3F9

	cmp	ah, 7
	je	L_F3F9

	jmp	L_F578

L_F3F9:
	push	ax
	push	cx

	call	L_F39E				; get character offset

	mov	di, bx

	pop	cx
	pop	bx

L_F402:
%ifdef PATCH
	; #### patch #### (don't wait for horizontal retrace)
	jmp	short L_F416
	nop
	nop
%else
	mov	dx, [0x0063]			; crtc base
%endif
	add	dx, byte 6			; crtc status register

L_F409:
	in	al, dx				; read status register
	test	al, 0x01			; check if horizontal sync
	jnz	L_F409				; wait for end of horizontal sync

	cli

L_F40F:
	in	al, dx
	test	al, 0x01
	jz	L_F40F				; wait for horizontal sync

L_F416:
	mov	al, bl
	stosb					; write character

	sti

	inc	di				; skip attribute

	loop	L_F402

	jmp	int_10_done


;-----------------------------------------------------------------------------
; int 10 func 0d - get pixel
; inp: BH = page
;      CX = x
;      DX = y
; out: AL = color
;-----------------------------------------------------------------------------

int_10_0d:					; F41E
	call	L_F452

	mov	al, [es:si]
	and	al, ah				; mask pixel
	shl	al, cl				; move value to high bits
	mov	cl, dh
	rol	al, cl				; and back to low bits
	jmp	int_10_done


;-----------------------------------------------------------------------------
; int 10 func 0c - set pixel
; inp: AL = color
;      BH = page
;      CX = x
;      DX = y
;-----------------------------------------------------------------------------

int_10_0c:					; F42F
	push	ax
	push	ax

	call	L_F452

	shr	al, cl
	and	al, ah

	mov	cl, [es:si]

	pop	bx

	test	bl, 0x80			; check if xor
	jnz	L_F44E

	not	ah
	and	cl, ah
	or	al, cl

L_F447:
	mov	[es:si], al

	pop	ax

	jmp	int_10_done

L_F44E:
	xor	al, cl
	jmp	short L_F447


; convert x/y to offset/index/mask
L_F452:
	push	bx
	push	ax

	mov	al, 0x28			; bytes per line

	push	dx
	and	dl, 0xfe
	mul	dl				; get line offset
	pop	dx

	test	dl, 0x01			; check if odd line
	jz	L_F465

	add	ax, 0x2000

L_F465:
	mov	si, ax
	pop	ax

	mov	dx, cx				; column
	mov	bx, 0x02c0
	mov	cx, 0x0302

	cmp	byte [0x0049], 0x06
	jb	L_F47D

	mov	bx, 0x0180
	mov	cx, 0x0703

L_F47D:
	and	ch, dl				; get pixel index in byte

	shr	dx, cl				; get byte index
	add	si, dx

	mov	dh, bh				; bits per pixel
	sub	cl, cl

L_F487:
	ror	al, 1
	add	cl, ch
	dec	bh
	jnz	L_F487

	mov	ah, bl				; bit mask
	shr	ah, cl				; align mask

	pop	bx
	ret


; scroll up in graphic modes
L_F495:
db 0x8A, 0xD8                           ; F495 mov bl,al
db 0x8B, 0xC1                           ; F497 mov ax,cx
db 0xE8, 0x69, 0x02                     ; F499 call 0xf705
db 0x8B, 0xF8                           ; F49C mov di,ax
db 0x2B, 0xD1                           ; F49E sub dx,cx
db 0x81, 0xC2, 0x01, 0x01               ; F4A0 add dx,0x101
db 0xD0, 0xE6                           ; F4A4 shl dh,1
db 0xD0, 0xE6                           ; F4A6 shl dh,1
db 0x80, 0x3E, 0x49, 0x00, 0x06         ; F4A8 cmp byte [0x49],0x6
db 0x73, 0x04                           ; F4AD jnc 0xf4b3
db 0xD0, 0xE2                           ; F4AF shl dl,1
db 0xD1, 0xE7                           ; F4B1 shl di,1
db 0x06                                 ; F4B3 push es
db 0x1F                                 ; F4B4 pop ds
db 0x2A, 0xED                           ; F4B5 sub ch,ch
db 0xD0, 0xE3                           ; F4B7 shl bl,1
db 0xD0, 0xE3                           ; F4B9 shl bl,1
db 0x74, 0x2D                           ; F4BB jz 0xf4ea
db 0x8A, 0xC3                           ; F4BD mov al,bl
db 0xB4, 0x50                           ; F4BF mov ah,0x50
db 0xF6, 0xE4                           ; F4C1 mul ah
db 0x8B, 0xF7                           ; F4C3 mov si,di
db 0x03, 0xF0                           ; F4C5 add si,ax
db 0x8A, 0xE6                           ; F4C7 mov ah,dh
db 0x2A, 0xE3                           ; F4C9 sub ah,bl
db 0xE8, 0x80, 0x00                     ; F4CB call 0xf54e
db 0x81, 0xEE, 0xB0, 0x1F               ; F4CE sub si,0x1fb0
db 0x81, 0xEF, 0xB0, 0x1F               ; F4D2 sub di,0x1fb0
db 0xFE, 0xCC                           ; F4D6 dec ah
db 0x75, 0xF1                           ; F4D8 jnz 0xf4cb
db 0x8A, 0xC7                           ; F4DA mov al,bh
db 0xE8, 0x88, 0x00                     ; F4DC call 0xf567
db 0x81, 0xEF, 0xB0, 0x1F               ; F4DF sub di,0x1fb0
db 0xFE, 0xCB                           ; F4E3 dec bl
db 0x75, 0xF5                           ; F4E5 jnz 0xf4dc
db 0xE9, 0xDB, 0xFC                     ; F4E7 jmp 0xf1c5
db 0x8A, 0xDE                           ; F4EA mov bl,dh
db 0xEB, 0xEC                           ; F4EC jmp short 0xf4da

; scroll down in graphic modes
L_F4EE:
db 0xFD                                 ; F4EE std
db 0x8A, 0xD8                           ; F4EF mov bl,al
db 0x8B, 0xC2                           ; F4F1 mov ax,dx
db 0xE8, 0x0F, 0x02                     ; F4F3 call 0xf705
db 0x8B, 0xF8                           ; F4F6 mov di,ax
db 0x2B, 0xD1                           ; F4F8 sub dx,cx
db 0x81, 0xC2, 0x01, 0x01               ; F4FA add dx,0x101
db 0xD0, 0xE6                           ; F4FE shl dh,1
db 0xD0, 0xE6                           ; F500 shl dh,1
db 0x80, 0x3E, 0x49, 0x00, 0x06         ; F502 cmp byte [0x49],0x6
db 0x73, 0x05                           ; F507 jnc 0xf50e
db 0xD0, 0xE2                           ; F509 shl dl,1
db 0xD1, 0xE7                           ; F50B shl di,1
db 0x47                                 ; F50D inc di
db 0x06                                 ; F50E push es
db 0x1F                                 ; F50F pop ds
db 0x2A, 0xED                           ; F510 sub ch,ch
db 0x81, 0xC7, 0xF0, 0x00               ; F512 add di,0xf0
db 0xD0, 0xE3                           ; F516 shl bl,1
db 0xD0, 0xE3                           ; F518 shl bl,1
db 0x74, 0x2E                           ; F51A jz 0xf54a
db 0x8A, 0xC3                           ; F51C mov al,bl
db 0xB4, 0x50                           ; F51E mov ah,0x50
db 0xF6, 0xE4                           ; F520 mul ah
db 0x8B, 0xF7                           ; F522 mov si,di
db 0x2B, 0xF0                           ; F524 sub si,ax
db 0x8A, 0xE6                           ; F526 mov ah,dh
db 0x2A, 0xE3                           ; F528 sub ah,bl
db 0xE8, 0x21, 0x00                     ; F52A call 0xf54e
db 0x81, 0xEE, 0x50, 0x20               ; F52D sub si,0x2050
db 0x81, 0xEF, 0x50, 0x20               ; F531 sub di,0x2050
db 0xFE, 0xCC                           ; F535 dec ah
db 0x75, 0xF1                           ; F537 jnz 0xf52a
db 0x8A, 0xC7                           ; F539 mov al,bh
db 0xE8, 0x29, 0x00                     ; F53B call 0xf567
db 0x81, 0xEF, 0x50, 0x20               ; F53E sub di,0x2050
db 0xFE, 0xCB                           ; F542 dec bl
db 0x75, 0xF5                           ; F544 jnz 0xf53b
db 0xFC                                 ; F546 cld
db 0xE9, 0x7B, 0xFC                     ; F547 jmp 0xf1c5
db 0x8A, 0xDE                           ; F54A mov bl,dh
db 0xEB, 0xEB                           ; F54C jmp short 0xf539
db 0x8A, 0xCA                           ; F54E mov cl,dl
db 0x56                                 ; F550 push si
db 0x57                                 ; F551 push di
db 0xF3, 0xA4                           ; F552 rep movsb
db 0x5F                                 ; F554 pop di
db 0x5E                                 ; F555 pop si
db 0x81, 0xC6, 0x00, 0x20               ; F556 add si,0x2000
db 0x81, 0xC7, 0x00, 0x20               ; F55A add di,0x2000
db 0x56                                 ; F55E push si
db 0x57                                 ; F55F push di
db 0x8A, 0xCA                           ; F560 mov cl,dl
db 0xF3, 0xA4                           ; F562 rep movsb
db 0x5F                                 ; F564 pop di
db 0x5E                                 ; F565 pop si
db 0xC3                                 ; F566 ret
db 0x8A, 0xCA                           ; F567 mov cl,dl
db 0x57                                 ; F569 push di
db 0xF3, 0xAA                           ; F56A rep stosb
db 0x5F                                 ; F56C pop di
db 0x81, 0xC7, 0x00, 0x20               ; F56D add di,0x2000
db 0x57                                 ; F571 push di
db 0x8A, 0xCA                           ; F572 mov cl,dl
db 0xF3, 0xAA                           ; F574 rep stosb
db 0x5F                                 ; F576 pop di
db 0xC3                                 ; F577 ret

; print char in mode 4/5
L_F578:
	mov	ah, 0

	push	ax
	call	L_F702				; get cursor position in AX
	mov	di, ax
	pop	ax

	cmp	al, 128
	jae	L_F58B

	mov	si, L_FA6E
	push	cs

	jmp	short L_F59A

L_F58B:
	sub	al, 128
	push	ds
	sub	si, si
	mov	ds, si
	lds	si, [4 * 0x1f]			; int 1f address
	mov	dx, ds
	pop	ds

	push	dx

L_F59A:
						; font pointer in [SS:SP]:SI, char in AL

	shl	ax, 1
	shl	ax, 1
	shl	ax, 1
	add	si, ax				; SI points to character

	cmp	byte [0x0049], 6		; video mode

	pop	ds				; font segement

	jb	L_F5D6

						; mode 6
L_F5AA:
	push	di
	push	si

	mov	dh, 4

L_F5AE:
	lodsb
	test	bl, 0x80
	jnz	L_F5CA

	stosb

	lodsb
L_F5B6:
	mov	[es:di + 0x1fff], al

	add	di, byte 80 - 1

	dec	dh
	jnz	L_F5AE

	pop	si
	pop	di

	inc	di
	loop	L_F5AA

	jmp	int_10_done

L_F5CA:
	xor	al, [es:di]
	stosb

	lodsb
	xor	al, [es:di + 0x1fff]
	jmp	short L_F5B6


; mode 5
L_F5D6:
db 0x8A, 0xD3                           ; F5D6 mov dl,bl
db 0xD1, 0xE7                           ; F5D8 shl di,1
db 0xE8, 0xD1, 0x00                     ; F5DA call 0xf6ae
db 0x57                                 ; F5DD push di
db 0x56                                 ; F5DE push si
db 0xB6, 0x04                           ; F5DF mov dh,0x4
db 0xAC                                 ; F5E1 lodsb
db 0xE8, 0xDE, 0x00                     ; F5E2 call 0xf6c3
db 0x23, 0xC3                           ; F5E5 and ax,bx
db 0xF6, 0xC2, 0x80                     ; F5E7 test dl,0x80
db 0x74, 0x07                           ; F5EA jz 0xf5f3
db 0x26, 0x32, 0x25                     ; F5EC xor ah,[es:di]
db 0x26, 0x32, 0x45, 0x01               ; F5EF xor al,[es:di+0x1]
db 0x26, 0x88, 0x25                     ; F5F3 mov [es:di],ah
db 0x26, 0x88, 0x45, 0x01               ; F5F6 mov [es:di+0x1],al
db 0xAC                                 ; F5FA lodsb
db 0xE8, 0xC5, 0x00                     ; F5FB call 0xf6c3
db 0x23, 0xC3                           ; F5FE and ax,bx
db 0xF6, 0xC2, 0x80                     ; F600 test dl,0x80
db 0x74, 0x0A                           ; F603 jz 0xf60f
db 0x26, 0x32, 0xA5, 0x00, 0x20         ; F605 xor ah,[es:di+0x2000]
db 0x26, 0x32, 0x85, 0x01, 0x20         ; F60A xor al,[es:di+0x2001]
db 0x26, 0x88, 0xA5, 0x00, 0x20         ; F60F mov [es:di+0x2000],ah
db 0x26, 0x88, 0x85, 0x01, 0x20         ; F614 mov [es:di+0x2001],al
db 0x83, 0xC7, 0x50                     ; F619 add di,byte +0x50
db 0xFE, 0xCE                           ; F61C dec dh
db 0x75, 0xC1                           ; F61E jnz 0xf5e1
db 0x5E                                 ; F620 pop si
db 0x5F                                 ; F621 pop di
db 0x47                                 ; F622 inc di
db 0x47                                 ; F623 inc di
db 0xE2, 0xB7                           ; F624 loop 0xf5dd
db 0xE9, 0x9C, 0xFB                     ; F626 jmp 0xf1c5
L_F629:
db 0xE8, 0xD6, 0x00                     ; F629 call 0xf702
db 0x8B, 0xF0                           ; F62C mov si,ax
db 0x83, 0xEC, 0x08                     ; F62E sub sp,byte +0x8
db 0x8B, 0xEC                           ; F631 mov bp,sp
db 0x80, 0x3E, 0x49, 0x00, 0x06         ; F633 cmp byte [0x49],0x6
db 0x06                                 ; F638 push es
db 0x1F                                 ; F639 pop ds
db 0x72, 0x1A                           ; F63A jc 0xf656
db 0xB6, 0x04                           ; F63C mov dh,0x4
db 0x8A, 0x04                           ; F63E mov al,[si]
db 0x88, 0x46, 0x00                     ; F640 mov [bp+0x0],al
db 0x45                                 ; F643 inc bp
db 0x8A, 0x84, 0x00, 0x20               ; F644 mov al,[si+0x2000]
db 0x88, 0x46, 0x00                     ; F648 mov [bp+0x0],al
db 0x45                                 ; F64B inc bp
db 0x83, 0xC6, 0x50                     ; F64C add si,byte +0x50
db 0xFE, 0xCE                           ; F64F dec dh
db 0x75, 0xEB                           ; F651 jnz 0xf63e
db 0xEB, 0x17                           ; F653 jmp short 0xf66c
db 0x90                                 ; F655 nop
db 0xD1, 0xE6                           ; F656 shl si,1
db 0xB6, 0x04                           ; F658 mov dh,0x4
db 0xE8, 0x88, 0x00                     ; F65A call 0xf6e5
db 0x81, 0xC6, 0x00, 0x20               ; F65D add si,0x2000
db 0xE8, 0x81, 0x00                     ; F661 call 0xf6e5
db 0x81, 0xEE, 0xB0, 0x1F               ; F664 sub si,0x1fb0
db 0xFE, 0xCE                           ; F668 dec dh
db 0x75, 0xEE                           ; F66A jnz 0xf65a
db 0xBF, 0x6E, 0xFA                     ; F66C mov di,0xfa6e
db 0x90                                 ; F66F nop
db 0x0E                                 ; F670 push cs
db 0x07                                 ; F671 pop es
db 0x83, 0xED, 0x08                     ; F672 sub bp,byte +0x8
db 0x8B, 0xF5                           ; F675 mov si,bp
db 0xFC                                 ; F677 cld
db 0xB0, 0x00                           ; F678 mov al,0x0
db 0x16                                 ; F67A push ss
db 0x1F                                 ; F67B pop ds
db 0xBA, 0x80, 0x00                     ; F67C mov dx,0x80
db 0x56                                 ; F67F push si
db 0x57                                 ; F680 push di
db 0xB9, 0x08, 0x00                     ; F681 mov cx,0x8
db 0xF3, 0xA6                           ; F684 repe cmpsb
db 0x5F                                 ; F686 pop di
db 0x5E                                 ; F687 pop si
db 0x74, 0x1E                           ; F688 jz 0xf6a8
db 0xFE, 0xC0                           ; F68A inc al
db 0x83, 0xC7, 0x08                     ; F68C add di,byte +0x8
db 0x4A                                 ; F68F dec dx
db 0x75, 0xED                           ; F690 jnz 0xf67f
db 0x3C, 0x00                           ; F692 cmp al,0x0
db 0x74, 0x12                           ; F694 jz 0xf6a8
db 0x2B, 0xC0                           ; F696 sub ax,ax
db 0x8E, 0xD8                           ; F698 mov ds,ax
db 0xC4, 0x3E, 0x7C, 0x00               ; F69A les di,[0x7c]
db 0x8C, 0xC0                           ; F69E mov ax,es
db 0x0B, 0xC7                           ; F6A0 or ax,di
db 0x74, 0x04                           ; F6A2 jz 0xf6a8
db 0xB0, 0x80                           ; F6A4 mov al,0x80
db 0xEB, 0xD2                           ; F6A6 jmp short 0xf67a
db 0x83, 0xC4, 0x08                     ; F6A8 add sp,byte +0x8
db 0xE9, 0x17, 0xFB                     ; F6AB jmp 0xf1c5
db 0x80, 0xE3, 0x03                     ; F6AE and bl,0x3
db 0x8A, 0xC3                           ; F6B1 mov al,bl
db 0x51                                 ; F6B3 push cx
db 0xB9, 0x03, 0x00                     ; F6B4 mov cx,0x3
db 0xD0, 0xE0                           ; F6B7 shl al,1
db 0xD0, 0xE0                           ; F6B9 shl al,1
db 0x0A, 0xD8                           ; F6BB or bl,al
db 0xE2, 0xF8                           ; F6BD loop 0xf6b7
db 0x8A, 0xFB                           ; F6BF mov bh,bl
db 0x59                                 ; F6C1 pop cx
db 0xC3                                 ; F6C2 ret
db 0x52                                 ; F6C3 push dx
db 0x51                                 ; F6C4 push cx
db 0x53                                 ; F6C5 push bx
db 0x2B, 0xD2                           ; F6C6 sub dx,dx
db 0xB9, 0x01, 0x00                     ; F6C8 mov cx,0x1
db 0x8B, 0xD8                           ; F6CB mov bx,ax
db 0x23, 0xD9                           ; F6CD and bx,cx
db 0x0B, 0xD3                           ; F6CF or dx,bx
db 0xD1, 0xE0                           ; F6D1 shl ax,1
db 0xD1, 0xE1                           ; F6D3 shl cx,1
db 0x8B, 0xD8                           ; F6D5 mov bx,ax
db 0x23, 0xD9                           ; F6D7 and bx,cx
db 0x0B, 0xD3                           ; F6D9 or dx,bx
db 0xD1, 0xE1                           ; F6DB shl cx,1
db 0x73, 0xEC                           ; F6DD jnc 0xf6cb
db 0x8B, 0xC2                           ; F6DF mov ax,dx
db 0x5B                                 ; F6E1 pop bx
db 0x59                                 ; F6E2 pop cx
db 0x5A                                 ; F6E3 pop dx
db 0xC3                                 ; F6E4 ret
db 0x8A, 0x24                           ; F6E5 mov ah,[si]
db 0x8A, 0x44, 0x01                     ; F6E7 mov al,[si+0x1]
db 0xB9, 0x00, 0xC0                     ; F6EA mov cx,0xc000
db 0xB2, 0x00                           ; F6ED mov dl,0x0
db 0x85, 0xC1                           ; F6EF test cx,ax
db 0xF8                                 ; F6F1 clc
db 0x74, 0x01                           ; F6F2 jz 0xf6f5
db 0xF9                                 ; F6F4 stc
db 0xD0, 0xD2                           ; F6F5 rcl dl,1
db 0xD1, 0xE9                           ; F6F7 shr cx,1
db 0xD1, 0xE9                           ; F6F9 shr cx,1
db 0x73, 0xF2                           ; F6FB jnc 0xf6ef
db 0x88, 0x56, 0x00                     ; F6FD mov [bp+0x0],dl
db 0x45                                 ; F700 inc bp
db 0xC3                                 ; F701 ret

L_F702:
	mov	ax, [0x0050]			; cursor position
	push	bx
	mov	bx, ax

	mov	al, ah
	mul	byte [0x004a]			; number of columns
	shl	ax, 1
	shl	ax, 1

	sub	bh, bh
	add	ax, bx

	pop	bx
	ret


;-----------------------------------------------------------------------------
; int 10 func 0e - print character
; inp: AL = character
;      BH = page
;      BL = foreground color
;-----------------------------------------------------------------------------

int_10_0e:					; F718
	push	ax

	push	ax
	mov	ah, 3
	mov	bh, [0x0062]			; current page
	int	0x10				; get cursor position
	pop	ax

	cmp	al, 0x08
	je	L_F779

	cmp	al, 0x0d
	je	L_F782

	cmp	al, 0x0a
	je	L_F786

	cmp	al, 0x07
	je	L_F78D

	mov	ah, 0x0a
	mov	cx, 1
	int	0x10				; write character without attribute

	inc	dl				; x + 1
	cmp	dl, [0x004a]			; number of columns
	jne	L_F775

	mov	dl, 0
	cmp	dh, 24
	jne	L_F773

L_F749:
						; we're at the lower right corner
	mov	ah, 0x02
	int	0x10				; set cursor position

	mov	al, [0x0049]			; video mode
	cmp	al, 4
	jb	L_F75A

	cmp	al, 7
	mov	bh, 0
	jne	L_F760

L_F75A:
	mov	ah, 0x08
	int	0x10				; read character and attribute
	mov	bh, ah				; attribute

L_F760:
	mov	ax, 0x0601
	sub	cx, cx
	mov	dh, 24
	mov	dl, [0x004a]			; number of columns
	dec	dl
L_F76D:
	int	0x10				; scroll up

L_F76F:
	pop	ax

	jmp	int_10_done

L_F773:
						; we're at end of line
	inc	dh				; y + 1

L_F775:
	mov	ah, 0x02
	jmp	short L_F76D

L_F779:
						; backspace
	cmp	dl, 0				; check if first column
	je	L_F775

	dec	dl
	jmp	short L_F775

L_F782:
						; CR
	mov	dl, 0				; x = 0
	jmp	short L_F775

L_F786:
						; LF
	cmp	dh, 24
	jne	L_F773

	jmp	short L_F749

L_F78D:
						; bell
	mov	bl, 2
	call	beep
	jmp	short L_F76F


L_F794:
	db	0x03, 0x03, 0x05, 0x05,
	db	0x03, 0x03, 0x03, 0x04


;-----------------------------------------------------------------------------
; int 10 func 04 - get light pen position
; out: AH = status (0/1)
;      BX = x
;      CH = y
;      DH = text mode row
;      DL = text mode column
;-----------------------------------------------------------------------------

int_10_04:					; F79C
	mov	ah, 0

	mov	dx, [0x0063]			; crtc base
	add	dx, byte 0x06
	in	al, dx

	test	al, 0x04
	jnz	L_F828

	test	al, 0x02
	jnz	L_F7B1

	jmp	L_F832

L_F7B1:
	mov	ah, 0x10
	mov	dx, [0x0063]
	mov	al, ah
	out	dx, al
	inc	dx
	in	al, dx
	mov	ch, al

	dec	dx
	inc	ah

	mov	al, ah
	out	dx, al
	inc	dx
	in	al, dx
	mov	ah, ch

	mov	bl, [0x0049]			; video mode
	sub	bh, bh
	mov	bl, [cs:bx + L_F794]
	sub	ax, bx

	mov	bx, [0x004e]			; page offset
	shr	bx, 1
	sub	ax, bx
	jns	L_F7E1

	sub	ax, ax

L_F7E1:
	mov	cl, 0x03

	cmp	byte [0x0049], 0x04
	jb	L_F814

	cmp	byte [0x49], 0x07
	je	L_F814

; graphic mode
	mov	dl, 0x28			; bytes per row
	div	dl
	mov	ch, al				; row
	add	ch, ch

	mov	bl, ah
	sub	bh, bh
	cmp	byte [0x0049], 0x06
	jne	L_F808

	mov	cl, 0x04
	shl	ah, 1

L_F808:
	shl	bx, cl				; x

	mov	dl, ah				; column
	mov	dh, al				; row
	shr	dh, 1
	shr	dh, 1

	jmp	short L_F826

; text mode
L_F814:
	div	byte [0x004a]			; divide by columns
	mov	dh, al				; row
	mov	dl, ah				; column

	shl	al, cl
	mov	ch, al				; pixel row

	mov	bl, ah
	xor	bh, bh
	shl	bx, cl				; pixel column

L_F826:
	mov	ah, 0x01

L_F828:
	push	dx
	mov	dx, [0x0063]
	add	dx, byte 0x07
	out	dx, al
	pop	dx

L_F832:
	pop	di
	pop	si
	pop	ds
	pop	ds
	pop	ds
	pop	ds
	pop	es
	iret


	db	0xff, 0xff, 0xff, 0xff
	db	0xff, 0xff, 0xff


;-----------------------------------------------------------------------------
; int 12
; out: AX = RAM size in KB
;-----------------------------------------------------------------------------

int_12:						; F841
	sti
	push	ds
	call	set_bios_ds

	mov	ax, [0x0013]			; ram size

	pop	ds
	iret

	dw	0xffff				; F84B


;-----------------------------------------------------------------------------
; int 11
; out: AX = configuration word
;-----------------------------------------------------------------------------

int_11:						; F84D
	sti
	push	ds
	call	set_bios_ds

	mov	ax, [0x0010]			; equipment word

	pop	ds
	iret

	dw	0xffff				; F857


;-----------------------------------------------------------------------------
; int 15
;-----------------------------------------------------------------------------

int_15:						; F859
	sti
	push	ds

	call	set_bios_ds

	and	byte [0x0071], 0x7f		; break flag

	call	L_F86A

	pop	ds
	retf	2

L_F86A:
	or	ah, ah
	jz	int_15_00

	dec	ah
	jz	int_15_01

	dec	ah
	jz	int_15_02

	dec	ah
	jnz	L_F87D
	jmp	int_15_03

L_F87D:
	mov	ah, 0x80
	stc
	ret


;-----------------------------------------------------------------------------
; int 15 func 00 - turn tape drive motor on
;-----------------------------------------------------------------------------

int_15_00:					; F881
	in	al, 0x61
	and	al, 0xf7

L_F885:
	out	0x61, al

	sub	ah, ah
	ret


;-----------------------------------------------------------------------------
; int 15 func 01 - turn tape drive motor off
;-----------------------------------------------------------------------------

int_15_01:					; F88A
	in	al, 0x61
	or	al, 0x08
	jmp	short L_F885


;-----------------------------------------------------------------------------
; int 15 func 02 - read data from tape
;-----------------------------------------------------------------------------

int_15_02:					; F890
	push	bx
	push	cx
	push	si

	mov	si, 0x0007
	call	L_FA58				; turn motor on

db 0xE4, 0x62                           ; F899 in al,0x62
db 0x24, 0x10                           ; F89B and al,0x10
db 0xA2, 0x6B, 0x00                     ; F89D mov [0x6b],al
db 0xBA, 0x7A, 0x3F                     ; F8A0 mov dx,0x3f7a
db 0xF6, 0x06, 0x71, 0x00, 0x80         ; F8A3 test byte [0x71],0x80
db 0x75, 0x03                           ; F8A8 jnz 0xf8ad
db 0x4A                                 ; F8AA dec dx
db 0x75, 0x03                           ; F8AB jnz 0xf8b0
db 0xE9, 0x84, 0x00                     ; F8AD jmp 0xf934
db 0xE8, 0xC6, 0x00                     ; F8B0 call 0xf979
db 0xE3, 0xEE                           ; F8B3 jcxz 0xf8a3
db 0xBA, 0x78, 0x03                     ; F8B5 mov dx,0x378
db 0xB9, 0x00, 0x02                     ; F8B8 mov cx,0x200
db 0xE4, 0x21                           ; F8BB in al,0x21
db 0x0C, 0x01                           ; F8BD or al,0x1
db 0xE6, 0x21                           ; F8BF out 0x21,al
db 0xF6, 0x06, 0x71, 0x00, 0x80         ; F8C1 test byte [0x71],0x80
db 0x75, 0x6C                           ; F8C6 jnz 0xf934
db 0x51                                 ; F8C8 push cx
db 0xE8, 0xAD, 0x00                     ; F8C9 call 0xf979
db 0x0B, 0xC9                           ; F8CC or cx,cx
db 0x59                                 ; F8CE pop cx
db 0x74, 0xC8                           ; F8CF jz 0xf899
db 0x3B, 0xD3                           ; F8D1 cmp dx,bx
db 0xE3, 0x04                           ; F8D3 jcxz 0xf8d9
db 0x73, 0xC2                           ; F8D5 jnc 0xf899
db 0xE2, 0xE8                           ; F8D7 loop 0xf8c1
db 0x72, 0xE6                           ; F8D9 jc 0xf8c1
db 0xE8, 0x9B, 0x00                     ; F8DB call 0xf979
db 0xE8, 0x6A, 0x00                     ; F8DE call 0xf94b
db 0x3C, 0x16                           ; F8E1 cmp al,0x16
db 0x75, 0x49                           ; F8E3 jnz 0xf92e
db 0x5E                                 ; F8E5 pop si
db 0x59                                 ; F8E6 pop cx
db 0x5B                                 ; F8E7 pop bx
db 0x51                                 ; F8E8 push cx
db 0xC7, 0x06, 0x69, 0x00, 0xFF, 0xFF   ; F8E9 mov word [0x69],0xffff
db 0xBA, 0x00, 0x01                     ; F8EF mov dx,0x100
db 0xF6, 0x06, 0x71, 0x00, 0x80         ; F8F2 test byte [0x71],0x80
db 0x75, 0x23                           ; F8F7 jnz 0xf91c
db 0xE8, 0x4F, 0x00                     ; F8F9 call 0xf94b
db 0x72, 0x1E                           ; F8FC jc 0xf91c
db 0xE3, 0x05                           ; F8FE jcxz 0xf905
db 0x26, 0x88, 0x07                     ; F900 mov [es:bx],al
db 0x43                                 ; F903 inc bx
db 0x49                                 ; F904 dec cx
db 0x4A                                 ; F905 dec dx
db 0x7F, 0xEA                           ; F906 jg 0xf8f2
db 0xE8, 0x40, 0x00                     ; F908 call 0xf94b
db 0xE8, 0x3D, 0x00                     ; F90B call 0xf94b
db 0x2A, 0xE4                           ; F90E sub ah,ah
db 0x81, 0x3E, 0x69, 0x00, 0x0F, 0x1D   ; F910 cmp word [0x69],0x1d0f
db 0x75, 0x06                           ; F916 jnz 0xf91e
db 0xE3, 0x06                           ; F918 jcxz 0xf920
db 0xEB, 0xCD                           ; F91A jmp short 0xf8e9
db 0xB4, 0x01                           ; F91C mov ah,0x1
db 0xFE, 0xC4                           ; F91E inc ah
db 0x5A                                 ; F920 pop dx
db 0x2B, 0xD1                           ; F921 sub dx,cx
db 0x50                                 ; F923 push ax
db 0xF6, 0xC4, 0x90                     ; F924 test ah,0x90
db 0x75, 0x13                           ; F927 jnz 0xf93c
db 0xE8, 0x1F, 0x00                     ; F929 call 0xf94b
db 0xEB, 0x0E                           ; F92C jmp short 0xf93c
db 0x4E                                 ; F92E dec si
db 0x74, 0x03                           ; F92F jz 0xf934
db 0xE9, 0x65, 0xFF                     ; F931 jmp 0xf899
db 0x5E                                 ; F934 pop si
db 0x59                                 ; F935 pop cx
db 0x5B                                 ; F936 pop bx
db 0x2B, 0xD2                           ; F937 sub dx,dx
db 0xB4, 0x04                           ; F939 mov ah,0x4
db 0x50                                 ; F93B push ax
db 0xE4, 0x21                           ; F93C in al,0x21
db 0x24, 0xFE                           ; F93E and al,0xfe
db 0xE6, 0x21                           ; F940 out 0x21,al
db 0xE8, 0x45, 0xFF                     ; F942 call 0xf88a
db 0x58                                 ; F945 pop ax
db 0x80, 0xFC, 0x01                     ; F946 cmp ah,0x1
db 0xF5                                 ; F949 cmc
db 0xC3                                 ; F94A ret
db 0x53                                 ; F94B push bx
db 0x51                                 ; F94C push cx
db 0xB1, 0x08                           ; F94D mov cl,0x8
db 0x51                                 ; F94F push cx
db 0xE8, 0x26, 0x00                     ; F950 call 0xf979
db 0xE3, 0x20                           ; F953 jcxz 0xf975
db 0x53                                 ; F955 push bx
db 0xE8, 0x20, 0x00                     ; F956 call 0xf979
db 0x58                                 ; F959 pop ax
db 0xE3, 0x19                           ; F95A jcxz 0xf975
db 0x03, 0xD8                           ; F95C add bx,ax
db 0x81, 0xFB, 0xF0, 0x06               ; F95E cmp bx,0x6f0
db 0xF5                                 ; F962 cmc
db 0x9F                                 ; F963 lahf
db 0x59                                 ; F964 pop cx
db 0xD0, 0xD5                           ; F965 rcl ch,1
db 0x9E                                 ; F967 sahf
db 0xE8, 0xD9, 0x00                     ; F968 call 0xfa44
db 0xFE, 0xC9                           ; F96B dec cl
db 0x75, 0xE0                           ; F96D jnz 0xf94f
db 0x8A, 0xC5                           ; F96F mov al,ch
db 0xF8                                 ; F971 clc
db 0x59                                 ; F972 pop cx
db 0x5B                                 ; F973 pop bx
db 0xC3                                 ; F974 ret
db 0x59                                 ; F975 pop cx
db 0xF9                                 ; F976 stc
db 0xEB, 0xF9                           ; F977 jmp short 0xf972
db 0xB9, 0x64, 0x00                     ; F979 mov cx,0x64
db 0x8A, 0x26, 0x6B, 0x00               ; F97C mov ah,[0x6b]
db 0xE4, 0x62                           ; F980 in al,0x62
db 0x24, 0x10                           ; F982 and al,0x10
db 0x3A, 0xC4                           ; F984 cmp al,ah
db 0xE1, 0xF8                           ; F986 loope 0xf980
db 0xA2, 0x6B, 0x00                     ; F988 mov [0x6b],al
db 0xB0, 0x00                           ; F98B mov al,0x0
db 0xE6, 0x43                           ; F98D out 0x43,al
db 0x8B, 0x1E, 0x67, 0x00               ; F98F mov bx,[0x67]
db 0xE4, 0x40                           ; F993 in al,0x40
db 0x8A, 0xE0                           ; F995 mov ah,al
db 0xE4, 0x40                           ; F997 in al,0x40
db 0x86, 0xC4                           ; F999 xchg al,ah
db 0x2B, 0xD8                           ; F99B sub bx,ax
db 0xA3, 0x67, 0x00                     ; F99D mov [0x67],ax
db 0xC3                                 ; F9A0 ret

int_15_03:					; F9A1
db 0x53                                 ; F9A1 push bx
db 0x51                                 ; F9A2 push cx
db 0xE4, 0x61                           ; F9A3 in al,0x61
db 0x24, 0xFD                           ; F9A5 and al,0xfd
db 0x0C, 0x01                           ; F9A7 or al,0x1
db 0xE6, 0x61                           ; F9A9 out 0x61,al
db 0xB0, 0xB6                           ; F9AB mov al,0xb6
db 0xE6, 0x43                           ; F9AD out 0x43,al
db 0xE8, 0xA6, 0x00                     ; F9AF call 0xfa58
db 0xB8, 0xA0, 0x04                     ; F9B2 mov ax,0x4a0
db 0xE8, 0x85, 0x00                     ; F9B5 call 0xfa3d
db 0xB9, 0x00, 0x08                     ; F9B8 mov cx,0x800
db 0xF9                                 ; F9BB stc
db 0xE8, 0x68, 0x00                     ; F9BC call 0xfa27
db 0xE2, 0xFA                           ; F9BF loop 0xf9bb
db 0xF8                                 ; F9C1 clc
db 0xE8, 0x62, 0x00                     ; F9C2 call 0xfa27
db 0x59                                 ; F9C5 pop cx
db 0x5B                                 ; F9C6 pop bx
db 0xB0, 0x16                           ; F9C7 mov al,0x16
db 0xE8, 0x44, 0x00                     ; F9C9 call 0xfa10
db 0xC7, 0x06, 0x69, 0x00, 0xFF, 0xFF   ; F9CC mov word [0x69],0xffff
db 0xBA, 0x00, 0x01                     ; F9D2 mov dx,0x100
db 0x26, 0x8A, 0x07                     ; F9D5 mov al,[es:bx]
db 0xE8, 0x35, 0x00                     ; F9D8 call 0xfa10
db 0xE3, 0x02                           ; F9DB jcxz 0xf9df
db 0x43                                 ; F9DD inc bx
db 0x49                                 ; F9DE dec cx
db 0x4A                                 ; F9DF dec dx
db 0x7F, 0xF3                           ; F9E0 jg 0xf9d5
db 0xA1, 0x69, 0x00                     ; F9E2 mov ax,[0x69]
db 0xF7, 0xD0                           ; F9E5 not ax
db 0x50                                 ; F9E7 push ax
db 0x86, 0xE0                           ; F9E8 xchg ah,al
db 0xE8, 0x23, 0x00                     ; F9EA call 0xfa10
db 0x58                                 ; F9ED pop ax
db 0xE8, 0x1F, 0x00                     ; F9EE call 0xfa10
db 0x0B, 0xC9                           ; F9F1 or cx,cx
db 0x75, 0xD7                           ; F9F3 jnz 0xf9cc
db 0x51                                 ; F9F5 push cx
db 0xB9, 0x20, 0x00                     ; F9F6 mov cx,0x20
db 0xF9                                 ; F9F9 stc
db 0xE8, 0x2A, 0x00                     ; F9FA call 0xfa27
db 0xE2, 0xFA                           ; F9FD loop 0xf9f9
db 0x59                                 ; F9FF pop cx
db 0xB0, 0xB0                           ; FA00 mov al,0xb0
db 0xE6, 0x43                           ; FA02 out 0x43,al
db 0xB8, 0x01, 0x00                     ; FA04 mov ax,0x1
db 0xE8, 0x33, 0x00                     ; FA07 call 0xfa3d
db 0xE8, 0x7D, 0xFE                     ; FA0A call 0xf88a
db 0x2B, 0xC0                           ; FA0D sub ax,ax
db 0xC3                                 ; FA0F ret
db 0x51                                 ; FA10 push cx
db 0x50                                 ; FA11 push ax
db 0x8A, 0xE8                           ; FA12 mov ch,al
db 0xB1, 0x08                           ; FA14 mov cl,0x8
db 0xD0, 0xD5                           ; FA16 rcl ch,1
db 0x9C                                 ; FA18 pushf
db 0xE8, 0x0B, 0x00                     ; FA19 call 0xfa27
db 0x9D                                 ; FA1C popf
db 0xE8, 0x24, 0x00                     ; FA1D call 0xfa44
db 0xFE, 0xC9                           ; FA20 dec cl
db 0x75, 0xF2                           ; FA22 jnz 0xfa16
db 0x58                                 ; FA24 pop ax
db 0x59                                 ; FA25 pop cx
db 0xC3                                 ; FA26 ret
db 0xB8, 0xA0, 0x04                     ; FA27 mov ax,0x4a0
db 0x72, 0x03                           ; FA2A jc 0xfa2f
db 0xB8, 0x50, 0x02                     ; FA2C mov ax,0x250
db 0x50                                 ; FA2F push ax
db 0xE4, 0x62                           ; FA30 in al,0x62
db 0x24, 0x20                           ; FA32 and al,0x20
db 0x74, 0xFA                           ; FA34 jz 0xfa30
db 0xE4, 0x62                           ; FA36 in al,0x62
db 0x24, 0x20                           ; FA38 and al,0x20
db 0x75, 0xFA                           ; FA3A jnz 0xfa36
db 0x58                                 ; FA3C pop ax
db 0xE6, 0x42                           ; FA3D out 0x42,al
db 0x8A, 0xC4                           ; FA3F mov al,ah
db 0xE6, 0x42                           ; FA41 out 0x42,al
db 0xC3                                 ; FA43 ret
db 0xA1, 0x69, 0x00                     ; FA44 mov ax,[0x69]
db 0xD1, 0xD8                           ; FA47 rcr ax,1
db 0xD1, 0xD0                           ; FA49 rcl ax,1
db 0xF8                                 ; FA4B clc
db 0x71, 0x04                           ; FA4C jno 0xfa52
db 0x35, 0x10, 0x08                     ; FA4E xor ax,0x810
db 0xF9                                 ; FA51 stc
db 0xD1, 0xD0                           ; FA52 rcl ax,1
db 0xA3, 0x69, 0x00                     ; FA54 mov [0x69],ax
db 0xC3                                 ; FA57 ret

L_FA58:
	call	int_15_00			; F881

	mov	bl, 0x42

L_FA5D:
	mov	cx, 0x0700

L_FA60:
	loop	L_FA60

	dec	bl
	jnz	L_FA5D

	ret

L_FA67:
	db	" 201", 0x0d, 0x0a

	db	0xff


; CGA font char 00-7f
L_FA6E:
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 00
	db	0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E	; 01
	db	0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E	; 02
	db	0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00	; 03
	db	0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00	; 04
	db	0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x7C, 0x38, 0x7C	; 05
	db	0x10, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C	; 06
	db	0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00	; 07
	db	0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF	; 08
	db	0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00	; 09
	db	0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF	; 0A
	db	0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78	; 0B
	db	0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18	; 0C
	db	0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0	; 0D
	db	0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0	; 0E
	db	0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99	; 0F
	db	0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00	; 10
	db	0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00	; 11
	db	0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18	; 12
	db	0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00	; 13
	db	0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00	; 14
	db	0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0xCC, 0x78	; 15
	db	0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00	; 16
	db	0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF	; 17
	db	0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00	; 18
	db	0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00	; 19
	db	0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00	; 1A
	db	0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00	; 1B
	db	0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00	; 1C
	db	0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00	; 1D
	db	0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00	; 1E
	db	0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00	; 1F
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 20
	db	0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00	; 21
	db	0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00	; 22
	db	0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00	; 23
	db	0x30, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00	; 24
	db	0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00	; 25
	db	0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00	; 26
	db	0x60, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00	; 27
	db	0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00	; 28
	db	0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00	; 29
	db	0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00	; 2A
	db	0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00	; 2B
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60	; 2C
	db	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00	; 2D
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00	; 2E
	db	0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00	; 2F
	db	0x7C, 0xC6, 0xCE, 0xDE, 0xF6, 0xE6, 0x7C, 0x00	; 30
	db	0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00	; 31
	db	0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00	; 32
	db	0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00	; 33
	db	0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00	; 34
	db	0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00	; 35
	db	0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00	; 36
	db	0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00	; 37
	db	0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00	; 38
	db	0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00	; 39
	db	0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00	; 3A
	db	0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60	; 3B
	db	0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00	; 3C
	db	0x00, 0x00, 0xFC, 0x00, 0x00, 0xFC, 0x00, 0x00	; 3D
	db	0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00	; 3E
	db	0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 0x30, 0x00	; 3F
	db	0x7C, 0xC6, 0xDE, 0xDE, 0xDE, 0xC0, 0x78, 0x00	; 40
	db	0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00	; 41
	db	0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00	; 42
	db	0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00	; 43
	db	0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00	; 44
	db	0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00	; 45
	db	0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00	; 46
	db	0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00	; 47
	db	0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00	; 48
	db	0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00	; 49
	db	0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00	; 4A
	db	0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00	; 4B
	db	0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00	; 4C
	db	0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00	; 4D
	db	0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00	; 4E
	db	0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00	; 4F
	db	0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00	; 50
	db	0x78, 0xCC, 0xCC, 0xCC, 0xDC, 0x78, 0x1C, 0x00	; 51
	db	0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00	; 52
	db	0x78, 0xCC, 0xE0, 0x70, 0x1C, 0xCC, 0x78, 0x00	; 53
	db	0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00	; 54
	db	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00	; 55
	db	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00	; 56
	db	0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x00	; 57
	db	0xC6, 0xC6, 0x6C, 0x38, 0x38, 0x6C, 0xC6, 0x00	; 58
	db	0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00	; 59
	db	0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00	; 5A
	db	0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00	; 5B
	db	0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00	; 5C
	db	0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00	; 5D
	db	0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00	; 5E
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF	; 5F
	db	0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00	; 60
	db	0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00	; 61
	db	0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00	; 62
	db	0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00	; 63
	db	0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00	; 64
	db	0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00	; 65
	db	0x38, 0x6C, 0x60, 0xF0, 0x60, 0x60, 0xF0, 0x00	; 66
	db	0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8	; 67
	db	0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00	; 68
	db	0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00	; 69
	db	0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78	; 6A
	db	0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00	; 6B
	db	0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00	; 6C
	db	0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xC6, 0x00	; 6D
	db	0x00, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00	; 6E
	db	0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00	; 6F
	db	0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0	; 70
	db	0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E	; 71
	db	0x00, 0x00, 0xDC, 0x76, 0x66, 0x60, 0xF0, 0x00	; 72
	db	0x00, 0x00, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x00	; 73
	db	0x10, 0x30, 0x7C, 0x30, 0x30, 0x34, 0x18, 0x00	; 74
	db	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00	; 75
	db	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00	; 76
	db	0x00, 0x00, 0xC6, 0xD6, 0xFE, 0xFE, 0x6C, 0x00	; 77
	db	0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00	; 78
	db	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8	; 79
	db	0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00	; 7A
	db	0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00	; 7B
	db	0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00	; 7C
	db	0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00	; 7D
	db	0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; 7E
	db	0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00	; 7F


;-----------------------------------------------------------------------------
; int 1a
;-----------------------------------------------------------------------------

int_1a:						; FE6E
	sti
	push	ds
	call	set_bios_ds

	or	ah, ah
	jz	int_1a_00

	dec	ah
	jz	int_1a_01

int_1a_done:					; FE7B
	sti
	pop	ds
	iret

int_1a_00:					; FE7E
	cli
	mov	al, [0x0070]			; 24 hour flag
	mov	[0x0070], byte 0
	mov	cx, [0x006e]
	mov	dx, [0x006c]
	jmp	short int_1a_done

int_1a_01:					; FE91
	cli
	mov	[0x006c], dx
	mov	[0x006e], cx
	mov	[0x0070], byte 0
	jmp	short int_1a_done


	db	0xff, 0xff, 0xff, 0xff


;-----------------------------------------------------------------------------
; int 08
;-----------------------------------------------------------------------------

int_08:						; FEA5
	sti

	push	ds
	push	ax
	push	dx

	call	set_bios_ds

	inc	word [0x006c]			; timer low
	jnz	L_FEB6

	inc	word [0x006e]			; timer high

L_FEB6:
	cmp	word [0x006e], byte 0x18
	jne	L_FED2

	cmp	word [0x006c], 0x00b0
	jne	L_FED2

	; 0x1800b0 = 1573040 ticks = 86400 seconds = 1 day

	sub	ax, ax
	mov	[0x006e], ax
	mov	[0x006c], ax
	mov	[0x0070], byte 1		; 24 hour flag

L_FED2:
	dec	byte [0x0040]			; floppy motor timeout
	jnz	L_FEE3

	and	[0x003f], byte 0xf0		; clear motor flags drive 0-3

	mov	al, 0x0c
	mov	dx, 0x03f2
	out	dx, al				; turn off motor

L_FEE3:
	int	0x1c

	mov	al, 0x20
	out	0x20, al

	pop	dx
	pop	ax
	pop	ds
	iret


L_FEED:
	db	"1801", 13, 10


; initial interrupt addresses
bios_int_addr_08:				; FEF3
	dw	int_08				; int 08 (FEA5)
	dw	int_09				; int 09 (E987)
	dw	L_E6DD				; int 0a
	dw	L_E6DD				; int 0b
	dw	L_E6DD				; int 0c
	dw	L_E6DD				; int 0d
	dw	int_0e				; int 0e
	dw	L_E6DD				; int 0f

bios_int_addr_10:				; FF03
	dw	int_10				; int 10 (F065)
	dw	int_11				; int 11 (F84D)
	dw	int_12				; int 12 (F841)
	dw	int_13				; int 13
	dw	int_14				; int 14
	dw	int_15				; int 15 (F859)
	dw	int_16				; int 16 (E82E)
	dw	int_17				; int 17 (EFD2)
	dw	0x0000				; int 18
	dw	int_19				; int 19 (E6F2)
	dw	int_1a				; int 1a (FE6E)
	dw	int_default			; int 1b (FF53)
	dw	int_default			; int 1c (FF53)
	dw	int_1d				; int 1d
	dw	int_1e				; int 1e (EFC7)
	dw	0x0000				; int 1f


L_FF23:
	db	"PARITY CHECK 1", 13, 10	; FF23
	db	" 301", 13, 10			; FF33
	db	"131", 13, 10			; FF39


set_bios_ds:					; FF3E
	push	ax
	mov	ax, 0x0040
	mov	ds, ax
	pop	ax
	ret

	db	0xff

; int 09 handler
L_FF47:
	mov	ah, 1
	push	ax
	mov	al, 0xff
	out	0x21, al			; mask all irqs
	mov	al, 0x20
	out	0x20, al
	pop	ax

int_default:
	iret


;-----------------------------------------------------------------------------
; int 05
;-----------------------------------------------------------------------------

int_05:						; FF54
	sti
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax, 0x0050
	mov	ds, ax

	cmp	byte [0x0000], 0x01		; check if already printing
	je	.done

	mov	byte [0x0000], 0x01		; set flag

	mov	ah, 0x0f
	int	0x10				; get video mode

	mov	cl, ah				; number of columns
	mov	ch, 25				; number of rows

	call	lpt0_prt_crlf

	push	cx
	mov	ah, 0x03
	int	0x10				; get cursor position
	pop	cx

	push	dx				; save cursor position

	xor	dx, dx				; row 0 / column 0

.next:						; FF7F
	mov	ah, 0x02
	int	0x10				; set cursor position

	mov	ah, 0x08
	int	0x10				; get character

	or	al, al
	jnz	.notnull

	mov	al, 0x20			; convert NUL to space

.notnull:					; FF8D
	push	dx
	xor	dx, dx
	xor	ah, ah
	int	0x17				; print character
	pop	dx

	test	ah, 0x25
	jnz	.error

	inc	dl				; next column
	cmp	cl, dl
	jne	.next

	xor	dl, dl				; reset column
	mov	ah, dl

	push	dx
	call	lpt0_prt_crlf
	pop	dx

	inc	dh				; next row
	cmp	ch, dh
	jne	.next

	pop	dx				; original cursor position

	mov	ah, 0x02
	int	0x10				; restore cursor position

	mov	byte [0x0000], 0x00		; reset printing flag

	jmp	short .done

.error:						; FFBB
	pop	dx

	mov	ah, 0x02
	int	0x10				; restore cursor position

	mov	byte [0x0000], 0xff		; set error flag

.done:						; FFC5
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	iret


lpt0_prt_crlf:					; FFCB
	xor	dx, dx
	xor	ah, ah
	mov	al, 0x0a
	int	0x17

	xor	ah, ah
	mov	al, 0x0d
	int	0x17
	ret


L_FFDA:
	db	"PARITY CHECK 2", 13, 10
	db	"601", 13, 10

	db	0xff


L_FFF0:
	jmp	0xf000:start

	db	"10/27/82"

	db	0xff, 0xff

	db	0x78				; checksum
