;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:   src/arch/ibmpc/bios/ega.asm                                  *
;* Created:     2003-09-14 by Hampa Hug <hampa@hampa.ch>                     *
;* Copyright:   (C) 2003-2008 Hampa Hug <hampa@hampa.ch>                     *
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

; $Id$


%include "pce.inc"


%define EGA_BIOS_CS 0xc000

%define BIOS_MODE 0x0049
%define BIOS_COLS 0x004a
%define BIOS_SIZE 0x004c
%define BIOS_OFFS 0x004e
%define BIOS_CPOS 0x0050
%define BIOS_CSIZ 0x0060
%define BIOS_PAGE 0x0062
%define BIOS_CRTC 0x0063
%define BIOS_MSEL 0x0065
%define BIOS_ROWS 0x0084
%define BIOS_CHRH 0x0085
%define BIOS_SWCH 0x0088

%define CRTC_INDEX   0x03d4
%define CRTC_DATA    0x03d5
%define CRTC_CSIZ_HI 0x0a
%define CRTC_CSIZ_LO 0x0b
%define CRTC_OFFS_HI 0x0c
%define CRTC_OFFS_LO 0x0d
%define CRTC_CPOS_HI 0x0e
%define CRTC_CPOS_LO 0x0f
%define CRTC_ROFS    0x13

%define TS_INDEX     0x03c4
%define TS_DATA      0x03c5
%define TS_WRPL      0x02

%define GDC_INDEX    0x03ce
%define GDC_DATA     0x03cf
%define GDC_SETR     0x00
%define GDC_ENAB     0x01
%define GDC_CCMP     0x02
%define GDC_FSEL     0x03
%define GDC_RDPL     0x04
%define GDC_MODE     0x05
%define GDC_MISC     0x06
%define GDC_CARE     0x07
%define GDC_BMSK     0x08

%define ATC_INDEX    0x03c0
%define ATC_DATA     0x03c1
%define ATC_OSCN     0x11


section .text


;-----------------------------------------------------------------------------

rom_start:
	dw	0xaa55
	db	(rom_end - rom_start + 511) / 512

init:
	jmp	start

	set_pos	0x1d
	db	" IBM "

start:
	push	ax
	call	ega_init
	pop	ax
	retf


msg_init:
	db "PCE EGA BIOS version ", PCE_VERSION_STR
	db 13, 10, 13, 10, 0


seg0000		dw 0x0000
seg0040		dw 0x0040
sega000		dw 0xa000
segb000		dw 0xb000
segb800		dw 0xb800

cursor14	db 0, 2, 4, 6, 8, 10, 11, 13, 14

modemap		db 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x06, 0x07
		db 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0d, 0x0d
		db 0x0d, 0x0d, 0x0d

pal_default	db 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07
		db 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f


;-----------------------------------------------------------------------------
; video parameter table
;-----------------------------------------------------------------------------
video_parameters:

; text 40 * 25, CGA
mode00_200:
	db	40, 24, 8
	dw	0x0800
	db	0x0b, 0x03, 0x00, 0x03				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x31, 0x15, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; text 40 * 25, CGA
mode01_200:
	db	40, 24, 8
	dw	0x0800
	db	0x0b, 0x03, 0x00, 0x03				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x31, 0x15, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; text 80 * 25, CGA
mode02_200:
	db	80, 24, 8
	dw	0x1000
	db	0x01, 0x03, 0x00, 0x03				; seq
	db	0x23
	db	0x70, 0x4f, 0x5c, 0x2f, 0x5f, 0x07, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x28, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; text 80 * 25, CGA
mode03_200:
	db	80, 24, 8
	dw	0x1000
	db	0x01, 0x03, 0x00, 0x03				; seq
	db	0x23
	db	0x70, 0x4f, 0x5c, 0x2f, 0x5f, 0x07, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x28, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; graphics 320 * 200 * 4
mode04:
	db	40, 24, 8
	dw	0x4000
	db	0x0b, 0x03, 0x00, 0x02				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x30, 0x14, 0x04, 0x11	; crtc
	db	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x00, 0xe0, 0xf0, 0xa2
	db	0xff
	db	0x00, 0x13, 0x15, 0x17, 0x02, 0x04, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x01, 0x00, 0x03, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0f, 0x00	; grc
	db	0xff

; graphics 320 * 200 * 4
mode05:
	db	40, 24, 8
	dw	0x4000
	db	0x0b, 0x03, 0x00, 0x02				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x30, 0x14, 0x04, 0x11	; crtc
	db	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x00, 0xe0, 0xf0, 0xa2
	db	0xff
	db	0x00, 0x13, 0x15, 0x17, 0x02, 0x04, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x01, 0x00, 0x03, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0f, 0x00	; grc
	db	0xff

; graphics 640 * 200 * 2
mode06:
	db	80, 24, 8
	dw	0x4000
	db	0x01, 0x01, 0x00, 0x06				; seq
	db	0x23
	db	0x70, 0x4f, 0x59, 0x2d, 0x5e, 0x06, 0x04, 0x11	; crtc
	db	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0xe0, 0x23, 0xc7, 0x28, 0x00, 0xdf, 0xef, 0xc2
	db	0xff
	db	0x00, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17	; atc
	db	0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17
	db	0x01, 0x00, 0x01, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00	; grc
	db	0xff

; text 80 * 25, MDA
mode07:
	db	80, 24, 14
	dw	0x1000
	db	0x00, 0x03, 0x00, 0x03				; seq
	db	0xa6
	db	0x60, 0x4f, 0x56, 0x3a, 0x51, 0x60, 0x70, 0x1f	; crtc
	db	0x00, 0x0d, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2e, 0x5d, 0x28, 0x0d, 0x5e, 0x6e, 0xa3
	db	0xff
	db	0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08	; atc
	db	0x10, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
	db	0x0e, 0x00, 0x0f, 0x08
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0a, 0x00	; grc
	db	0xff

mode08:
	db	40, 24, 8
	dw	0x4000
	db	0x00, 0x00, 0x00, 0x03				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x31, 0x15, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

mode09:
	db	40, 24, 8
	dw	0x4000
	db	0x00, 0x00, 0x00, 0x03				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x31, 0x15, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

mode0a:
	db	40, 24, 8
	dw	0x4000
	db	0x00, 0x00, 0x00, 0x03				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x31, 0x15, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

mode0b:
	db	80, 24, 8
	dw	0x1000
	db	0x01, 0x04, 0x00, 0x07				; seq
	db	0x23
	db	0x70, 0x4f, 0x5c, 0x2f, 0x5f, 0x07, 0x04, 0x11	; crtc
	db	0x00, 0x07, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x28, 0x08, 0xe0, 0xf0, 0xa3
	db	0xff
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; atc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x00, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00	; grc
	db	0xff

mode0c:
	db	80, 24, 14
	dw	0x1000
	db	0x00, 0x04, 0x00, 0x07				; seq
	db	0xa6
	db	0x60, 0x4f, 0x56, 0x3a, 0x51, 0x60, 0x70, 0x1f	; crtc
	db	0x00, 0x0d, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2e, 0x5d, 0x28, 0x0d, 0x5e, 0x6e, 0xa3
	db	0xff
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; atc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x0e, 0x00, 0x0f, 0x08
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00	; grc
	db	0xff

; graphics 320 * 200 * 16
mode0d:
	db	40, 24, 8
	dw	0x2000
	db	0x0b, 0x0f, 0x00, 0x06				; seq
	db	0x23
	db	0x37, 0x27, 0x2d, 0x37, 0x30, 0x14, 0x04, 0x11	; crtc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0xe1, 0x24, 0xc7, 0x14, 0x00, 0xe0, 0xf0, 0xe3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x01, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0f	; grc
	db	0xff

; graphics 640 * 200 * 16
mode0e:
	db	80, 24, 8
	dw	0x4000
	db	0x01, 0x0f, 0x00, 0x06				; seq
	db	0x23
	db	0x70, 0x4f, 0x59, 0x2d, 0x5e, 0x06, 0x04, 0x11	; crtc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0xe0, 0x23, 0xc7, 0x28, 0x00, 0xdf, 0xef, 0xe3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07	; atc
	db	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
	db	0x01, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0f	; grc
	db	0xff

; graphics 640 * 350 * 2, with 64K
mode0f_64:
	db	80, 24, 14
	dw	0x8000
	db	0x05, 0x0f, 0x00, 0x00				; seq
	db	0xa2
	db	0x60, 0x4f, 0x56, 0x1a, 0x50, 0xe0, 0x70, 0x1f	; crtc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2e, 0x5d, 0x14, 0x0d, 0x5e, 0x6e, 0x8b
	db	0xff
	db	0x00, 0x08, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00	; atc
	db	0x00, 0x08, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00
	db	0x0b, 0x00, 0x05, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x07, 0x0f	; grc
	db	0xff

; graphics 640 * 350 * 16, with 64K
mode10_64:
	db	80, 24, 14
	dw	0x8000
	db	0x05, 0x0f, 0x00, 0x00				; seq
	db	0xa7
	db	0x5b, 0x4f, 0x53, 0x17, 0x50, 0xba, 0x6c, 0x1f	; crtc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2b, 0x5d, 0x14, 0x0f, 0x5f, 0x0a, 0x8b
	db	0xff
	db	0x00, 0x01, 0x00, 0x00, 0x04, 0x07, 0x00, 0x00	; atc
	db	0x00, 0x01, 0x00, 0x00, 0x04, 0x07, 0x00, 0x00
	db	0x01, 0x00, 0x05, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x07, 0x0f	; grc
	db	0xff

; graphics 640 * 350 * 2
mode0f:
	db	80, 24, 14
	dw	0x8000
	db	0x01, 0x0f, 0x00, 0x06				; seq
	db	0xa2
	db	0x60, 0x4f, 0x56, 0x3a, 0x50, 0x60, 0x70, 0x1f	; crtc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2e, 0x5d, 0x28, 0x0d, 0x5e, 0x6e, 0xe3
	db	0xff
	db	0x00, 0x08, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00	; atc
	db	0x00, 0x08, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00
	db	0x0b, 0x00, 0x05, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0f	; grc
	db	0xff

; graphics 640 * 350 * 16
mode10:
	db	80, 24, 14
	dw	0x8000
	db	0x01, 0x0f, 0x00, 0x06				; seq
	db	0xa7
	db	0x5b, 0x4f, 0x53, 0x37, 0x52, 0x00, 0x6c, 0x1f	; crtc
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2b, 0x5d, 0x28, 0x0f, 0x5f, 0x0a, 0xe3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07	; atc
	db	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	db	0x01, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0f	; grc
	db	0xff

; text 40 * 25, ECD
mode00_350:
	db	40, 24, 14
	dw	0x0800
	db	0x0b, 0x03, 0x00, 0x03				; seq
	db	0xa7
	db	0x2d, 0x27, 0x2b, 0x2d, 0x28, 0x6d, 0x6c, 0x1f	; crtc
	db	0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2b, 0x5d, 0x14, 0x0f, 0x5e, 0x0a, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07	; atc
	db	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; text 40 * 25, ECD
mode01_350:
	db	40, 24, 14
	dw	0x0800
	db	0x0b, 0x03, 0x00, 0x03				; seq
	db	0xa7
	db	0x2d, 0x27, 0x2b, 0x2d, 0x28, 0x6d, 0x6c, 0x1f	; crtc
	db	0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2b, 0x5d, 0x14, 0x0f, 0x5e, 0x0a, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07	; atc
	db	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; text 80 * 25, ECD
mode02_350:
	db	80, 24, 14
	dw	0x1000
	db	0x01, 0x03, 0x00, 0x03				; seq
	db	0xa7
	db	0x5b, 0x4f, 0x53, 0x37, 0x51, 0x5b, 0x6c, 0x1f	; crtc
	db	0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2b, 0x5d, 0x28, 0x0f, 0x5e, 0x0a, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07	; atc
	db	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff

; text 80 * 25, ECD
mode03_350:
	db	80, 24, 14
	dw	0x1000
	db	0x01, 0x03, 0x00, 0x03				; seq
	db	0xa7
	db	0x5b, 0x4f, 0x53, 0x37, 0x51, 0x5b, 0x6c, 0x1f	; crtc
	db	0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
	db	0x5e, 0x2b, 0x5d, 0x28, 0x0f, 0x5e, 0x0a, 0xa3
	db	0xff
	db	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07	; atc
	db	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
	db	0x08, 0x00, 0x0f, 0x00
	db	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00	; grc
	db	0xff


ptr00a8:
	dw	video_parameters, EGA_BIOS_CS	; video parameter table
	dw	0, 0
	dw	0, 0
	dw	0, 0
	dw	0, 0
	dw	0, 0


%include "fnt8x8.inc"
%include "fnt8x14.inc"


int_10_00_func:
	dw	int_10_00_00
	dw	int_10_00_01
	dw	int_10_00_02
	dw	int_10_00_03
	dw	int_10_00_04
	dw	int_10_00_05
	dw	int_10_00_06
	dw	int_10_00_07
	dw	int_10_00_08
	dw	int_10_00_09
	dw	int_10_00_0a
	dw	int_10_00_0b
	dw	int_10_00_0c
	dw	int_10_00_0d
	dw	int_10_00_0e
	dw	int_10_00_0f
	dw	int_10_00_10
int_10_00_funcend:


int_10_func:
	dw	int_10_00
	dw	int_10_01
	dw	int_10_02
	dw	int_10_03
	dw	int_10_04
	dw	int_10_05
	dw	int_10_06
	dw	int_10_07
	dw	int_10_08
	dw	int_10_09
	dw	int_10_0a
	dw	int_10_0b
	dw	int_10_0c
	dw	int_10_0d
	dw	int_10_0e
	dw	int_10_0f
	dw	int_10_10
	dw	int_10_11
	dw	int_10_12
	dw	int_10_13
	dw	int_10_14
	dw	int_10_15
	dw	int_10_16
	dw	int_10_17
	dw	int_10_18
	dw	int_10_19
	dw	int_10_1a
int_10_funcend:


;-----------------------------------------------------------------------------
; Write EGA registers
;
; inp: AL    = first register
;      CX    = register count
;      DX    = index register
;      DS:SI = pointer to registers values
;-----------------------------------------------------------------------------
ega_set_regs:
	push	ax
	push	cx
	push	si

.next:
	mov	ah, [si]
	inc	si
	out	dx, ax
	inc	al
	loop	.next

	pop	si
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Write an ATC register
;
; inp: AL = register index
;      AH = register value
;-----------------------------------------------------------------------------
atc_set_reg:
	push	ax
	push	cx
	push	dx

	mov	cx, ax

	mov	dx, 0x03ba
	in	al, dx
	mov	dl, 0xda
	in	al, dx				; reset index/data flip-flop

	mov	dx, ATC_INDEX
	mov	al, cl
	and	al, 0x1f
	out	dx, al
	mov	al, ch
	out	dx, al

	mov	al, 0x20			; enable palette
	out	dx, al

	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Write ATC registers
;
; inp: AL    = first register
;      CX    = register count
;      DS:SI = pointer to registers
;-----------------------------------------------------------------------------
atc_set_regs:
	push	ax
	push	cx
	push	dx
	push	si

	mov	ah, al

.next:
	mov	dx, 0x03ba
	in	al, dx
	mov	dl, 0xda
	in	al, dx				; reset index/data flipflop

	mov	dx, ATC_INDEX
	mov	al, ah
	and	al, 0x1f
	out	dx, al				; set index

	lodsb
	out	dx, al				; set value

	inc	ah
	loop	.next

	mov	dx, ATC_INDEX
	mov	al, 0x20			; enable palette
	out	dx, al

	pop	si
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Check for an enhanced color display
;
; out: CF = 1 if ECD, 0 otherwise
;-----------------------------------------------------------------------------
ega_check_ecd:
	push	ax
	push	ds

	mov	ds, [cs:seg0040]

	mov	al, [BIOS_SWCH]			; switches
	and	al, 0x0f

	cmp	al, 0x09			; enhanced color display (pri)
	je	.ecd

	cmp	al, 0x03			; enhanced color display (sec)
	je	.ecd

	clc
	jmp	.done

.ecd:
	stc

.done:
	pop	ds
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Get the CRTC port address
;
; out: DX = CRTC port address
;-----------------------------------------------------------------------------
ega_get_crtc:
	push	ds
	mov	ds, [cs:seg0040]
	mov	dx, [BIOS_CRTC]
	pop	ds
	ret


;-----------------------------------------------------------------------------
; Get the video parameter table entry for the current video mode
;
; out: ES:BX = table entry
;-----------------------------------------------------------------------------
ega_get_vpt:
	push	ax
	push	cx

	mov	es, [cs:seg0040]

	mov	ah, 0
	mov	al, [es:BIOS_MODE]		; current mode
	mov	cl, 6

	les	bx, [es:0x00a8]
	les	bx, [es:bx]			; video parameter table

	cmp	al, 0x0f
	jb	.nomem

	add	bx, mode0f - mode0f_64
	jmp	.addmode

.nomem:
	cmp	al, 0x03
	ja	.addmode

	call	ega_check_ecd			; check for ECD
	jnc	.addmode

	add	bx, mode00_350 - mode00_200

.addmode:
	shl	ax, cl
	add	bx, ax

	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Set the current page offset
;
; inp: AX = page offset
;-----------------------------------------------------------------------------
ega_set_page_ofs:
	push	ax
	push	dx

	call	ega_get_crtc

	push	ax
	mov	al, CRTC_OFFS_HI
	out	dx, ax
	pop	ax

	mov	ah, al
	mov	al, CRTC_OFFS_LO
	out	dx, ax

	pop	dx
	pop	ax
	ret


; set cursor position AX
crtc_set_curs_pos:
	push	ax
	push	dx

	push	ax
	mov	dx, CRTC_INDEX
	mov	al, CRTC_CPOS_HI
	out	dx, ax
	pop	ax

	mov	ah, al
	mov	al, CRTC_CPOS_LO
	out	dx, ax

	pop	dx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Set up plane 2 for writing
;-----------------------------------------------------------------------------
plane_2_enter:
	push	ax
	push	dx

	mov	dx, 0x03ce		; GC

	mov	ax, 0x0001
	out	dx, ax			; enable set/reset register

	mov	ax, 0x0003		; data rotate register
	out	dx, ax			; unmodified data

	mov	ax, 0x0005		; graphics mode register
	out	dx, ax			; disable odd/even, write mode 0

	mov	ax, 0x0406		; miscellaneous
	out	dx, ax			; map memory to a000

	mov	ax, 0xff08		; bit mask register
	out	dx, ax

	mov	dx, 0x03c4		; sequencer

	mov	ax, 0x0402		; map mask
	out	dx, ax

	mov	ax, 0x0604		; memory mode register
	out	dx, ax			; disable odd/even

	pop	dx
	pop	ax
	ret

;-----------------------------------------------------------------------------
; Switch from accessing plane 2 back to text mode
;-----------------------------------------------------------------------------
plane_2_leave:
	push	ax
	push	dx
	push	ds

	mov	ds, [cs:seg0040]

	mov	dx, 0x03ce		; GC

	mov	ax, 0x1005		; graphics mode register
	out	dx, ax			; enable odd/even

	mov	ax, 0x0e06		; miscellaneous
	cmp	word [BIOS_CRTC], 0x03b4
	jne	.color
	mov	ah, 0x0a		; b000
.color:
	out	dx, ax

	mov	dx, 0x03c4		; TS

	mov	ax, 0x0302		; map mask
	out	dx, ax

	mov	ax, 0x0204		; memory mode register
	out	dx, ax			; enable odd/even

	pop	ds
	pop	dx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; Get video memory segment
;
; out: AX = Video memory segment for current video mode
;-----------------------------------------------------------------------------
ega_get_seg:
	mov	ax, 0xb800
	cmp	word [BIOS_CRTC], 0x03b4
	jne	.done

	mov	ah, 0xb0

.done:
	ret


; get mode in AH
get_modemap:
	push	bx
	mov	bl, [BIOS_MODE]
	and	bx, 0x007f
	mov	ah, [cs:bx + modemap]
	pop	bx
	ret


; Get page AL offset in AX
get_pofs:
	push	dx
	and	ax, 0x0007
	mul	word [BIOS_SIZE]
	pop	dx
	shl	ax, 1
	ret

; convert position (AL, AH) into addr in AX
get_cofs:
	push	cx
	push	dx

	mov	cx, ax

	mov	al, ah				; row
	mov	ah, 0
	mul	word [BIOS_COLS]

	mov	ch, 0
	add	ax, cx
	shl	ax, 1

	pop	dx
	pop	cx
	ret

; set character AL CX times at (DL, DH) in page BH
txt_set_char_xy:
	jcxz	.done

	xchg	al, bh
	call	get_pofs
	mov	di, ax

	mov	ax, dx
	call	get_cofs
	add	di, ax

	call	ega_get_seg
	mov	es, ax

.next:
	mov	[es:di], bh
	add	di, 2
	loop	.next

.done:
	ret


; set character AL CX times at DL, DH in page BH with color BL
gra_set_char_xyc:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	bp
	push	ds
	push	es

	jcxz	.done

	mul	byte [BIOS_CHRH]
	mov	si, ax				; font index

	mov	al, bh
	call	get_pofs
	mov	di, ax

	push	dx
	mov	al, dh
	mul	byte [BIOS_COLS]
	mul	word [BIOS_CHRH]
	pop	dx
	add	di, ax

	mov	dh, 0
	add	di, dx

	mov	bh, [BIOS_CHRH]
	mov	bp, [BIOS_COLS]

	mov	dx, TS_INDEX
	mov	ax, (0x0f << 8) | TS_WRPL
	out	dx, ax

	mov	dx, GDC_INDEX
	mov	ax, 0x0205
	out	dx, ax

	mov	es, [cs:sega000]

	mov	ds, [cs:seg0000]
	lds	ax, [4 * 0x43]
	add	si, ax

.next:
	mov	ax, (0xff << 8) | GDC_BMSK
	out	dx, ax				; set bit mask

	mov	byte [es:di], 0x00		; clear background
	mov	al, [es:di]			; fill latches

	mov	ah, [si]
	mov	al, GDC_BMSK
	out	dx, ax
	mov	al, bl

	push	cx
	push	di
	rep	stosb
	pop	di
	pop	cx

	inc	si
	add	di, bp
	dec	bh
	jnz	.next

	mov	ax, (0xff << 8) | GDC_BMSK
	out	dx, ax				; reset bit mask

	mov	dx, GDC_INDEX
	mov	ax, 0x0005
	out	dx, ax				; reset read/write mode

.done:
	pop	es
	pop	ds
	pop	bp
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


; scroll entire text screen up one line
txt_scroll_up_1:
	push	ax
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es

	mov	al, [BIOS_PAGE]
	call	get_pofs

	mov	dx, [BIOS_COLS]

	mov	si, ax
	add	si, dx
	add	si, dx

	mov	di, ax

	mov	cx, [BIOS_SIZE]
	shr	cx, 1
	sub	cx, dx

	call	ega_get_seg
	mov	ds, ax
	mov	es, ax

	rep	movsw

	mov	ax, 0x0720
	mov	cx, dx
	rep	stosw

	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	ax
	ret


gra_scroll_up_1:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es

	mov	al, [BIOS_PAGE]
	call	get_pofs
	mov	di, ax
	mov	si, ax

	mov	ax, [BIOS_COLS]
	mul	word [BIOS_CHRH]
	mov	bx, ax				; row size in bytes
	mov	dl, [BIOS_ROWS]
	mov	dh, 0
	mul	dx
	mov	cx, ax				; page size in bytes

	add	si, bx

	mov	dx, GDC_INDEX
	mov	ax, (0x01 << 8) | GDC_MODE
	out	dx, ax				; write mode 1

	mov	dx, TS_INDEX
	mov	ax, (0x0f << 8) | TS_WRPL
	out	dx, ax

	mov	ds, [cs:sega000]
	mov	es, [cs:sega000]

	rep	movsb

	mov	dx, GDC_INDEX
	mov	ax, (0x02 << 8) | GDC_MODE
	out	dx, ax

	mov	ax, (0xff << 8) | GDC_BMSK
	out	dx, ax

	mov	al, 0
	mov	cx, bx
	rep	stosb

	mov	dx, GDC_INDEX
	mov	ax, (0x00 << 8) | GDC_MODE
	out	dx, ax

	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


; Clear the rectangle (CL, CH) - (DL, DH) with AX
txt_clear_rect:
	push	ax
	push	cx
	push	dx
	push	di
	push	es

	sub	dh, ch
	jb	.done
	inc	dh
	sub	dl, cl
	jb	.done
	inc	dl

	push	ax
	mov	al, [BIOS_PAGE]
	call	get_pofs
	mov	di, ax

	mov	ax, cx
	call	get_cofs
	add	di, ax

	call	ega_get_seg
	mov	es, ax
	pop	ax

.next:
	mov	cl, dl
	mov	ch, 0
	push	di
	rep	stosw
	pop	di

	mov	cx, [BIOS_COLS]
	shl	cx, 1
	add	di, cx

	dec	dh
	jnz	.next

.done:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	ax
	ret


; clear CX words at B800:DI with AX
ega_clear_text:
	push	cx
	push	di
	push	es

	mov	es, [cs:segb800]
	rep	stosw

	pop	es
	pop	di
	pop	cx
	ret


; Clear the entire screen in text modes
ega_clear_text_screen:
	push	ax
	push	cx
	push	di
	push	es

	mov	ax, 0x0720
	mov	cx, 16384
	mov	es, [cs:segb800]
	xor	di, di
	rep	stosw

	pop	es
	pop	di
	pop	cx
	pop	ax
	ret


; Clear the entire screen in CGA graphics modes
ega_clear_cga_screen:
	push	ax
	push	cx
	push	di
	push	es

	xor	ax, ax
	mov	cx, 8192
	mov	es, [cs:segb800]
	xor	di, di
	rep	stosw

	pop	es
	pop	di
	pop	cx
	pop	ax
	ret


; clear CX bytes at A000:DI with color AL
ega_clear_graph:
	push	cx
	push	dx
	push	di
	push	es

	push	ax
	mov	dx, GDC_INDEX
	mov	ax, (0x02 << 8) | GDC_MODE
	out	dx, ax

	mov	ax, (0x00 << 8) | GDC_FSEL
	out	dx, ax

	mov	ax, (0xff << 8) | GDC_BMSK
	out	dx, ax

	mov	dx, TS_INDEX
	mov	ax, (0x0f << 8) | TS_WRPL
	out	dx, ax
	pop	ax

	mov	es, [cs:sega000]
	rep	stosb

	pop	es
	pop	di
	pop	dx
	pop	cx
	ret


; Clear the entire screen in graphics modes
ega_clear_graph_screen:
	push	ax
	push	cx
	push	dx
	push	di

	mov	al, 0
	mov	cx, 65535
	xor	di, di
	call	ega_clear_graph

	mov	dx, GDC_INDEX
	mov	ax, (0x00 << 8) | GDC_MODE
	out	dx, ax

	pop	di
	pop	dx
	pop	cx
	pop	ax
	ret


; init mode AL with parameter table DS:SI
int_10_init_mode:
	push	ax
	push	cx
	push	dx
	push	bx
	push	di
	push	es

	mov	dx, 0x03d0
	out	dx, al

	mov	es, [cs:seg0040]

	mov	[es:BIOS_MODE], al

	mov	bx, si

	mov	dx, 0x3c2
	mov	al, [bx + 9]
	out	dx, al				; misc output register

	lea	si, [bx + 10]
	mov	al, 0x00
	mov	cx, 25
	mov	dx, CRTC_INDEX
	call	ega_set_regs			; crtc register

	lea	si, [bx + 5]
	mov	al, 0x01
	mov	cx, 5
	mov	dx, TS_INDEX
	call	ega_set_regs			; sequencer

	lea	si, [bx + 35]
	mov	al, 0x00
	mov	cx, 20
	mov	dx, ATC_INDEX
	call	atc_set_regs			; atc

	lea	si, [bx + 55]
	mov	al, 0x00
	mov	cx, 9
	mov	dx, GDC_INDEX
	call	ega_set_regs			; grc

	mov	dx, TS_INDEX
	mov	ax, 0x0300			; reset register
	out	dx, ax

	mov	al, [bx]
	mov	ah, 0
	mov	[es:BIOS_COLS], ax

	mov	al, [bx + 1]
	mov	[es:BIOS_ROWS], al

	mov	al, [bx + 2]
	mov	ah, 0
	mov	[es:BIOS_CHRH], ax

	mov	ax, [bx + 3]
	mov	[es:BIOS_SIZE], ax

	mov	word [es:BIOS_OFFS], 0

	mov	di, BIOS_CPOS
	mov	cx, 8
	xor	ax, ax
	rep	stosw

	mov	word [es:BIOS_CSIZ], 0x0607
	mov	byte [es:BIOS_PAGE], 0x00
	mov	word [es:BIOS_CRTC], 0x03d4

	xor	ax, ax
	mov	es, ax

	mov	dx, cs

	mov	al, [bx + 2]			; character height

	cmp	al, 8
	jbe	.fnt8x8

	mov	ax, fnt_8x14
	jmp	.fntdone

.fnt8x8:
	mov	ax, fnt_8x8
	jmp	.fntdone

.fntdone:
	mov	word [es:4 * 0x43], ax
	mov	word [es:4 * 0x43 + 2], dx

	pop	es
	pop	di
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


; init mode AL with parameter table CS:SI
int_10_init_mode_cs:
	push	ds

	push	cs
	pop	ds
	call	int_10_init_mode

	pop	ds
	ret


; initialize a text mode
int_10_00_text_mode:
	push	bx

	cmp	byte [0x0088], 0x09
	je	.ecd

	call	int_10_init_mode_cs
	mov	bl, 0x00		; character map
	call	int_10_1102		; load 8x8 font
	jmp	.cga

.ecd:
	mov	si, di
	call	int_10_init_mode_cs
	mov	bl, 0x00
	call	int_10_1101		; load 8x14 font

.cga:
	call	ega_clear_text_screen

	pop	bx
	ret


int_10_00_graph16:
	call	int_10_init_mode_cs
	call	ega_clear_graph_screen
	ret


int_10_00_00:
	mov	si, mode00_200
	mov	di, mode00_350
	jmp	int_10_00_text_mode

int_10_00_01:
	mov	si, mode01_200
	mov	di, mode01_350
	jmp	int_10_00_text_mode

int_10_00_02:
	mov	si, mode02_200
	mov	di, mode02_350
	jmp	int_10_00_text_mode

int_10_00_03:
	mov	si, mode03_200
	mov	di, mode03_350
	jmp	int_10_00_text_mode


int_10_00_04:
	mov	si, mode04
	call	int_10_init_mode_cs
	call	ega_clear_cga_screen
	ret

int_10_00_05:
	mov	si, mode05
	call	int_10_init_mode_cs
	call	ega_clear_cga_screen
	ret

int_10_00_06:
	mov	si, mode06
	call	int_10_init_mode_cs
	call	ega_clear_cga_screen
	ret

int_10_00_07:
	ret

int_10_00_08:
int_10_00_09:
int_10_00_0a:
int_10_00_0b:
int_10_00_0c:
	;pceh	PCEH_STOP
	ret

int_10_00_0d:
	mov	si, mode0d
	jmp	int_10_00_graph16

int_10_00_0e:
	mov	si, mode0e
	jmp	int_10_00_graph16

int_10_00_0f:
	mov	si, mode0f
	jmp	int_10_00_graph16

int_10_00_10:
	mov	si, mode10
	jmp	int_10_00_graph16


;-----------------------------------------------------------------------------
; int 10 func 00 - set video mode
; inp: AL = video mode
;-----------------------------------------------------------------------------
int_10_00:
	push	si
	push	di

	push	ax
	and	al, 0x7f
	cmp	al, 0x12
	pop	ax
	ja	.badfunc

	push	ax
	and	al, 0x7f
	mov	ah, 0
	mov	si, ax
	pop	ax

	shl	si, 1
	cmp	si, (int_10_00_funcend - int_10_00_func)
	jae	.badfunc

	mov	ds, [cs:seg0040]

	call	[cs:si + int_10_00_func]
	jmp	.done

.badfunc:
	;pceh	PCEH_STOP

.done:
	pop	di
	pop	si
	ret


;-----------------------------------------------------------------------------
; int 10 func 01 - set cursor size
; inp: CH = start line
;      CL = end line
;-----------------------------------------------------------------------------
int_10_01:
	push	ax
	push	cx
	push	dx
	push	bx

	mov	ds, [cs:seg0040]

	cmp	cl, 0x07
	jbe	.clok
	mov	cl, 0x08

.clok:
	cmp	ch, 0x07
	jbe	.chok
	mov	ch, 0x08

.chok:
	mov	[BIOS_CSIZ], cx

	mov	bh, 0
	mov	bl, ch
	mov	ch, [cs:bx + cursor14]
	mov	bl, cl
	mov	cl, [cs:bx + cursor14]

	mov	dx, [BIOS_CRTC]
	mov	al, CRTC_CSIZ_HI
	mov	ah, ch
	out	dx, ax

	mov	al, CRTC_CSIZ_LO
	mov	ah, cl
	out	dx, ax

	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 02 - set cursor position
; inp: DH = row
;      DL = column
;-----------------------------------------------------------------------------
int_10_02:
	push	ax
	push	dx
	push	bx

	mov	ds, [cs:seg0040]

	mov	al, bh				; page

	mov	bl, bh
	and	bx, 0x0007
	shl	bx, 1

	mov	[bx + BIOS_CPOS], dx

	cmp	al, [BIOS_PAGE]
	jne	.done

	call	get_pofs
	mov	bx, ax

	mov	ax, dx
	call	get_cofs
	add	bx, ax
	shr	bx, 1

	mov	dx, [BIOS_CRTC]
	mov	al, CRTC_CPOS_HI
	mov	ah, bh
	out	dx, ax

	mov	al, CRTC_CPOS_LO
	mov	ah, bl
	out	dx, ax

.done:
	pop	bx
	pop	dx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 03 - get cursor position and size
; out: CH cursor start line
;      CL cursor end line
;      DH = cursor row
;      DL = cursor column
;-----------------------------------------------------------------------------
int_10_03:
	push	bx

	mov	ds, [cs:seg0040]

	mov	bl, bh
	and	bx, 0x0007
	shl	bx, 1

	mov	cx, [BIOS_CSIZ]
	mov	dx, [bx + BIOS_CPOS]

	pop	bx
	ret


; int 10 func 04 - get lightpen position
int_10_04:
	mov	ah, 0
	ret


;-----------------------------------------------------------------------------
; int 10 func 05 - set current page
; inp: AL = page number
;-----------------------------------------------------------------------------
int_10_05:
	push	ax
	push	cx
	push	dx
	push	bx

	mov	ds, [cs:seg0040]

	mov	[BIOS_PAGE], al

	call	get_pofs
	mov	cx, ax
	shr	cx, 1

	mov	dx, [BIOS_CRTC]
	mov	al, CRTC_OFFS_HI
	mov	ah, ch
	out	dx, ax
	mov	al, CRTC_OFFS_LO
	mov	ah, cl
	out	dx, ax

	mov	bl, [BIOS_PAGE]
	mov	bh, 0
	shl	bx, 1
	mov	ax, [bx + BIOS_CPOS]
	call	get_cofs
	shr	ax, 1
	add	cx, ax

	mov	dx, [BIOS_CRTC]
	mov	al, CRTC_CPOS_HI
	mov	ah, ch
	out	dx, ax
	mov	al, CRTC_CPOS_LO
	mov	ah, cl
	out	dx, ax

	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 06 - scroll up
; inp: AL = row count
;      BH = attribute for new lines
;      CH = top left row
;      CL = top left column
;      DH = bottom right row
;      DL = bottom right column
;-----------------------------------------------------------------------------
int_10_06_cga_txt:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	bp
	push	es

	or	al, al
	jz	.clear

	push	dx
	sub	dh, ch
	inc	dh
	cmp	al, dh
	pop	dx

	jae	.clear
	jmp	.scroll

.clear:
	mov	ah, bh
	mov	al, 0x20
	call	txt_clear_rect
	jmp	.done

.scroll:
	sub	dl, cl
	inc	dl				; window width
	sub	dh, ch
	inc	dh				; window height

	push	ax
	mov	al, [BIOS_PAGE]
	call	get_pofs
	mov	di, ax

	mov	ax, cx
	call	get_cofs
	add	di, ax				; dest in DI
	pop	ax

	mov	bp, [BIOS_COLS]
	shl	bp, 1

	push	ax
	push	dx
	mov	ah, 0
	mul	bp
	mov	si, di
	add	si, ax				; source in SI
	pop	dx
	pop	ax

	mov	bl, dh
	sub	bl, al				; rows to copy

	push	ax
	call	ega_get_seg
	mov	ds, ax
	mov	es, ax
	pop	ax

.copy:
	push	si
	push	di

	mov	cl, dl
	mov	ch, 0
	rep	movsw

	pop	di
	pop	si

	add	si, bp
	add	di, bp

	dec	bl
	jnz	.copy

	mov	bl, al				; rows to clear
	mov	ah, bh
	mov	al, 0x20

.clearrow:
	push	di
	mov	cl, dl
	mov	ch, 0
	rep	stosw
	pop	di

	add	di, bp

	dec	bl
	jnz	.clearrow

.done:
	pop	es
	pop	bp
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret

int_10_06:
	push	ax

	mov	ds, [cs:seg0040]

	call	get_modemap

	cmp	ah, 0x00
	je	.mode00

	cmp	ah, 0x04
	jb	.done
	cmp	ah, 0x06
	jbe	.mode04

	jmp	.done

.mode00:
	call	int_10_06_cga_txt
	jmp	.done

.mode04:
	pop	ax
	int	0x42
	ret

.done:
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 07 - scroll down
; inp: AL = row count
;      BH = attribute for new lines
;      CH = top left row
;      CL = top left column
;      DH = bottom right row
;      DL = bottom right column
;-----------------------------------------------------------------------------
int_10_07_cga_txt:
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	bp
	push	es

	or	al, al
	jz	.clear

	push	dx
	sub	dh, ch
	inc	dh
	cmp	al, dh
	pop	dx

	jae	.clear
	jmp	.scroll

.clear:
	mov	ah, bh
	mov	al, 0x20
	call	txt_clear_rect
	jmp	.done

.scroll:
	sub	dl, cl
	inc	dl				; window width
	sub	dh, ch
	inc	dh				; window height

	push	ax
	mov	al, [BIOS_PAGE]
	call	get_pofs
	mov	di, ax

	mov	ax, cx
	add	ah, dh
	dec	ah
	call	get_cofs
	add	di, ax				; dest in DI
	pop	ax

	mov	bp, [BIOS_COLS]
	shl	bp, 1

	push	ax
	push	dx
	mov	ah, 0
	mul	bp
	mov	si, di
	sub	si, ax				; source in SI
	pop	dx
	pop	ax

	mov	bl, dh
	sub	bl, al				; rows to copy

	push	ax
	call	ega_get_seg
	mov	ds, ax
	mov	es, ax
	pop	ax

.copy:
	push	si
	push	di

	mov	cl, dl
	mov	ch, 0
	rep	movsw

	pop	di
	pop	si

	sub	si, bp
	sub	di, bp

	dec	bl
	jnz	.copy

	mov	bl, al				; rows to clear
	mov	ah, bh
	mov	al, 0x20

.clearrow:
	push	di
	mov	cl, dl
	mov	ch, 0
	rep	stosw
	pop	di

	sub	di, bp

	dec	bl
	jnz	.clearrow

.done:
	pop	es
	pop	bp
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret

int_10_07:
	push	ax

	mov	ds, [cs:seg0040]

	call	get_modemap

	cmp	ah, 0x00
	je	.mode00

	cmp	ah, 0x04
	jb	.done
	cmp	ah, 0x06
	jbe	.mode04

	jmp	.done

.mode00:
	call	int_10_07_cga_txt
	jmp	.done

.mode04:
	pop	ax
	int	0x42
	ret

.done:
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 08 - get character and attribute
; inp: BH = page
; out: AH = attribute
;      AL = character
;-----------------------------------------------------------------------------
int_10_08:
	push	bx
	push	si
	push	es

	mov	ds, [cs:seg0040]

	call	ega_get_seg
	mov	es, ax

	mov	al, bh
	call	get_pofs
	mov	si, ax

	mov	bl, bh
	and	bx, 0x0007
	shl	bx, 1
	mov	ax, [bx + BIOS_CPOS]
	call	get_cofs
	add	si, ax

	mov	ax, [es:si]

	pop	es
	pop	si
	pop	bx
	ret


;-----------------------------------------------------------------------------
; int 10 func 09 - set character and attribute
; inp: AL = character
;      BH = page
;      BL = attribute
;      CX = count
;-----------------------------------------------------------------------------
int_10_09_mode03:
	push	cx
	push	bx
	push	di
	push	es

	mov	ah, bl

	push	ax

	call	ega_get_seg
	mov	es, ax

	mov	al, bh
	call	get_pofs
	mov	di, ax

	mov	bl, bh
	and	bx, 0x0007
	shl	bx, 1
	mov	ax, [bx + BIOS_CPOS]
	call	get_cofs
	add	di, ax

	pop	ax

	rep	stosw

	pop	es
	pop	di
	pop	bx
	pop	cx
	ret

int_10_09_mode0d:
	push	dx

	push	bx
	mov	bl, [BIOS_PAGE]
	and	bx, 0x0007
	mov	dx, [bx + BIOS_CPOS]
	pop	bx

	call	gra_set_char_xyc

	pop	dx
	ret

int_10_09:
	push	ax

	mov	ds, [cs:seg0040]

	call	get_modemap

	cmp	ah, 0x00
	je	.mode00

	cmp	ah, 0x0d
	je	.mode0d

	cmp	ah, 0x04
	jb	.done
	cmp	ah, 0x06
	jbe	.mode04

	jmp	.done

.mode00:
	call	int_10_09_mode03
	jmp	.done

.mode0d:
	call	int_10_09_mode0d
	jmp	.done

.mode04:
	pop	ax
	int	0x42
	ret

.done:
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 0a - set character
; inp: AL = character
;      BH = page
;      CX = count
;-----------------------------------------------------------------------------
int_10_0a_mode03:
	push	ax
	push	cx
	push	bx
	push	di
	push	es

	jcxz	.done

	mov	ds, [cs:seg0040]

	push	ax

	call	ega_get_seg
	mov	es, ax

	mov	al, bh
	call	get_pofs
	mov	di, ax

	mov	bl, bh
	and	bx, 0x0007
	shl	bx, 1
	mov	ax, [bx + BIOS_CPOS]
	call	get_cofs
	add	di, ax

	pop	ax

.next:
	stosb
	inc	di
	loop	.next

.done:
	pop	es
	pop	di
	pop	bx
	pop	cx
	pop	ax
	ret

int_10_0a:
	push	ax

	mov	ds, [cs:seg0040]

	push	bx
	mov	bl, [BIOS_MODE]
	and	bx, 0x007f
	mov	ah, [cs:bx + modemap]
	pop	bx

	cmp	ah, 0x00
	jne	.not00

	call	int_10_0a_mode03
	jmp	.done

.not00:
	cmp	ah, 0x0d
	jne	.not0d

	;call	int_10_0e_mode0d
	jmp	.done

.not0d:
	pop	ax
	push	ax

	int	0x42

.done:
	pop	ax
	ret


; int 10 func 0b - set background color
int_10_0b:
	ret


;-----------------------------------------------------------------------------
; int 10 func 0c - set pixel
; inp: AL = color
;      BH = page
;      CX = x
;      DX = y
;-----------------------------------------------------------------------------
int_10_0c:
	push	ax
	push	cx
	push	dx
	push	bx
	push	es

	mov	ds, [cs:seg0040]

	mov	bl, al				; color

	mov	ax, dx
	mul	word [BIOS_COLS]
	mov	dl, cl				; bit index
	mov	dh, bl				; color
	shr	cx, 1
	shr	cx, 1
	shr	cx, 1
	add	cx, ax

	mov	al, bh
	call	get_pofs
	mov	bx, ax
	add	bx, cx

	mov	cx, dx

	mov	dx, TS_INDEX
	mov	ax, (0x0f << 8) | TS_WRPL
	out	dx, ax				; enable all write planes

	mov	dx, GDC_INDEX
	mov	ax, (0x02 << 8) | GDC_MODE
	out	dx, ax				; set write mode 2

	and	cl, 0x07			; bit index
	mov	ah, 0x80
	shr	ah, cl
	mov	al, GDC_BMSK
	out	dx, ax				; set bit mask

	mov	ax, 0xa000
	mov	es, ax

	mov	al, [es:bx]			; fill latches
	mov	[es:bx], ch			; set pixel

	mov	ax, (0x00 << 8) | GDC_MODE
	out	dx, ax				; reset mode

	mov	ax, (0xff << 8) | GDC_BMSK
	out	dx, ax				; reset bit mask

	pop	es
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 0d - get pixel
;
; inp: BH = page
;      CX = x
;      DX = y
; out: AL = color
;-----------------------------------------------------------------------------
int_10_0d:
	mov	al, 0x00
	ret


;-----------------------------------------------------------------------------
; int 10 func 0e - print character
; inp: AL = character
;      BL = color
;-----------------------------------------------------------------------------
int_10_0e_mode0d:
	push	cx
	push	dx
	push	bx
	push	di
	push	es

	mov	ah, bl				; save color

	mov	bl, [BIOS_PAGE]
	and	bx, 0x0007
	shl	bx, 1

	mov	dx, [bx + BIOS_CPOS]

	cmp	al, 0x0a
	jne	.notlf

	inc	dh
	cmp	dh, 25
	jb	.ok

	call	gra_scroll_up_1
	mov	dh, 24
	jmp	.ok

.notlf:
	cmp	al, 0x0d
	jne	.notcr

	mov	dl, 0
	jmp	.ok

.notcr:
	cmp	al, 0x08
	jne	.notbs

	or	dl, dl
	jz	.ok

	dec	dl
	jmp	.ok

.notbs:
	push	bx
	mov	bh, [BIOS_PAGE]
	mov	bl, ah
	mov	cx, 1
	call	gra_set_char_xyc
	pop	bx

	inc	dl
	cmp	dl, [BIOS_COLS]
	jb	.ok

	mov	dl, 0
	inc	dh
	cmp	dh, 25
	jb	.ok

	call	gra_scroll_up_1
	mov	dh, 24

.ok:
	mov	[bx + BIOS_CPOS], dx

	pop	es
	pop	di
	pop	bx
	pop	dx
	pop	cx
	ret

int_10_0e_mode03:
	push	cx
	push	dx
	push	bx
	push	di
	push	es

	mov	bl, [BIOS_PAGE]
	and	bx, 0x0007
	shl	bx, 1

	mov	dx, [bx + BIOS_CPOS]

	cmp	al, 0x0a
	jne	.notlf

	inc	dh
	cmp	dh, 25
	jb	.ok

	call	txt_scroll_up_1
	mov	dh, 24
	jmp	.ok

.notlf:
	cmp	al, 0x0d
	jne	.notcr

	mov	dl, 0
	jmp	.ok

.notcr:
	cmp	al, 0x08
	jne	.notbs

	or	dl, dl
	jz	.ok

	dec	dl
	jmp	.ok

.notbs:
	mov	cl, al

	call	ega_get_seg
	mov	es, ax

	mov	al, [BIOS_PAGE]
	call	get_pofs
	mov	di, ax

	mov	ax, [bx + BIOS_CPOS]
	call	get_cofs
	add	di, ax

	mov	[es:di], cl

	mov	dx, [bx + BIOS_CPOS]
	inc	dl
	cmp	dl, [BIOS_COLS]
	jb	.ok

	mov	dl, 0
	inc	dh
	cmp	dh, 25
	jb	.ok

	call	txt_scroll_up_1
	mov	dh, 24

.ok:
	mov	[bx + BIOS_CPOS], dx

	mov	ax, dx
	call	get_cofs
	shr	ax, 1

	mov	cx, ax
	mov	dx, [BIOS_CRTC]
	mov	al, CRTC_CPOS_HI
	mov	ah, ch
	out	dx, ax
	mov	al, CRTC_CPOS_LO
	mov	ah, cl
	out	dx, ax

	pop	es
	pop	di
	pop	bx
	pop	dx
	pop	cx
	ret

int_10_0e:
	push	ax

	mov	ds, [cs:seg0040]

	push	bx
	mov	bl, [BIOS_MODE]
	and	bx, 0x007f
	mov	ah, [cs:bx + modemap]
	pop	bx

	cmp	ah, 0x00
	jne	.not00

	call	int_10_0e_mode03
	jmp	.done

.not00:
	cmp	ah, 0x0d
	jne	.not0d

	call	int_10_0e_mode0d
	jmp	.done

.not0d:
	pop	ax
	push	ax

	int	0x42

.done:
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 0f - get video mode
;
; out: AL = video mode
;      AH = columns
;      BH = current page
;-----------------------------------------------------------------------------
int_10_0f:
	mov	ds, [cs:seg0040]
	mov	al, [BIOS_MODE]
	mov	ah, [BIOS_COLS]
	mov	bh, [BIOS_PAGE]
	ret


;-----------------------------------------------------------------------------
; int 10 func 10
;-----------------------------------------------------------------------------
int_10_10:
	cmp	al, 0x01
	jb	int_10_1000
	je	int_10_1001

	cmp	al, 0x03
	jb	int_10_1002
	je	int_10_1003

	ret


;-----------------------------------------------------------------------------
; int 10 func 1000 - set palette register
;
; inp: BL = register index
;      BH = new value
;-----------------------------------------------------------------------------
int_10_1000:
	push	ax

	mov	ax, bx
	call	atc_set_reg

	mov	ds, [cs:seg0040]
	lds	si, [0x00a8]
	lds	si, [si + 4]
	mov	ax, ds
	or	ax, si
	jz	.done

	mov	al, bl
	mov	ah, 0
	add	si, ax
	mov	[si], bh

.done:
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 1001 - set overscan color
;
; inp BH = new value
;-----------------------------------------------------------------------------
int_10_1001:
	push	ax
	mov	ah, bh
	mov	al, ATC_OSCN
	call	atc_set_reg
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 1002 - set entire palette
;-----------------------------------------------------------------------------
int_10_1002:
	push	ax
	push	cx
	push	si

	mov	al, 0
	mov	cx, 16
	push	es
	pop	ds
	mov	si, dx
	call	atc_set_regs

	pop	si
	pop	cx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 1003 - enable blinking
;-----------------------------------------------------------------------------
int_10_1003:
	push	ax
	push	bx
	push	es

	mov	ds, [cs:seg0040]

	mov	al, bl

	call	ega_get_vpt
	mov	ah, [es:bx + 0x33]		; ATC / attribute mode

	or	al, al
	jz	.noblink

	cmp	al, 1
	je	.blink

	jmp	.writeback

.noblink:
	and	ah, ~0x08
	and	byte [BIOS_MSEL], ~0x20
	jmp	.writeback

.blink:
	or	ah, 0x08
	or	byte [BIOS_MSEL], 0x20

.writeback:
	mov	al, 0x10
	call	atc_set_reg

	pop	es
	pop	bx
	pop	ax
	ret


;-----------------------------------------------------------------------------
; int 10 func 11
;-----------------------------------------------------------------------------
int_10_11:
	cmp	al, 0x01
	jb	int_10_1100
	je	int_10_1101

	cmp	al, 0x03
	jb	int_10_1102

	cmp	al, 0x30
	je	int_10_1130

	ret


;-----------------------------------------------------------------------------
; int 10 func 1100 - load fonts
;
; inp: BH    = character height
;      BL    = character map number
;      CX    = character count
;      DX    = first character
;      ES:BP = pointer to font
;-----------------------------------------------------------------------------
int_10_1100:
	push	ax
	push	cx
	push	dx
	push	bx
	push	di
	push	bp
	push	es

	jcxz	.done

	call	plane_2_enter

	push	es
	pop	ds
	mov	si, bp			; ds:si point to font

	xchg	cx, dx

	mov	es, [cs:sega000]
	mov	di, cx			; first character
	mov	cx, 0x0005
	shl	di, cl			; first character offset

	mov	ah, bl			; map number
	ror	ah, 1
	ror	ah, 1
	and	ax, 0xc000
	add	di, ax			; map offset

	mov	bl, 32
	sub	bl, bh

	mov	al, 0x00

.nextchar:
	mov	cl, bh			; character height
	rep	movsb

	mov	cl, bl
	rep	stosb			; clear rest

	dec	dx
	jnz	.nextchar

	call	plane_2_leave

.done:
	pop	es
	pop	bp
	pop	di
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret


; load complete font at CS:BP
load_font:
	push	cx
	push	dx
	push	es

	mov	cx, 256
	xor	dx, dx
	push	cs
	pop	es

	call	int_10_1100

	pop	es
	pop	dx
	pop	cx
	ret


;-----------------------------------------------------------------------------
; int 10 func 1101 - load 8x14 font
;-----------------------------------------------------------------------------
int_10_1101:
	push	bx
	push	bp

	mov	bh, 14
	mov	bp, fnt_8x14

	call	load_font

	pop	bp
	pop	bx
	ret


;-----------------------------------------------------------------------------
; int 10 func 1102 - load 8x8 font
;-----------------------------------------------------------------------------
int_10_1102:
	push	bx
	push	bp

	mov	bh, 8
	mov	bp, fnt_8x8

	call	load_font

	pop	bp
	pop	bx
	ret


;-----------------------------------------------------------------------------
; int 10 func 1130 - get font information
;-----------------------------------------------------------------------------
int_10_1130:
	mov	ds, [cs:seg0040]

	mov	cx, [BIOS_CHRH]			; character height
	mov	dl, [BIOS_ROWS]			; number of rows

	push	cs
	pop	es

	cmp	bh, 0x01
	jb	.int1f
	je	.int43

	cmp	bh, 0x03
	jb	.fnt8x14
	je	.fnt8x8

	cmp	bh, 0x05
	jb	.fnt8x8_2

	xor	bp, bp

	jmp	.done

.int1f:
	xor	bp, bp
	mov	es, bp
	les	bp, [es:4 * 0x1f]
	jmp	.done

.int43:
	xor	bp, bp
	mov	es, bp
	les	bp, [es:4 * 0x43]
	jmp	.done

.fnt8x14:
	mov	bp, fnt_8x14
	jmp	.done

.fnt8x8:
	mov	bp, fnt_8x8
	jmp	.done

.fnt8x8_2:
	mov	bp, fnt_8x8 + 8 * 128
	jmp	.done

.done:
	ret


int_10_12:
	cmp	bl, 0x10
	je	int_10_12_10
	ret


;-----------------------------------------------------------------------------
; int 10 func 1210 - get ega config
;-----------------------------------------------------------------------------
int_10_12_10:
	mov	bh, 0				; monitor type
	mov	bl, 3				; ram size
	mov	ch, 0				; feature connector
	mov	cl, 9				; switches
	ret


int_10_13:
	ret


int_10_14:
int_10_15:
int_10_16:
int_10_17:
int_10_18:
int_10_19:
	ret


int_10_1a:
	ret


;-----------------------------------------------------------------------------
; int 10
;-----------------------------------------------------------------------------
int_10:
	sti
	cld

	push	si
	push	ds

	push	ax
	mov	al, ah
	mov	ah, 0
	shl	ax, 1
	mov	si, ax
	pop	ax

	cmp	si, (int_10_funcend - int_10_func)
	jae	.badfunc

	call	word [cs:si + int_10_func]
	jmp	.done

.badfunc:

.done:
	pop	ds
	pop	si
	iret


;-----------------------------------------------------------------------------
; Print a string
;
; inp: CS:SI = pointer to ASCIIZ string
;-----------------------------------------------------------------------------
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


;-----------------------------------------------------------------------------
; EGA initialization
;-----------------------------------------------------------------------------
ega_init:
	push	ax
	push	si
	push	ds

	xor	ax, ax
	mov	ds, ax

	; PCE specific check for presence of EGA
	mov	al, [0x0410]
	and	al, 0x30
	jnz	.done

	cli

	mov	ax, [4 * 0x10]
	mov	[4 * 0x42], ax
	mov	ax, [4 * 0x10 + 2]
	mov	[4 * 0x42 + 2], ax

	mov	word [4 * 0x10], int_10
	mov	word [4 * 0x10 + 2], cs

	mov	word [4 * 0x1f], fnt_8x8 + 8 * 128
	mov	word [4 * 0x1f + 2], cs

	mov	word [4 * 0x43], fnt_8x8
	mov	word [4 * 0x43 + 2], cs

	mov	ds, [cs:seg0040]

	mov	word [0x00a8], ptr00a8
	mov	word [0x00a8 + 2], cs

	sti

	mov	byte [0x0087], 0x60
	mov	byte [0x0088], 0x09		; switches

	mov	ax, 0x0003
	int	0x10

	mov	si, msg_init
	call	prt_string

.done:
	pop	ds
	pop	si
	pop	ax
	ret


	times	16383 - ($ - $$) db 0xff

	db	0x00

rom_end:
