;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     ega.asm                                                    *
;* Created:       2003-09-14 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-09-14 by Hampa Hug <hampa@hampa.ch>                   *
;* Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

; $Id: ega.asm,v 1.2 2003/09/19 14:48:36 hampa Exp $


%include "config.inc"
%include "hook.inc"


%macro set_pos 1
  times %1 - ($ - $$) db 0
%endmacro


%define BIOS_MODE 0x0049
%define BIOS_COLS 0x004a
%define BIOS_SIZE 0x004c
%define BIOS_OFFS 0x004e
%define BIOS_CPOS 0x0050
%define BIOS_CSIZ 0x0060
%define BIOS_PAGE 0x0062
%define BIOS_CRTC 0x0063

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

%define PAL_R1 0x04
%define PAL_G1 0x02
%define PAL_B1 0x01
%define PAL_R0 0x20
%define PAL_G0 0x10
%define PAL_B0 0x08


section .text


;-----------------------------------------------------------------------------

rom_start:
  dw      0xaa55
  db      (rom_end - rom_start + 511) / 512

start:
  push    ax

  pcehook PCEH_GET_VIDEO
  cmp     ax, 4
  jne     .done

  call    ega_init

.done
  pop     ax
  retf


seg0040   dw 0x0040
sega000   dw 0xa000
segb000   dw 0xb000
segb800   dw 0xb800

bios_ds   dw 0x0040

cursor14  db 0, 1, 2, 3, 4, 5, 11, 12

pal_default:
  db      0x00
  db      PAL_B1
  db      PAL_G1
  db      PAL_G1 + PAL_B1
  db      PAL_R1
  db      PAL_R1 + PAL_B1
  db      PAL_R1 + PAL_G1
  db      PAL_R1 + PAL_G1 + PAL_B1

  db      PAL_R0 + PAL_G0 + PAL_B0
  db      PAL_B1 + PAL_B0
  db      PAL_G1 + PAL_G0
  db      (PAL_G1 + PAL_B1) + (PAL_G0 + PAL_B0)
  db      (PAL_R1) + (PAL_R0)
  db      (PAL_R1 + PAL_B1) + (PAL_R0 + PAL_B0)
  db      (PAL_R1 + PAL_G1) + (PAL_R0 + PAL_G0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)

pal_mono:
  db      0x00
  db      PAL_R1 + PAL_G1 + PAL_B1
  db      PAL_R1 + PAL_G1 + PAL_B1
  db      PAL_R1 + PAL_G1 + PAL_B1
  db      PAL_R1 + PAL_G1 + PAL_B1
  db      PAL_R1 + PAL_G1 + PAL_B1
  db      PAL_R1 + PAL_G1 + PAL_B1
  db      PAL_R1 + PAL_G1 + PAL_B1

  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)
  db      (PAL_R1 + PAL_G1 + PAL_B1) + (PAL_R0 + PAL_G0 + PAL_B0)


%include "fnt8x8.inc"
%include "fnt8x14.inc"


int_10_00_func:
  dw      int_10_00_00
  dw      int_10_00_01
  dw      int_10_00_02
  dw      int_10_00_03
  dw      int_10_00_04
  dw      int_10_00_05
  dw      int_10_00_06
  dw      int_10_00_07
  dw      int_10_00_08
  dw      int_10_00_09
  dw      int_10_00_0a
  dw      int_10_00_0b
  dw      int_10_00_0c
  dw      int_10_00_0d
  dw      int_10_00_0e
  dw      int_10_00_0f
  dw      int_10_00_10
int_10_00_funcend:


int_10_func:
  dw      int_10_00
  dw      int_10_01
  dw      int_10_02
  dw      int_10_03
  dw      int_10_04
  dw      int_10_05
  dw      int_10_06
  dw      int_10_07
  dw      int_10_08
  dw      int_10_09
  dw      int_10_0a
  dw      int_10_0b
  dw      int_10_0c
  dw      int_10_0d
  dw      int_10_0e
  dw      int_10_0f
  dw      int_10_10
  dw      int_10_11
  dw      int_10_12
  dw      int_10_13
  dw      int_10_14
  dw      int_10_15
  dw      int_10_16
  dw      int_10_17
  dw      int_10_18
  dw      int_10_19
  dw      int_10_1a
int_10_funcend:


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


; write AH into register AL
atc_set_reg:
  push    ax
  push    dx

  push    ax
  mov     dx, 0x3da
  in      al, dx
  pop      ax

  mov     dx, ATC_INDEX
  or      al, 0x20
  out     dx, al
  mov     al, ah
  out     dx, al

  pop     dx
  pop     ax
  ret


; set AH into palette register AL
atc_set_pal_reg:
  push    ax
  push    cx
  push    dx

  mov     cx, ax

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

  pop     dx
  pop     cx
  pop     ax
  ret


; set CX palette entries from DS:SI starting at AL
atc_set_palette:
  push    ax
  push    cx
  push    dx
  push    si

  mov     ah, al

.next:
  mov     dx, 0x03da
  in      al, dx                        ; reset index/data flipflop

  mov     dx, ATC_INDEX
  mov     al, ah
  and     al, 0x0f
  out     dx, al                        ; set index

  lodsb
  out     dx, al                        ; set value

  inc     ah
  loop    .next

  mov     dx, 0x03da
  in      al, dx

  mov     dx, ATC_INDEX
  mov     al, 0x20
  out     dx, al

  pop     si
  pop     dx
  pop     cx
  pop     ax
  ret


atc_set_palette_default:
  push    ax
  push    cx
  push    si
  push    ds

  mov     al, 0
  mov     cx, 16
  push    cs
  pop     ds
  mov     si, pal_default
  call    atc_set_palette

  pop     ds
  pop     si
  pop     cx
  pop     ax
  ret


atc_set_palette_mono:
  push    ax
  push    cx
  push    si
  push    ds

  mov     al, 0
  mov     cx, 16
  push    cs
  pop     ds
  mov     si, pal_mono
  call    atc_set_palette

  pop     ds
  pop     si
  pop     cx
  pop     ax
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
txt_clear:
  push    cx
  push    es

  push    ax
  call    get_segm
  mov     es, ax
  pop     ax

  rep     stosw

  pop     es
  pop     cx
  ret


; clear CX bytes at A000:DI with color AL
gra_clear:
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


int_10_00_00:
int_10_00_01:
  mov     [BIOS_MODE], al
  ret


int_10_00_02:
  call    int_10_00_03
  mov     byte [BIOS_MODE], 0x02
  ret


int_10_00_03:
  push    ax
  push    cx
  push    dx
  push    di

  mov     dx, 0x03d0
  mov     al, 3
  out     dx, al

  mov     byte [BIOS_MODE], 0x03
  mov     word [BIOS_COLS], 80
  mov     word [BIOS_SIZE], 4000
  mov     word [BIOS_OFFS], 0

  mov     di, BIOS_CPOS
  mov     cx, 8
.next:
  mov     word [di], 0x0000
  add     di, 2
  loop    .next

  mov     word [BIOS_CSIZ], 0x0607
  mov     byte [BIOS_PAGE], 0x00
  mov     word [BIOS_CRTC], 0x03d4

  ; cursor size
  mov     dx, 0x03d4
  mov     ax, 0x0b0a
  out     dx, ax
  mov     ax, 0x0c0b
  out     dx, ax

  mov     dx, 0x3c2
  mov     al, 0xc2
  out     dx, al

  mov     dx, CRTC_INDEX
  mov     ax, (0x28 << 8) | CRTC_ROFS
  out     dx, ax

  xor     ax, ax
  call    crtc_set_curs_pos

  xor     ax, ax
  call    crtc_set_page_ofs

  call    atc_set_palette_default

  mov     ax, 0x0720
  xor     di, di
  mov     cx, 8000
  call    txt_clear

  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


int_10_00_07:
  push    ax
  push    cx
  push    dx
  push    di

  mov     dx, 0x03d0
  mov     al, 7
  out     dx, al

  mov     byte [BIOS_MODE], 0x07
  mov     word [BIOS_COLS], 80
  mov     word [BIOS_SIZE], 4000
  mov     word [BIOS_OFFS], 0

  mov     di, BIOS_CPOS
  mov     cx, 8
.next:
  mov     word [di], 0x0000
  add     di, 2
  loop    .next

  mov     word [BIOS_CSIZ], 0x0607
  mov     byte [BIOS_PAGE], 0x00
  mov     word [BIOS_CRTC], 0x03b4

  ; cursor size
  mov     dx, 0x03b4
  mov     ax, 0x0b0a
  out     dx, ax
  mov     ax, 0x0c0b
  out     dx, ax

  mov     dx, 0x3c2
  mov     al, 0xc2
  out     dx, al

  mov     dx, 0x3b4
  mov     ax, (0x28 << 8) | CRTC_ROFS
  out     dx, ax

  xor     ax, ax
  call    crtc_set_curs_pos

  xor     ax, ax
  call    crtc_set_page_ofs

  call    atc_set_palette_mono

  mov     ax, 0x0720
  xor     di, di
  mov     cx, 8000
  call    txt_clear

  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


int_10_00_04:
int_10_00_05:
int_10_00_06:
int_10_00_08:
int_10_00_09:
int_10_00_0a:
int_10_00_0b:
int_10_00_0c:
  mov     [BIOS_MODE], al
  ret


int_10_00_0d:
  push    ax
  push    cx
  push    dx
  push    di

  mov     dx, 0x03d0
  mov     al, 0x0d
  out     dx, al

  mov     byte [BIOS_MODE], 0x0d
  mov     word [BIOS_COLS], 40
  mov     word [BIOS_SIZE], 8000
  mov     word [BIOS_OFFS], 0

  mov     word [BIOS_CSIZ], 0x0607
  mov     byte [BIOS_PAGE], 0x00
  mov     word [BIOS_CRTC], 0x03d4

  mov     dx, 0x3c2
  mov     al, 0xc2
  out     dx, al

  mov     dx, CRTC_INDEX
  mov     ax, (0x14 << 8) | CRTC_ROFS
  out     dx, ax

  xor     ax, ax
  call    crtc_set_page_ofs

  call    atc_set_palette_default

  mov     al, 0
  xor     di, di
  mov     cx, 64000
  call    gra_clear

  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


int_10_00_0e:
  push    ax
  push    cx
  push    dx
  push    di

  mov     dx, 0x03d0
  mov     al, 0x0e
  out     dx, al

  mov     byte [BIOS_MODE], 0x0e
  mov     word [BIOS_COLS], 80
  mov     word [BIOS_SIZE], 16000
  mov     word [BIOS_OFFS], 0

  mov     word [BIOS_CSIZ], 0x0607
  mov     byte [BIOS_PAGE], 0x00
  mov     word [BIOS_CRTC], 0x03d4

  mov     dx, 0x3c2
  mov     al, 0xc2
  out     dx, al

  mov     dx, CRTC_INDEX
  mov     ax, (0x28 << 8) | CRTC_ROFS
  out     dx, ax

  xor     ax, ax
  call    crtc_set_page_ofs

  call    atc_set_palette_default

  mov     al, 0
  xor     di, di
  mov     cx, 64000
  call    gra_clear

  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


int_10_00_0f:
  call    int_10_00_10
  mov     byte [BIOS_MODE], 0x0f
  ret


int_10_00_10:
  push    ax
  push    cx
  push    dx
  push    di

  mov     dx, 0x03d0
  mov     al, 0x10
  out     dx, al

  mov     byte [BIOS_MODE], 0x10
  mov     word [BIOS_COLS], 80
  mov     word [BIOS_SIZE], 28000
  mov     word [BIOS_OFFS], 0

  mov     word [BIOS_CSIZ], 0x0607
  mov     byte [BIOS_PAGE], 0x00
  mov     word [BIOS_CRTC], 0x03d4

  mov     dx, 0x3c2
  mov     al, 0xc2
  out     dx, al

  mov     dx, CRTC_INDEX
  mov     ax, (0x28 << 8) | CRTC_ROFS
  out     dx, ax

  xor     ax, ax
  call    crtc_set_page_ofs

  call    atc_set_palette_default

  mov     al, 0
  xor     di, di
  mov     cx, 2 * 28000
  call    gra_clear

  pop     di
  pop     dx
  pop     cx
  pop     ax
  ret


; int 10 func 00 - set video mode
int_10_00:
  push    si

  mov     ds, [cs:seg0040]

  push     ax
  mov     ah, 0
  mov     si, ax
  pop     ax

  shl     si, 1
  cmp     si, (int_10_00_funcend - int_10_00_func)
  jae     .badfunc

  call    [cs:si + int_10_00_func]

.badfunc:

.done:
  pop     si
  ret


; int 10 func 01 - set cursor size
int_10_01:
  push    ax
  push    cx
  push    dx
  push    bx

  mov     ds, [cs:bios_ds]

  and     cx, 0x0707
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


; int 10 func 02 - set cursor position
int_10_02:
  push    ax
  push    dx
  push    bx

  mov     ds, [cs:bios_ds]

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


; int 10 func 03 - get cursor position and size
int_10_03:
  push    bx

  mov     ds, [cs:bios_ds]

  mov     bl, bh
  and     bx, 0x0007
  shl     bx, 1

  mov     cx, [BIOS_CSIZ]
  mov     dx, [bx + BIOS_CPOS]

  pop     bx
  ret


; int 10 func 04 - get lightpen position
int_10_04:
  mov     ah, 0
  ret


; int 10 func 05 - set current page
int_10_05:
  push    ax
  push    cx
  push    dx
  push    bx

  mov     ds, [cs:bios_ds]

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


; int 10 func 06 - scroll up
int_10_06:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    bp
  push    es

  mov     ds, [cs:seg0040]

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


; int 10 func 07 - scroll down
int_10_07:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si
  push    di
  push    bp
  push    es

  mov     ds, [cs:seg0040]

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
  mov     ah, 0
  mul     bp
  mov     si, di
  add     si, ax                        ; source in SI
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


; int 10 func 08 - get character and attribute
int_10_08:
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


; int 10 func 09 - set character and attribute
int_10_09:
  push    ax
  push    cx
  push    bx
  push    di
  push    es

  mov     ds, [cs:bios_ds]

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
  pop     ax
  ret


; int 10 func 0a - set character
int_10_0a:
  push    ax
  push    cx
  push    bx
  push    di
  push    es

  jcxz    .done

  mov     ds, [cs:bios_ds]

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


; int 10 func 0b - set background color
int_10_0b:
  ret


; int 10 func 0c - set pixel
int_10_0c:
  ret


; int 10 func 0d - get pixel
int_10_0d:
  ret


; int 10 func 0e - print character
int_10_0e:
  push    ax
  push    cx
  push    dx
  push    bx
  push    di
  push    es

  mov     ds, [cs:bios_ds]

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
  pop     ax
  ret


; int 10 func 0f - get video mode
int_10_0f:
  mov     ds, [cs:bios_ds]
  mov     al, [BIOS_MODE]
  mov     ah, [BIOS_COLS]
  mov     bh, [BIOS_PAGE]
  ret


int_10_10:
  or      al, al
  je      int_10_1000

  cmp     al, 0x02
  je      int_10_1002

  cmp     al, 0x03
  je      int_10_1003

  pcehook PCEH_STOP
  ret


; int 10 func 1000 - set palette register
int_10_1000:
  push    ax
  mov     al, bh
  mov     ah, bl
  call    atc_set_pal_reg
  pop     ax
  ret


; int 10 func 1001 - set overscan color
int_10_1001:
  push    ax
  mov     ah, bh
  mov     al, ATC_OSCN
  call    atc_set_reg
  pop     ax
  ret


; int 10 func 1002 - set entire palette
int_10_1002:
  push    ax
  push    cx
  push    si

  mov     al, 0
  mov     cx, 16
  push    es
  pop     ds
  mov     si, dx
  call    atc_set_palette

  pop     si
  pop     cx
  pop     ax
  ret


; int 10 func 1003 - enable blinking
int_10_1003:
  ret


int_10_11:
  cmp     al, 0x30
  je      int_10_1130

  pcehook PCEH_STOP
  ret


; int 10 func 1130 - get font information
int_10_1130:
  mov     es, [cs:seg0040]
  mov     cx, 14
  mov     dl, [es:BIOS_COLS]
  dec     dl

  push    cs
  pop     es

  cmp     bh, 2
  jne     .not8x14
  mov     bp, fnt_8x14
  jmp     .done

.not8x14
  cmp     bh, 3
  jne     .not8x8
  mov     bp, fnt_8x8
  jmp     .done

.not8x8:
  cmp     bh, 4
  jne     .not8x8_2
  mov     bp, fnt_8x8 + 8 * 128
  jmp     .done

.not8x8_2:
  xor     bp, bp

.done:
  ret


int_10_12:
  cmp     bl, 0x10
  je      int_10_12_10

  pcehook PCEH_STOP
  ret


; int 10 func 1210 - get ega config
int_10_12_10:
  mov     bh, 0                 ; monitor type
  mov     bl, 3                 ; ram size
  ret


int_10_13:
  pcehook PCEH_STOP
  ret


int_10_14:
int_10_15:
int_10_16:
int_10_17:
int_10_18:
int_10_19:
  pcehook PCEH_STOP
  ret


int_10_1a:
  or      al, al
  jz      int_10_1a00

  cmp     al, 1
  je      int_10_1a01

  ret


; int 10 func 1a00 - get video adapter
int_10_1a00:
  mov     al, 0x1a
  mov     bl, 0x04
  mov     bh, 0x00
  ret


; int 10 func 1a01 - set video adapter
int_10_1a01:
  mov     al, 0x1a
  ret


int_10:
;  pcehook PCEH_STOP

  push    si
  push    ds

  cld

  push    ax
  mov     al, ah
  mov     ah, 0
  shl     ax, 1
  mov     si, ax
  pop     ax

  cmp     si, (int_10_funcend - int_10_func)
  jae     .done

  call    word [cs:si + int_10_func]

.done:
  pop     ds
  pop     si
  iret


ega_init:
  push    ds

  xor     ax, ax
  mov     ds, ax

  mov     word [4 * 0x10], int_10
  mov     word [4 * 0x10 + 2], cs

  pop     ds
  ret


rom_end:
