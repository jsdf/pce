;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     src/arch/ibmpc/bios/vga.asm                                *
;* Created:       2004-03-26 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2004-03-28 by Hampa Hug <hampa@hampa.ch>                   *
;* Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


%define VGA_BIOS_CS 0xc000

%define BIOS_MODE 0x0049
%define BIOS_COLS 0x004a
%define BIOS_SIZE 0x004c
%define BIOS_OFFS 0x004e
%define BIOS_CPOS 0x0050
%define BIOS_CSIZ 0x0060
%define BIOS_PAGE 0x0062
%define BIOS_CRTC 0x0063
%define BIOS_ROWS 0x0084
%define BIOS_CHRH 0x0085

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

%define DAC_CMSK     0x03c6
%define DAC_WRAD     0x03c7
%define DAC_RDAD     0x03c8
%define DAC_RGBV     0x03c9
%define DAC_STAT     0x03c7


section .text


;-----------------------------------------------------------------------------

rom_start:
  dw      0xaa55
  db      (rom_end - rom_start + 511) / 512

init:
  jmp     start

  set_pos 0x1d
  db      " IBM "

start:
  push    ax

  pceh    PCEH_GET_VIDEO
  cmp     ax, 4
  jne     .done

  call    ega_init

.done
  pop     ax
  retf


msg_init  db "PCE VGA BIOS version ", PCE_VERSION_STR
          db " (", PCE_CFG_DATE, " ", PCE_CFG_TIME, ")"
          db 13, 10, 13, 10, 0


seg0000   dw 0x0000
seg0040   dw 0x0040
sega000   dw 0xa000
segb000   dw 0xb000
segb800   dw 0xb800

cursor14  db 0, 2, 4, 6, 8, 10, 11, 13, 14

modemap   db 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x06, 0x07
          db 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0d, 0x0d
          db 0x0d, 0x0d, 0x0d, 0x13

pal_ega:
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x00

pal_vga:
%include "pal13.inc"


mode00:
  db      40, 24, 14
  dw      2000
  db      0x00, 0x03, 0x00, 0x02                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xa3
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x00, 0x00, 0x0f, 0x00
  db      0x0f, 0x0f, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00        ; gdc
  db      0x00

mode01:
  db      40, 24, 14
  dw      2000
  db      0x00, 0x03, 0x00, 0x02                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0xa3
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x00, 0x00, 0x0f, 0x00
  db      0x0f, 0x0f, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00        ; gdc
  db      0x00

mode02:
  db      80, 24, 14
  dw      4000
  db      0x00, 0x03, 0x00, 0x02                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xa3
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x00, 0x00, 0x0f, 0x00
  db      0x0f, 0x0f, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00        ; gdc
  db      0x00

mode03:
  db      80, 24, 14
  dw      4000
  db      0x00, 0x03, 0x00, 0x02                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xa3
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x00, 0x00, 0x0f, 0x00
  db      0x0f, 0x0f, 0x00, 0x00, 0x00, 0x10, 0x0e, 0x00        ; gdc
  db      0x00

mode04:
  db      40, 24, 8
  dw      8192
  db      0x00, 0x03, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01
  db      0x00
  db      0x00, 0x1b, 0x2d, 0x3f, 0x00, 0x03, 0x04, 0x06        ; atc
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x01, 0x00, 0x03, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0x00        ; gdc
  db      0xff

mode05:
  db      40, 24, 8
  dw      8192
  db      0x00, 0x03, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01
  db      0x00
  db      0x00, 0x1b, 0x2d, 0x3f, 0x00, 0x03, 0x04, 0x06        ; atc
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x01, 0x00, 0x03, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0x00        ; gdc
  db      0xff

mode06:
  db      80, 24, 8
  dw      8192
  db      0x00, 0x01, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01
  db      0x00
  db      0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; atc
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x01, 0x00, 0x01, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00        ; gdc
  db      0xff

mode07:
  db      80, 24, 14
  dw      4000
  db      0x00, 0x03, 0x00, 0x02                                ; ts
  db      0x42                                                  ; misc out
  db      0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x0d, 0x0b, 0x0d, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xa3
  db      0x00
  db      0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07        ; atc
  db      0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
  db      0x00, 0x00, 0x0f, 0x00
  db      0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x0a, 0xa3        ; gdc
  db      0x00

mode08:
  times   64 db 0x00

mode09:
  times   64 db 0x00

mode0a:
  times   64 db 0x00

mode0b:
  times   64 db 0x00

mode0c:
  times   64 db 0x00

mode0d:
  db      40, 24, 8
  dw      8192
  db      0x00, 0x00, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x01, 0x00, 0x0f, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; gdc
  db      0x00

mode0e:
  db      80, 24, 8
  dw      16384
  db      0x00, 0x00, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x01, 0x00, 0x0f, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; gdc
  db      0x00

mode0f:
  db      80, 24, 14
  dw      28000
  db      0x00, 0x00, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x01, 0x00, 0x05, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; gdc
  db      0x00

mode10:
  db      80, 24, 14
  dw      28000
  db      0x00, 0x00, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x01, 0x00, 0x0f, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; gdc
  db      0x00


mode11:
  db      80, 24, 16
  dw      38400
  db      0x00, 0x00, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x01, 0x00, 0x0f, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; gdc
  db      0x00


mode12:
  db      80, 24, 16
  dw      38400
  db      0x00, 0x00, 0x00, 0x06                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07        ; atc
  db      0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
  db      0x01, 0x00, 0x0f, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; gdc
  db      0x00

mode13:
  db      40, 24, 8
  dw      64000
  db      0x00, 0x0f, 0x00, 0x0e                                ; ts
  db      0x43                                                  ; misc out
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        ; crtc
  db      0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00
  db      0x00, 0x00, 0x00, 0x28, 0x60, 0x00, 0x00, 0x00
  db      0x00
  db      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07        ; atc
  db      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
  db      0x01, 0x00, 0x0f, 0x00
  db      0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00        ; gdc
  db      0xff


; display combination code table
dcct:
  db      0x01                          ; entry count
  db      0x00                          ; version
  db      0x08                          ; max val
  db      0x00                          ; reserved

  db      0x08, 0x00


; secondary video save pointer table
vspt2:
  dw      0x1a
  dw      dcct, VGA_BIOS_CS
  dw      0, 0                          ; second alphanumeric character set override
  dw      0, 0                          ; user palette profile table
  dw      0, 0
  dw      0, 0
  dw      0, 0


ptr00a8:
  dw      mode00, VGA_BIOS_CS           ; video parameter table
  dw      pal_ega, VGA_BIOS_CS          ; parameter dynamic save area
  dw      0, 0                          ; alphanumeric character set override
  dw      0, 0                          ; graphics character set override
  dw      vspt2, VGA_BIOS_CS            ; secondary video save pointer table
  dw      0, 0
  dw      0, 0


%include "fnt8x8.inc"
%include "fnt8x14.inc"
%include "fnt8x16.inc"


int10_00_func:
  dw      int10_00_00
  dw      int10_00_01
  dw      int10_00_02
  dw      int10_00_03
  dw      int10_00_04
  dw      int10_00_05
  dw      int10_00_06
  dw      int10_00_07
  dw      int10_00_08
  dw      int10_00_09
  dw      int10_00_0a
  dw      int10_00_0b
  dw      int10_00_0c
  dw      int10_00_0d
  dw      int10_00_0e
  dw      int10_00_0f
  dw      int10_00_10
  dw      int10_00_11
  dw      int10_00_12
  dw      int10_00_13
int10_00_funcend:

int10_10_func:
  dw      int10_1000
  dw      int10_1001
  dw      int10_1002
  dw      int10_1003
  dw      int10_1004
  dw      int10_1005
  dw      int10_1006
  dw      int10_1007
  dw      int10_1008
  dw      int10_1009
  dw      int10_100a
  dw      int10_100b
  dw      int10_100c
  dw      int10_100d
  dw      int10_100e
  dw      int10_100f
  dw      int10_1010
  dw      int10_1011
  dw      int10_1012
  dw      int10_1013
  dw      int10_1014
  dw      int10_1015
  dw      int10_1016
  dw      int10_1017
  dw      int10_1018
  dw      int10_1019
  dw      int10_101a
  dw      int10_101b
int10_10_funcend:

int10_func:
  dw      int10_00
  dw      int10_01
  dw      int10_02
  dw      int10_03
  dw      int10_04
  dw      int10_05
  dw      int10_06
  dw      int10_07
  dw      int10_08
  dw      int10_09
  dw      int10_0a
  dw      int10_0b
  dw      int10_0c
  dw      int10_0d
  dw      int10_0e
  dw      int10_0f
  dw      int10_10
  dw      int10_11
  dw      int10_12
  dw      int10_13
  dw      int10_14
  dw      int10_15
  dw      int10_16
  dw      int10_17
  dw      int10_18
  dw      int10_19
  dw      int10_1a
  dw      int10_1b
  dw      int10_1c
int10_funcend:


; write AH into register AL at DX
ega_set_reg:
  out     dx, ax
  ret


; write CX bytes from DS:SI into index/data DX, starting at AL
ega_set_regs:
  push    ax
  push    cx
  push    si

.next:
  mov     ah, [si]
  inc     si
  out     dx, ax
  inc     al
  loop    .next

  pop     si
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------

; set page offset AX
crtc_set_page_ofs:
  push    ax
  push    cx
  push    dx

  mov     cx, ax
  mov     dx, CRTC_INDEX
  mov     al, CRTC_OFFS_HI
  mov     ah, ch
  out     dx, ax
  mov     al, CRTC_OFFS_LO
  mov     ah, cl
  out     dx, ax

  pop     dx
  pop     cx
  pop     ax
  ret

; set cursor position AX
crtc_set_curs_pos:
  push    ax
  push    cx
  push    dx

  mov     cx, ax
  mov     dx, CRTC_INDEX
  mov     al, CRTC_CPOS_HI
  mov     ah, ch
  out     dx, ax
  mov     al, CRTC_CPOS_LO
  mov     ah, cl
  out     dx, ax

  pop     dx
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------

; set AH into ATC register AL
atc_set_reg:
  push    ax
  push    cx
  push    dx

  mov     cx, ax

  cli

  mov     dx, 0x3da
  in      al, dx

  mov     dx, ATC_INDEX
  mov     al, cl
  and     al, 0x1f
  out     dx, al
  mov     al, ch
  out     dx, al

  mov     al, 0x20
  out     dx, al

  sti

  pop     dx
  pop     cx
  pop     ax
  ret

; read ATC register AL into AH
atc_get_reg:
  push    cx
  push    dx

  mov     cl, al

  cli

  mov     dx, 0x3da                     ; clear flip/flop
  in      al, dx

  mov     dx, ATC_INDEX
  mov     al, cl
  and     al, 0x1f
  out     dx, al                        ; index

  inc     dx
  in      al, dx                        ; read register
  mov     ch, al

  mov     dx, 0x3da                     ; clear flip/flop
  in      al, dx

  mov     dx, ATC_INDEX
  mov     al, 0x20
  out     dx, al

  sti

  mov     ax, cx

  pop     dx
  pop     cx
  ret

; write CX bytes from DS:SI starting at AL
atc_set_regs:
  push    ax
  push    cx
  push    si

.next:
  mov     ah, [si]
  inc     si
  call    atc_set_reg

  inc     al
  loop    .next

  pop     si
  pop     cx
  pop     ax
  ret

; read CX bytes to ES:DI starting at AL
atc_get_regs:
  push    ax
  push    cx
  push    di

  cli

.next:
  call    atc_get_reg
  mov     [es:di], ah
  inc     di
  loop    .next

  pop     di
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------

; write [AH DH DL] into DAC index AL
dac_set_color:
  push    ax
  push    cx
  push    dx

  mov     cx, dx

  cli
  mov     dx, DAC_WRAD
  out     dx, al

  mov     dx, DAC_RGBV
  mov     al, ah
  out     dx, al
  mov     al, ch
  out     dx, al
  mov     al, cl
  out     dx, al
  sti

  pop     dx
  pop     cx
  pop     ax
  ret

; read DAC entry AL into [AH DH DL]
dac_get_color:
  push    cx

  cli

  mov     dx, DAC_RDAD
  out     dx, al

  mov     dx, DAC_RGBV
  in      al, dx
  mov     ah, al
  in      al, dx
  mov     ch, al
  in      al, dx
  mov     cl, al

  sti

  mov     dx, cx

  pop     cx
  ret

; write CX DAC entries from DS:SI starting at AL
dac_set_colors:
  push    ax
  push    cx
  push    dx
  push    si

.next:
  mov     ah, [si]
  mov     dh, [si + 1]
  mov     dl, [si + 2]
  call    dac_set_color
  add     si, 3
  inc     al
  loop    .next

  pop     si
  pop     dx
  pop     cx
  pop     ax
  ret

; read CX DAC entries starting at AL to ES:DI
dac_get_colors:
  push    ax
  push    cx
  push    dx
  push    di

.next:
  call    dac_get_color
  mov     [es:di], ah
  mov     [es:di + 1], dh
  mov     [es:di + 2], dl
  add     di, 3
  inc     al
  loop    .next

  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------

; get mode in AH
get_modemap:
  push    bx
  mov     bl, [BIOS_MODE]
  and     bx, 0x007f
  mov     ah, [cs:bx + modemap]
  pop     bx
  ret


; get video mem segment in AX
get_segm:
  mov     ax, 0xb800
  cmp     word [BIOS_CRTC], 0x03b4
  jne     .done

  mov     ah, 0xb0

.done:
  ret


; Get page AL offset in AX
get_pofs:
  push    dx
  and     ax, 0x0007
  mul     word [BIOS_SIZE]
  pop     dx
  shl     ax, 1
  ret

; convert position (AL, AH) into addr in AX
get_cofs:
  push    cx
  push    dx

  mov     cx, ax

  mov     al, ah                        ; row
  mov     ah, 0
  mul     word [BIOS_COLS]

  mov     ch, 0
  add     ax, cx
  shl     ax, 1

  pop     dx
  pop     cx
  ret

; set character AL CX times at (DL, DH) in page BH
txt_set_char_xy:
  jcxz    .done

  xchg    al, bh
  call    get_pofs
  mov     di, ax

  mov     ax, dx
  call    get_cofs
  add     di, ax

  call    get_segm
  mov     es, ax

.next:
  mov     [es:di], bh
  add     di, 2
  loop    .next

.done:
  ret


; set character AL CX times at DL, DH in page BH with color BL
ega_set_char_xyc:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    bp
  push    ds
  push    es

  jcxz    .done

  mul     byte [BIOS_CHRH]
  mov     si, ax                        ; font index

  mov     al, bh
  call    get_pofs
  mov     di, ax

  push    dx
  mov     al, dh
  mul     byte [BIOS_COLS]
  mul     word [BIOS_CHRH]
  pop     dx
  add     di, ax

  mov     dh, 0
  add     di, dx

  mov     bh, [BIOS_CHRH]
  mov     bp, [BIOS_COLS]

  mov     dx, TS_INDEX
  mov     ax, (0x0f << 8) | TS_WRPL
  out     dx, ax

  mov     dx, GDC_INDEX
  mov     ax, 0x0205
  out     dx, ax

  mov     es, [cs:sega000]

  mov     ds, [cs:seg0000]
  lds     ax, [4 * 0x43]                ; pointer to font
  add     si, ax

.next:
  mov     ax, (0xff << 8) | GDC_BMSK
  out     dx, ax                        ; set bit mask

  mov     byte [es:di], 0x00            ; clear background
  mov     al, [es:di]                   ; fill latches

  mov     ah, [si]
  mov     al, GDC_BMSK
  out     dx, ax
  mov     al, bl

  push    cx
  push    di
  rep     stosb
  pop     di
  pop     cx

  inc     si
  add     di, bp
  dec     bh
  jnz     .next

  mov     ax, (0xff << 8) | GDC_BMSK
  out     dx, ax                        ; reset bit mask

  mov     dx, GDC_INDEX
  mov     ax, 0x0005
  out     dx, ax                        ; reset read/write mode

.done:
  pop     es
  pop     ds
  pop     bp
  pop     di
  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


; set character AL CX times at DL, DH in page BH with color BL
vga_set_char_xyc:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    bp
  push    ds
  push    es

  jcxz    .done

  mul     byte [BIOS_CHRH]
  mov     si, ax                        ; font index

  mov     al, bh
  call    get_pofs
  mov     di, ax                        ; paget offset

  push    dx
  mov     al, dh                        ; y
  mul     byte [BIOS_COLS]
  mul     word [BIOS_CHRH]
  pop     dx
  mov     dh, 0
  add     ax, dx
  shl     ax, 1
  shl     ax, 1
  shl     ax, 1
  add     di, ax

  mov     bh, [BIOS_CHRH]
  mov     bp, [BIOS_COLS]
  shl     bp, 1
  shl     bp, 1
  shl     bp, 1
  mov     es, [cs:sega000]

  mov     ds, [cs:seg0000]
  lds     ax, [4 * 0x43]                ; pointer to font
  add     si, ax

.next_row:
  mov     ah, [si]                      ; font byte

  push    cx
  push    di

  shl     cx, 1
  shl     cx, 1
  shl     cx, 1                         ; pixel count

.next_pixel:
  mov     al, 0x00
  test    ah, 0x80
  jz      .black

  mov     al, bl

.black:
  stosb

  rol     ah, 1
  loop    .next_pixel

  pop     di
  pop     cx

  inc     si
  add     di, bp
  dec     bh
  jnz     .next_row

.done:
  pop     es
  pop     ds
  pop     bp
  pop     di
  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


; scroll entire text screen up one line
txt_scroll_up_1:
  push    ax
  push    cx
  push    dx
  push    si
  push    di
  push    ds
  push    es

  mov     al, [BIOS_PAGE]
  call    get_pofs

  mov     dx, [BIOS_COLS]

  mov     si, ax
  add     si, dx
  add     si, dx

  mov     di, ax

  mov     cx, [BIOS_SIZE]
  shr     cx, 1
  sub     cx, dx

  call    get_segm
  mov     ds, ax
  mov     es, ax

  rep     movsw

  mov     ax, 0x0720
  mov     cx, dx
  rep     stosw

  pop     es
  pop     ds
  pop     di
  pop     si
  pop     dx
  pop     cx
  pop     ax
  ret


ega_scroll_up_1:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    ds
  push    es

  mov     al, [BIOS_PAGE]
  call    get_pofs
  mov     di, ax
  mov     si, ax

  mov     ax, [BIOS_COLS]
  mul     word [BIOS_CHRH]
  mov     bx, ax                        ; row size in bytes
  mov     dl, [BIOS_ROWS]
  mov     dh, 0
  mul     dx
  mov     cx, ax                        ; page size in bytes

  add     si, bx

  mov     dx, GDC_INDEX
  mov     ax, (0x01 << 8) | GDC_MODE
  out     dx, ax                        ; write mode 1

  mov     dx, TS_INDEX
  mov     ax, (0x0f << 8) | TS_WRPL
  out     dx, ax

  mov     ds, [cs:sega000]
  mov     es, [cs:sega000]

  rep     movsb

  mov     dx, GDC_INDEX
  mov     ax, (0x02 << 8) | GDC_MODE
  out     dx, ax

  mov     ax, (0xff << 8) | GDC_BMSK
  out     dx, ax

  mov     al, 0
  mov     cx, bx
  rep     stosb

  mov     dx, GDC_INDEX
  mov     ax, (0x00 << 8) | GDC_MODE
  out     dx, ax

  pop     es
  pop     ds
  pop     di
  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


vga_scroll_up_1:
  push    ax
  push    cx
  push    dx
  push    si
  push    di

  mov     ax, [BIOS_COLS]
  mul     word [BIOS_CHRH]
  shl     ax, 1
  shl     ax, 1
  shl     ax, 1

  mov     cx, [BIOS_SIZE]
  sub     cx, ax

  mov     ds, [cs:sega000]
  mov     si, ax

  mov     es, [cs:sega000]
  xor     di, di

  rep     movsb

  mov     cx, ax
  mov     al, 0x00
  rep     stosb

  pop     di
  pop     si
  pop     dx
  pop     cx
  pop     ax
  ret


; Clear the rectangle (CL, CH) - (DL, DH) with AX
txt_clear_rect:
  push    ax
  push    cx
  push    dx
  push    di
  push    es

  sub     dh, ch
  jb      .done
  inc     dh
  sub     dl, cl
  jb      .done
  inc     dl

  push    ax
  mov     al, [BIOS_PAGE]
  call    get_pofs
  mov     di, ax

  mov     ax, cx
  call    get_cofs
  add     di, ax

  call    get_segm
  mov     es, ax
  pop     ax

.next:
  mov     cl, dl
  mov     ch, 0
  push    di
  rep     stosw
  pop     di

  mov     cx, [BIOS_COLS]
  shl     cx, 1
  add     di, cx

  dec     dh
  jnz     .next

.done:
  pop     es
  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


; clear CX words at B800:DI with AX
vga_clear_cga:
  push    cx
  push    di
  push    es

  mov     es, [cs:segb800]
  rep     stosw

  pop     es
  pop     di
  pop     cx
  ret


; clear CX bytes at A000:DI with color AL
vga_clear_ega:
  push    cx
  push    dx
  push    di
  push    es

  push    ax
  mov     dx, GDC_INDEX
  mov     ax, (0x02 << 8) | GDC_MODE
  out     dx, ax

  mov     ax, (0x00 << 8) | GDC_FSEL
  out     dx, ax

  mov     ax, (0xff << 8) | GDC_BMSK
  out     dx, ax

  mov     dx, TS_INDEX
  mov     ax, (0x0f << 8) | TS_WRPL
  out     dx, ax
  pop     ax

  mov     es, [cs:sega000]
  rep     stosb

  pop     es
  pop     di
  pop     dx
  pop     cx
  ret


; clear CX bytes at A000:DI with color AL
vga_clear:
  push    cx
  push    di
  push    es

  mov     es, [cs:sega000]
  rep     stosb

  pop     es
  pop     di
  pop     cx
  ret


; init mode AL with parameter table DS:SI
int10_init_mode:
  push    ax
  push    cx
  push    dx
  push    bx
  push    di
  push    es

  mov     dx, 0x03d0
  out     dx, al

  mov     es, [cs:seg0040]

  mov     [es:BIOS_MODE], al

  mov     bx, si

  lea     si, [bx + 10]
  mov     al, 0x00
  mov     cx, 25
  mov     dx, CRTC_INDEX
  call    ega_set_regs

  lea     si, [bx + 5]
  mov     al, 0x01
  mov     cx, 5
  mov     dx, TS_INDEX
  call    ega_set_regs

  lea     si, [bx + 35]
  mov     al, 0x00
  mov     cx, 20
  mov     dx, ATC_INDEX
  call    atc_set_regs

  lea     si, [bx + 55]
  mov     al, 0x00
  mov     cx, 9
  mov     dx, GDC_INDEX
  call    ega_set_regs

  mov     dx, 0x3cc
  mov     al, [bx + 9]
  out     dx, al                        ; misc output register

  mov     al, [bx]
  mov     ah, 0
  mov     [es:BIOS_COLS], ax

  mov     al, [bx + 1]
  mov     [es:BIOS_ROWS], al

  mov     al, [bx + 2]
  mov     ah, 0
  mov     [es:BIOS_CHRH], ax

  mov     ax, [bx + 3]
  mov     [es:BIOS_SIZE], ax

  mov     word [es:BIOS_OFFS], 0

  mov     di, BIOS_CPOS
  mov     cx, 8
  xor     ax, ax
  rep     stosw

  mov     word [es:BIOS_CSIZ], 0x0607
  mov     byte [es:BIOS_PAGE], 0x00
  mov     word [es:BIOS_CRTC], 0x03d4

  xor     ax, ax
  mov     es, ax

  mov     dx, cs

  mov     al, [bx + 2]                  ; character height

  cmp     al, 8
  jbe     .fnt8x8

  cmp     al, 14
  jbe     .fnt8x14

  jmp     .fnt8x16

.fnt8x8:
  mov     ax, fnt_8x8
  jmp     .fntdone

.fnt8x14:
  mov     ax, fnt_8x14
  jmp     .fntdone

.fnt8x16:
  mov     ax, fnt_8x16

.fntdone:
  mov     word [es:4 * 0x43], ax
  mov     word [es:4 * 0x43 + 2], dx

  pop     es
  pop     di
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


int10_00_00:
int10_00_01:
int10_00_02:
int10_00_03:
  push    ax
  push    cx
  push    di

  mov     ax, 0x0720
  xor     di, di
  mov     cx, 8192
  call    vga_clear_cga

  pop     di
  pop     cx
  pop     ax
  ret


int10_00_04:
int10_00_05:
int10_00_06:
  push    ax
  push    cx
  push    di

  xor     ax, ax
  xor     di, di
  mov     cx, 8192
  call    vga_clear_cga

  pop     di
  pop     cx
  pop     ax
  ret


int10_00_07:
  push    ax
  push    cx
  push    di

  mov     word [BIOS_CRTC], 0x03b4

  mov     ax, 0x0720
  xor     di, di
  mov     cx, 8000
;  call    txt_clear

  pop     di
  pop     cx
  pop     ax
  ret


int10_00_08:
int10_00_09:
int10_00_0a:
int10_00_0b:
int10_00_0c:
  pceh    PCEH_STOP
  ret


int10_00_0d:
int10_00_0e:
int10_00_0f:
int10_00_10:
int10_00_11:
int10_00_12:
  push    ax
  push    cx
  push    dx
  push    di

  test    al, 0x80
  jnz     .noclear

  mov     al, 0
  xor     di, di
  mov     cx, 64000
  call    vga_clear_ega

  mov     dx, GDC_INDEX
  mov     ax, (0x00 << 8) | GDC_MODE
  out     dx, ax

.noclear:
  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret

int10_00_13:
  push    ax
  push    cx
  push    dx
  push    di

  push    cs
  pop     ds

  mov     al, 0
  mov     cx, 256
  mov     si, pal_vga
  call    dac_set_colors

  test    al, 0x80
  jnz     .noclear

  mov     al, 0
  xor     di, di
  mov     cx, 64000
  call    vga_clear

.noclear:
  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 00 - set video mode
; inp: AL = video mode
;-----------------------------------------------------------------------------
int10_00:
  push    si

  push    ax
  and     al, 0x7f
  cmp     al, 0x13
  pop     ax
  ja      .badfunc

  push    cs
  pop     ds

  push    ax
  and     al, 0x7f
  mov     ah, al
  mov     al, 0
  shr     ax, 1
  shr     ax, 1
  add     ax, mode00
  mov     si, ax
  pop     ax

  cmp     byte [si], 0x00
  je      .badfunc

  call    int10_init_mode

  push    ax
  and     al, 0x7f
  mov     ah, 0
  mov     si, ax
  pop     ax

  shl     si, 1
  cmp     si, (int10_00_funcend - int10_00_func)
  jae     .badfunc

  mov     ds, [cs:seg0040]

  call    [cs:si + int10_00_func]
  jmp     .done

.badfunc:
  pceh    PCEH_STOP

.done:
  pop     si
  ret


;-----------------------------------------------------------------------------
; int 10 func 01 - set cursor size
; inp: CH = start line
;      CL = end line
;-----------------------------------------------------------------------------
int10_01:
  push    ax
  push    cx
  push    dx
  push    bx

  mov     ds, [cs:seg0040]

  cmp     cl, 0x07
  jbe     .clok
  mov     cl, 0x08

.clok:
  cmp     ch, 0x07
  jbe     .chok
  mov     ch, 0x08

.chok:
  mov     [BIOS_CSIZ], cx

  mov     bh, 0
  mov     bl, ch
  mov     ch, [cs:bx + cursor14]
  mov     bl, cl
  mov     cl, [cs:bx + cursor14]

  mov     dx, [BIOS_CRTC]
  mov     al, CRTC_CSIZ_HI
  mov     ah, ch
  out     dx, ax

  mov     al, CRTC_CSIZ_LO
  mov     ah, cl
  out     dx, ax

  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 02 - set cursor position
; inp: DH = row
;      DL = column
;-----------------------------------------------------------------------------
int10_02:
  push    ax
  push    dx
  push    bx

  mov     ds, [cs:seg0040]

  mov     al, bh                        ; page

  mov     bl, bh
  and     bx, 0x0007
  shl     bx, 1

  mov     [bx + BIOS_CPOS], dx

  cmp     al, [BIOS_PAGE]
  jne     .done

  call    get_pofs
  mov     bx, ax

  mov     ax, dx
  call    get_cofs
  add     bx, ax
  shr     bx, 1

  mov     dx, [BIOS_CRTC]
  mov     al, CRTC_CPOS_HI
  mov     ah, bh
  out     dx, ax

  mov     al, CRTC_CPOS_LO
  mov     ah, bl
  out     dx, ax

.done:
  pop     bx
  pop     dx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 03 - get cursor position and size
; out: CH cursor start line
;      CL cursor end line
;      DH = cursor row
;      DL = cursor column
;-----------------------------------------------------------------------------
int10_03:
  push    bx

  mov     ds, [cs:seg0040]

  mov     bl, bh
  and     bx, 0x0007
  shl     bx, 1

  mov     cx, [BIOS_CSIZ]
  mov     dx, [bx + BIOS_CPOS]

  pop     bx
  ret


; int 10 func 04 - get lightpen position
int10_04:
  mov     ah, 0
  ret


;-----------------------------------------------------------------------------
; int 10 func 05 - set current page
; inp: AL = page number
;-----------------------------------------------------------------------------
int10_05:
  push    ax
  push    cx
  push    dx
  push    bx

  mov     ds, [cs:seg0040]

  mov     [BIOS_PAGE], al

  call    get_pofs
  mov     cx, ax
  shr     cx, 1

  mov     dx, [BIOS_CRTC]
  mov     al, CRTC_OFFS_HI
  mov     ah, ch
  out     dx, ax
  mov     al, CRTC_OFFS_LO
  mov     ah, cl
  out     dx, ax

  mov     bl, [BIOS_PAGE]
  mov     bh, 0
  shl     bx, 1
  mov     ax, [bx + BIOS_CPOS]
  call    get_cofs
  shr     ax, 1
  add     cx, ax

  mov     dx, [BIOS_CRTC]
  mov     al, CRTC_CPOS_HI
  mov     ah, ch
  out     dx, ax
  mov     al, CRTC_CPOS_LO
  mov     ah, cl
  out     dx, ax

  pop     bx
  pop     dx
  pop     cx
  pop     ax
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
int10_06_cga_txt:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    bp
  push    es

  or      al, al
  jz      .clear

  push    dx
  sub     dh, ch
  inc     dh
  cmp     al, dh
  pop     dx

  jae     .clear
  jmp     .scroll

.clear:
  mov     ah, bh
  mov     al, 0x20
  call    txt_clear_rect
  jmp     .done

.scroll:
  sub     dl, cl
  inc     dl                            ; window width
  sub     dh, ch
  inc     dh                            ; window height

  push    ax
  mov     al, [BIOS_PAGE]
  call    get_pofs
  mov     di, ax

  mov     ax, cx
  call    get_cofs
  add     di, ax                        ; dest in DI
  pop     ax

  mov     bp, [BIOS_COLS]
  shl     bp, 1

  push    ax
  push    dx
  mov     ah, 0
  mul     bp
  mov     si, di
  add     si, ax                        ; source in SI
  pop     dx
  pop     ax

  mov     bl, dh
  sub     bl, al                        ; rows to copy

  push    ax
  call    get_segm
  mov     ds, ax
  mov     es, ax
  pop     ax

.copy:
  push    si
  push    di

  mov     cl, dl
  mov     ch, 0
  rep     movsw

  pop     di
  pop     si

  add     si, bp
  add     di, bp

  dec     bl
  jnz     .copy

  mov     bl, al                        ; rows to clear
  mov     ah, bh
  mov     al, 0x20

.clearrow:
  push    di
  mov     cl, dl
  mov     ch, 0
  rep     stosw
  pop     di

  add     di, bp

  dec     bl
  jnz     .clearrow

.done:
  pop     es
  pop     bp
  pop     di
  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret

int10_06:
  push    ax

  mov     ds, [cs:seg0040]

  call    get_modemap

  cmp     ah, 0x00
  je      .mode00

  cmp     ah, 0x04
  jb      .done
  cmp     ah, 0x06
  jbe     .mode04

  jmp     .done

.mode00:
  call    int10_06_cga_txt
  jmp     .done

.mode04:
  pop     ax
  int     0x42
  ret

.done:
  pop     ax
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
int10_07_cga_txt:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    bp
  push    es

  or      al, al
  jz      .clear

  push    dx
  sub     dh, ch
  inc     dh
  cmp     al, dh
  pop     dx

  jae     .clear
  jmp     .scroll

.clear:
  mov     ah, bh
  mov     al, 0x20
  call    txt_clear_rect
  jmp     .done

.scroll:
  sub     dl, cl
  inc     dl                            ; window width
  sub     dh, ch
  inc     dh                            ; window height

  push    ax
  mov     al, [BIOS_PAGE]
  call    get_pofs
  mov     di, ax

  mov     ax, cx
  add     ah, dh
  dec     ah
  call    get_cofs
  add     di, ax                        ; dest in DI
  pop     ax

  mov     bp, [BIOS_COLS]
  shl     bp, 1

  push    ax
  push    dx
  mov     ah, 0
  mul     bp
  mov     si, di
  sub     si, ax                        ; source in SI
  pop     dx
  pop     ax

  mov     bl, dh
  sub     bl, al                        ; rows to copy

  push    ax
  call    get_segm
  mov     ds, ax
  mov     es, ax
  pop     ax

.copy:
  push    si
  push    di

  mov     cl, dl
  mov     ch, 0
  rep     movsw

  pop     di
  pop     si

  sub     si, bp
  sub     di, bp

  dec     bl
  jnz     .copy

  mov     bl, al                        ; rows to clear
  mov     ah, bh
  mov     al, 0x20

.clearrow:
  push    di
  mov     cl, dl
  mov     ch, 0
  rep     stosw
  pop     di

  sub     di, bp

  dec     bl
  jnz     .clearrow

.done:
  pop     es
  pop     bp
  pop     di
  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret

int10_07:
  push    ax

  mov     ds, [cs:seg0040]

  call    get_modemap

  cmp     ah, 0x00
  je      .mode00

  cmp     ah, 0x04
  jb      .done
  cmp     ah, 0x06
  jbe     .mode04

  jmp     .done

.mode00:
  call    int10_07_cga_txt
  jmp     .done

.mode04:
  pop     ax
  int     0x42
  ret

.done:
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 08 - get character and attribute
; inp: BH = page
; out: AH = attribute
;      AL = character
;-----------------------------------------------------------------------------
int10_08:
  push    bx
  push    si
  push    es

  mov     ds, [cs:seg0040]

  call    get_segm
  mov     es, ax

  mov     al, bh
  call    get_pofs
  mov     si, ax

  mov     bl, bh
  and     bx, 0x0007
  shl     bx, 1
  mov     ax, [bx + BIOS_CPOS]
  call    get_cofs
  add     si, ax

  mov     ax, [es:si]

  pop     es
  pop     si
  pop     bx
  ret


;-----------------------------------------------------------------------------
; int 10 func 09 - set character and attribute
; inp: AL = character
;      BH = page
;      BL = attribute
;      CX = count
;-----------------------------------------------------------------------------
int10_09_mode03:
  push    cx
  push    bx
  push    di
  push    es

  mov     ah, bl

  push    ax

  call    get_segm
  mov     es, ax

  mov     al, bh
  call    get_pofs
  mov     di, ax

  mov     bl, bh
  and     bx, 0x0007
  shl     bx, 1
  mov     ax, [bx + BIOS_CPOS]
  call    get_cofs
  add     di, ax

  pop     ax

  rep     stosw

  pop     es
  pop     di
  pop     bx
  pop     cx
  ret

int10_09_mode0d:
  push    dx

  push    bx
  mov     bl, [BIOS_PAGE]
  and     bx, 0x0007
  mov     dx, [bx + BIOS_CPOS]
  pop     bx

  call    ega_set_char_xyc

  pop     dx
  ret

int10_09:
  push    ax

  mov     ds, [cs:seg0040]

  call    get_modemap

  cmp     ah, 0x00
  je      .mode00

  cmp     ah, 0x0d
  je      .mode0d

  cmp     ah, 0x04
  jb      .done
  cmp     ah, 0x06
  jbe     .mode04

  jmp     .done

.mode00:
  call    int10_09_mode03
  jmp     .done

.mode0d:
  call    int10_09_mode0d
  jmp     .done

.mode04:
  pop     ax
  int     0x42
  ret

.done:
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 0a - set character
; inp: AL = character
;      BH = page
;      CX = count
;-----------------------------------------------------------------------------
int10_0a_mode03:
  push    ax
  push    cx
  push    bx
  push    di
  push    es

  jcxz    .done

  mov     ds, [cs:seg0040]

  push    ax

  call    get_segm
  mov     es, ax

  mov     al, bh
  call    get_pofs
  mov     di, ax

  mov     bl, bh
  and     bx, 0x0007
  shl     bx, 1
  mov     ax, [bx + BIOS_CPOS]
  call    get_cofs
  add     di, ax

  pop     ax

.next:
  stosb
  inc     di
  loop    .next

.done:
  pop     es
  pop     di
  pop     bx
  pop     cx
  pop     ax
  ret

int10_0a:
  push    ax

  mov     ds, [cs:seg0040]

  push    bx
  mov     bl, [BIOS_MODE]
  and     bx, 0x007f
  mov     ah, [cs:bx + modemap]
  pop     bx

  cmp     ah, 0x00
  jne     .not00

  call    int10_0a_mode03
  jmp     .done

.not00:
  cmp     ah, 0x0d
  jne     .not0d

;  call    int10_0e_mode0d
  jmp     .done

.not0d:
  pop     ax
  push    ax

  int     0x42

.done:
  pop     ax
  ret


; int 10 func 0b - set background color
int10_0b:
  ret


;-----------------------------------------------------------------------------
; int 10 func 0c - set pixel
; inp: AL = color
;      BH = page
;      CX = x
;      DX = y
;-----------------------------------------------------------------------------
int10_0c:
  push    ax
  push    cx
  push    dx
  push    bx
  push    es

  mov     ds, [cs:seg0040]

  mov     bl, al                        ; color

  mov     ax, dx
  mul     word [BIOS_COLS]
  mov     dl, cl                        ; bit index
  mov     dh, bl                        ; color
  shr     cx, 1
  shr     cx, 1
  shr     cx, 1
  add     cx, ax

  mov     al, bh
  call    get_pofs
  mov     bx, ax
  add     bx, cx

  mov     cx, dx

  mov     dx, TS_INDEX
  mov     ax, (0x0f << 8) | TS_WRPL
  out     dx, ax                        ; enable all write planes

  mov     dx, GDC_INDEX
  mov     ax, (0x02 << 8) | GDC_MODE
  out     dx, ax                        ; set write mode 2

  and     cl, 0x07                      ; bit index
  mov     ah, 0x80
  shr     ah, cl
  mov     al, GDC_BMSK
  out     dx, ax                        ; set bit mask

  mov     ax, 0xa000
  mov     es, ax

  mov     al, [es:bx]                   ; fill latches
  mov     [es:bx], ch                   ; set pixel

  mov     ax, (0x00 << 8) | GDC_MODE
  out     dx, ax                        ; reset mode

  mov     ax, (0xff << 8) | GDC_BMSK
  out     dx, ax                        ; reset bit mask

  pop     es
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 0d - get pixel
; inp: BH = page
;      CX = x
;      DX = y
; out: AL = color
;-----------------------------------------------------------------------------
int10_0d:
  ret


;-----------------------------------------------------------------------------
; int 10 func 0e - print character
; inp: AL = character
;      BL = color
;-----------------------------------------------------------------------------
int10_0e:
  push    ax

  mov     ds, [cs:seg0040]

  push    bx
  mov     bl, [BIOS_MODE]
  and     bx, 0x007f
  mov     ah, [cs:bx + modemap]
  pop     bx

  cmp     ah, 0x00
  jne     .not00

  call    int10_0e_mode03
  jmp     .done

.not00:
  cmp     ah, 0x0d
  jne     .not0d

  call    int10_0e_mode0d
  jmp     .done

.not0d:
  cmp     ah, 0x13
  jne     .not13

  call    int10_0e_mode13
  jmp     .done

.not13:
  pop     ax
  push    ax

  int     0x42

.done:
  pop     ax
  ret


int10_0e_mode13:
  push    cx
  push    dx
  push    bx
  push    di
  push    es

  mov     ah, bl                        ; save color

  mov     bl, [BIOS_PAGE]
  and     bx, 0x0007
  shl     bx, 1

  mov     dx, [bx + BIOS_CPOS]

  cmp     al, 0x0a
  jne     .notlf

  inc     dh
  cmp     dh, 25
  jb      .ok

  call    vga_scroll_up_1
  mov     dh, 24
  jmp     .ok

.notlf:
  cmp     al, 0x0d
  jne     .notcr

  mov     dl, 0
  jmp     .ok

.notcr:
  cmp     al, 0x08
  jne     .notbs

  or      dl, dl
  jz      .ok

  dec     dl
  jmp     .ok

.notbs:
  push    bx
  mov     bh, [BIOS_PAGE]
  mov     bl, ah
  mov     cx, 1
  call    vga_set_char_xyc
  pop     bx

  inc     dl
  cmp     dl, [BIOS_COLS]
  jb      .ok

  mov     dl, 0
  inc     dh
  cmp     dh, 25
  jb      .ok

  call    vga_scroll_up_1
  mov     dh, 24

.ok:
  mov     [bx + BIOS_CPOS], dx

  pop     es
  pop     di
  pop     bx
  pop     dx
  pop     cx
  ret

int10_0e_mode0d:
  push    cx
  push    dx
  push    bx
  push    di
  push    es

  mov     ah, bl                        ; save color

  mov     bl, [BIOS_PAGE]
  and     bx, 0x0007
  shl     bx, 1

  mov     dx, [bx + BIOS_CPOS]

  cmp     al, 0x0a
  jne     .notlf

  inc     dh
  cmp     dh, 25
  jb      .ok

  call    ega_scroll_up_1
  mov     dh, 24
  jmp     .ok

.notlf:
  cmp     al, 0x0d
  jne     .notcr

  mov     dl, 0
  jmp     .ok

.notcr:
  cmp     al, 0x08
  jne     .notbs

  or      dl, dl
  jz      .ok

  dec     dl
  jmp     .ok

.notbs:
  push    bx
  mov     bh, [BIOS_PAGE]
  mov     bl, ah
  mov     cx, 1
  call    ega_set_char_xyc
  pop     bx

  inc     dl
  cmp     dl, [BIOS_COLS]
  jb      .ok

  mov     dl, 0
  inc     dh
  cmp     dh, 25
  jb      .ok

  call    ega_scroll_up_1
  mov     dh, 24

.ok:
  mov     [bx + BIOS_CPOS], dx

  pop     es
  pop     di
  pop     bx
  pop     dx
  pop     cx
  ret

int10_0e_mode03:
  push    cx
  push    dx
  push    bx
  push    di
  push    es

  mov     bl, [BIOS_PAGE]
  and     bx, 0x0007
  shl     bx, 1

  mov     dx, [bx + BIOS_CPOS]

  cmp     al, 0x0a
  jne     .notlf

  inc     dh
  cmp     dh, 25
  jb      .ok

  call    txt_scroll_up_1
  mov     dh, 24
  jmp     .ok

.notlf:
  cmp     al, 0x0d
  jne     .notcr

  mov     dl, 0
  jmp     .ok

.notcr:
  cmp     al, 0x08
  jne     .notbs

  or      dl, dl
  jz      .ok

  dec     dl
  jmp     .ok

.notbs:
  mov     cl, al

  call    get_segm
  mov     es, ax

  mov     al, [BIOS_PAGE]
  call    get_pofs
  mov     di, ax

  mov     ax, [bx + BIOS_CPOS]
  call    get_cofs
  add     di, ax

  mov     [es:di], cl

  mov     dx, [bx + BIOS_CPOS]
  inc     dl
  cmp     dl, [BIOS_COLS]
  jb      .ok

  mov     dl, 0
  inc     dh
  cmp     dh, 25
  jb      .ok

  call    txt_scroll_up_1
  mov     dh, 24

.ok:
  mov     [bx + BIOS_CPOS], dx

  mov     ax, dx
  call    get_cofs
  shr     ax, 1

  mov     cx, ax
  mov     dx, [BIOS_CRTC]
  mov     al, CRTC_CPOS_HI
  mov     ah, ch
  out     dx, ax
  mov     al, CRTC_CPOS_LO
  mov     ah, cl
  out     dx, ax

  pop     es
  pop     di
  pop     bx
  pop     dx
  pop     cx
  ret


;-----------------------------------------------------------------------------
; int 10 func 0f - get video mode
; out: AL = video mode
;      AH = columns
;      BH = current page
;-----------------------------------------------------------------------------
int10_0f:
  mov     ds, [cs:seg0040]
  mov     al, [BIOS_MODE]
  mov     ah, [BIOS_COLS]
  mov     bh, [BIOS_PAGE]
  ret


;-----------------------------------------------------------------------------
; int 10 func 10 - palette
;-----------------------------------------------------------------------------
int10_10:
  push    si

  mov     si, ax
  and     si, 0x00ff
  shl     si, 1

  cmp     si, int10_10_funcend - int10_10_func
  ja      .done

  call    [cs:si + int10_10_func]

.done:
  pop     si
  ret


int10_1004:
int10_1005:
int10_1006:
int10_100a:
int10_100b:
int10_100c:
int10_100d:
int10_100e:
int10_100f:
int10_1011:
int10_1013:
int10_1014:
int10_1016:
int10_101a:
int10_101b:
  ret


;-----------------------------------------------------------------------------
; int 10 func 1000 - set palette register
; inp: BL = register index
;      BH = new value
;-----------------------------------------------------------------------------
int10_1000:
  push    ax
  mov     al, bh
  mov     ah, bl
  mov     ax, bx
  call    atc_set_reg

  mov     ds, [cs:seg0040]
  lds     si, [0x00a8]
  lds     si, [si + 4]
  mov     ax, ds
  or      ax, si
  jz      .done

  mov     al, bl
  mov     ah, 0
  add     si, ax
  mov     [si], bh

.done:
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1001 - set overscan color
; inp: BH = new value
;-----------------------------------------------------------------------------
int10_1001:
  push    ax
  mov     ah, bh
  mov     al, ATC_OSCN
  call    atc_set_reg
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1002 - set entire palette
; inp: ES:DX pointer to palette
;-----------------------------------------------------------------------------
int10_1002:
  push    ax
  push    cx
  push    si

  mov     al, 0
  mov     cx, 17
  push    es
  pop     ds
  mov     si, dx
  call    atc_set_regs

  pop     si
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1003 - enable blinking
; inp: BL = blink enable
;-----------------------------------------------------------------------------
int10_1003:
  ret


;-----------------------------------------------------------------------------
; int 10 func 1007 - get palette register
;
; inp: BL = register index
; out: BH = value
;-----------------------------------------------------------------------------
int10_1007:
  push    ax
  mov     al, bl
  call    atc_get_reg
  mov     bh, ah
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1008 - get overscan color
; out: BH = overscan color
;-----------------------------------------------------------------------------
int10_1008:
  push    ax
  mov     al, ATC_OSCN
  call    atc_get_reg
  mov     bh, ah
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1009 - get entire palette
; inp: ES:DX pointer to 17 byte buffer
;-----------------------------------------------------------------------------
int10_1009:
  push    ax
  push    cx
  push    di

  mov     di, dx
  mov     al, 0
  mov     cx, 17
  call    atc_get_regs

  pop     di
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1010 - set dac entry
; inp: BX = index
;      CH = green
;      CL = blue
;      DH = red
;-----------------------------------------------------------------------------
int10_1010:
  push    ax
  push    dx

  mov     al, bl
  mov     ah, dh
  mov     dx, cx
  call    dac_set_color

  pop     dx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1012 - set multiple dac entries
; inp: BX = index
;      CX = count
;      ES:DX = buffer
;-----------------------------------------------------------------------------
int10_1012:
  push    ax
  push    si

  mov     al, bl
  push    es
  pop     ds
  mov     si, dx
  call    dac_set_colors

  pop     si
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1015 - get dac entry
; inp: BX = index
; out: CH = green
;      CL = blue
;      DH = red
;-----------------------------------------------------------------------------
int10_1015:
  push    ax
  push    dx

  mov     al, bl
  call    dac_get_color

  mov     cx, dx

  pop     dx
  mov     dh, ah
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1017 - get multiple dac entries
; inp: BX = index
;      CX = count
;      ES:DX = buffer
;-----------------------------------------------------------------------------
int10_1017:
  push    ax
  push    di

  mov     al, bl
  mov     di, dx
  call    dac_get_colors

  pop     di
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1018 - set dac color mask
; inp: BL = color mask
;-----------------------------------------------------------------------------
int10_1018:
  push    ax
  push    dx

  mov     dx, DAC_CMSK
  mov     al, bl
  out     dx, al

  pop     dx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 1019 - get dac color mask
; out: BL = color mask
;-----------------------------------------------------------------------------
int10_1019:
  push    ax
  push    dx

  mov     dx, DAC_CMSK
  in      al, dx
  mov     bl, al

  pop     dx
  pop     ax
  ret


;-----------------------------------------------------------------------------
; int 10 func 11
;-----------------------------------------------------------------------------
int10_11:
  cmp     al, 0x30
  je      int10_1130
  ret


; int 10 func 1130 - get font information
int10_1130:
  mov     cx, 14
  mov     dl, 24

  push    cs
  pop     es

  cmp     bh, 0x01
  jb      .int1f
  je      .int43

  cmp     bh, 0x03
  jb      .fnt8x14
  je      .fnt8x8

  cmp     bh, 0x05
  jb      .fnt8x8_2
  je      .fnt9x14

  cmp     bh, 0x07
  jb      .fnt8x16
  je      .fnt9x16

  jmp     .done

.int1f:
  xor     bp, bp
  mov     es, bp
  les     bp, [es:4 * 0x1f]
  jmp     .done

.int43:
  xor     bp, bp
  mov     es, bp
  les     bp, [es:4 * 0x43]
  jmp     .done

.fnt8x14:
  mov     bp, fnt_8x14
  jmp     .done

.fnt8x8:
  mov     bp, fnt_8x8
  jmp     .done

.fnt8x8_2:
  mov     bp, fnt_8x8 + 8 * 128
  jmp     .done

.fnt8x16:
.fnt9x14:
.fnt9x16:
  jmp     .done

.done:
  ret


int10_12:
  cmp     bl, 0x10
  je      int10_12_10
  ret


; int 10 func 1210 - get ega config
int10_12_10:
  mov     bh, 0                 ; monitor type
  mov     bl, 3                 ; ram size
  mov     ch, 0                 ; featrue connector
  mov     cl, 7                 ; switches
  ret


int10_13:
int10_14:
int10_15:
int10_16:
int10_17:
int10_18:
int10_19:
int10_1b:
int10_1c:
  pceh    PCEH_STOP
  ret


;-----------------------------------------------------------------------------
; int 10 func 11
;-----------------------------------------------------------------------------
int10_1a:
  cmp     al, 0x01
  jb      int10_1a00
  je      int10_1a01

  ret


;-----------------------------------------------------------------------------
; int 10 func 1a00 - get video adapter info
; out: AL = 0x1a
;      BL = primary video adapter
;      BH = secondary video adapter
;-----------------------------------------------------------------------------
int10_1a00:
  mov     al, 0x1a
  mov     bx, 0x0008
  ret


;-----------------------------------------------------------------------------
; int 10 func 1a01 - set video adapter info
; inp: BL = primary video adapter
;      BH = secondary video adapter
; out: AL = 0x1a
;-----------------------------------------------------------------------------
int10_1a01:
  mov     al, 0x1a
  ret


int10:
  push    si
  push    ds

  cld

  push    ax
  mov     al, ah
  mov     ah, 0
  shl     ax, 1
  mov     si, ax
  pop     ax

  cmp     si, (int10_funcend - int10_func)
  jae     .badfunc

  call    word [cs:si + int10_func]
  jmp     .done

.badfunc:
;  pceh    PCEH_STOP

.done:
  pop     ds
  pop     si
  iret


; print string at CS:SI
prt_string:
  push    ax
  push    bx
  push    si

  xor     bx, bx

.next
  cs      lodsb
  or      al, al
  jz      .done

  mov     ah, 0x0e
  int     0x10

  jmp     short .next

.done
  pop     si
  pop     bx
  pop     ax
  ret


ega_init:
  push    ax
  push    si
  push    ds

  xor     ax, ax
  mov     ds, ax

  mov     ax, [4 * 0x10]
  mov     [4 * 0x42], ax
  mov     ax, [4 * 0x10 + 2]
  mov     [4 * 0x42 + 2], ax

  mov     word [4 * 0x10], int10
  mov     word [4 * 0x10 + 2], cs

  mov     word [4 * 0x1f], fnt_8x8 + 8 * 128
  mov     word [4 * 0x1f + 2], cs

  mov     word [4 * 0x43], fnt_8x8
  mov     word [4 * 0x43 + 2], cs

  mov     ds, [cs:seg0040]

  mov     byte [0x0087], 0x60
  mov     byte [0x0088], 0x09           ; switches

  mov     word [0x00a8], ptr00a8
  mov     word [0x00a8 + 2], cs

  mov     ax, 0x0003
  int     0x10

  mov     si, msg_init
  call    prt_string

  pop     ds
  pop     si
  pop     ax
  ret


  times 16383 - ($ - $$) db 0xff

  db      0x00

rom_end:
