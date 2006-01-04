;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     src/arch/ibmpc/bios/pce.asm                                *
;* Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2006-01-04 by Hampa Hug <hampa@hampa.ch>                   *
;* Copyright:     (C) 2003-2006 Hampa Hug <hampa@hampa.ch>                   *
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


section .text


;-----------------------------------------------------------------------------

init:
  jmp     start


msg_init       db "PC BIOS version ", PCE_VERSION_STR
               db " (", PCE_CFG_DATE, " ", PCE_CFG_TIME, ")"
               db 13, 10, 13, 10, 0

msg_memchk1    db "Memory size:    ", 0
msg_memchk2    db "KB", 13, 0

msg_serial     db "Serial ports:   ", 0
msg_parallel   db "Parallel ports: ", 0
msg_video      db "Video adapter:  ", 0
msg_mda        db "MDA", 0
msg_cga        db "CGA", 0

msg_rom1       db "ROM[", 0
msg_rom2       db "]:", 0
msg_cksmok     db " checksum ok", 0
msg_cksmbad    db " bad checksum", 0


start:
  cli
  mov     ax, 0x0050
  mov     ss, ax
  mov     sp, 1024

  call    set_bios_ds

  call    init_data
  call    init_int
  call    init_pic
  call    init_pit
  call    init_ppi
  call    init_dma

  sti

  call    init_rom1
  call    init_video
  call    init_mem
  call    init_misc
  call    init_keyboard
  call    init_serport
  call    init_parport
  call    init_time
  call    init_biosdata

  call    prt_nl

  push    cs
  pop     ds

  int     0x19
  int     0x18

done:
  jmp     done


;-----------------------------------------------------------------------------

init_data:
  push    ax
  push    cx
  push    di
  push    es

  cld

  push    ds
  pop     es

  xor     di, di
  xor     ax, ax
  mov     cx, 256 / 2
  rep     stosw

  pop     es
  pop     di
  pop     cx
  pop     ax
  ret


init_biosdata:
  push    ax
  push    ds

  mov     ax, 0x0040
  mov     ds, ax

  pceh    PCEH_GET_HDCNT
  mov     [0x0075], al

  pop     ds
  pop     ax
  ret


init_int:
  push    ax
  push    cx
  push    si
  push    di
  push    es
  push    ds

  mov     ax, 0xf000
  mov     ds, ax
  mov     si, inttab

  xor     di, di
  mov     es, di

  mov     cx, 32 * 2
  cld
  rep     movsw

  mov     cx, 256 - 32

.next:
  mov     ax, int_default
  stosw
  mov     ax, 0xf000
  stosw
  loop    .next

  ; set int 0x40 == int 0x13
  mov     ax, [es:4 * 0x13 + 0]
  mov     cx, [es:4 * 0x13 + 2]
  mov     [es:4 * 0x40 + 0], ax
  mov     [es:4 * 0x40 + 2], cx

  pop     ds
  pop     es
  pop     di
  pop     si
  pop     cx
  pop     ax
  ret


init_pic:
  mov     al, 0x13
  out     0x20, al

  mov     al, 0x08
  out     0x21, al

  mov     al, 0x01
  out     0x21, al

  mov     al, 0x00
  out     0x21, al
  ret


init_pit:
  mov     al, 0x34
  out     0x43, al
  mov     al, 0
  out     0x40, al
  out     0x40, al
  ret


init_ppi:
  push    ax
  mov     al, 0x99
  out     0x63, al                      ; set up ppi ports

  mov     al, 0xfc
  out     0x61, al

  in      al, 0x60                      ; get config word
  mov     ah, 0x00

  mov     [0x0010], ax                  ; equipment word

  mov     al, 0x7c
  out     0x61, al

  pop     ax
  ret


init_dma:
  push    ax

  mov     al, 0
  out     0x0d, al                      ; master clear

  out     0x0c, al                      ; clear flip flop

  out     0x00, al                      ; address
  out     0x00, al

  mov     al, 0xff
  out     0x01, al                      ; count
  out     0x01, al

  mov     al, 0x58                      ; mode channel 0
  out     0x0b, al

  mov     al, 0x41
  out     0x0b, al

  inc     ax
  out     0x0b, al

  inc     ax
  out     0x0b, al

  pop     ax
  ret


init_video:
  push    ax
  push    si

  mov     ax, [0x0010]
  and     al, 0x30

  cmp     al, 0x30
  je      .mda

  cmp     al, 0x20
  je      .cga

  xor     ax, ax
  jmp     .prtmsg

.cga:
  mov     ax, 0x0003
  int     0x10
  mov     ax, msg_cga
  jmp     .prtmsg

.mda:
  mov     ax, 0x0007
  int     0x10
  mov     ax, msg_mda

.prtmsg:
  mov     si, msg_init
  call    prt_string

  or      ax, ax
  jz      .done

  mov     si, msg_video
  call    prt_string

  mov     si, ax
  call    prt_string

  call    prt_nl

.done:
  pop     si
  pop     ax
  ret


init_misc:
  xor     ax, ax

  mov     [0x006c], ax
  mov     [0x006e], ax
  mov     [0x0070], al

  mov     [0x0000], ax                  ; COM1
  mov     [0x0002], ax
  mov     [0x0004], ax
  mov     [0x0006], ax
  mov     [0x0008], ax                  ; LPT1
  mov     [0x000a], ax
  mov     [0x000c], ax
  mov     [0x000e], ax

  ret


; check rom checksum at ES:0000
; returns ah=00 if ok
check_rom:
  push    cx
  push    bx

  mov     ch, [es:0x0002]
  shl     ch, 1
  mov     cl, 0

  xor     bx, bx
  mov     ah, 0

.cksum:
  add     ah, [es:bx]
  inc     bx
  loop    .cksum

  pop     bx
  pop     cx
  ret


; start rom at ES:0000
start_rom:
  push    cs
  mov     ax, .romret
  push    ax
  push    es
  mov     ax, 3
  push    ax
  retf

.romret:
  ret


init_rom1:
  push    ax
  push    dx
  push    si
  push    es

  mov     dx, 0xc000

.next:
  mov     es, dx
  cmp     word [es:0x0000], 0xaa55
  jne     .norom

  mov     ah, [es:0x0002]
  mov     al, 0
  shr     ax, 1
  shr     ax, 1
  shr     ax, 1
  add     dx, ax
  jz      .norom

;  mov     si, msg_rom1
;  call    prt_string
;  mov     ax, es
;  call    prt_uint16
;  mov     si, msg_rom2
;  call    prt_string

;  call    check_rom
;  or      ah, ah
;  jz      .romok
  jmp     .romok

;  mov     si, msg_cksmbad
;  call    prt_string

;  call    prt_nl

  jmp     .skiprom

.romok:
;  mov     si, msg_cksmok
;  call    prt_string

  push    dx
  call    start_rom
  pop     dx

;  call    prt_nl

.norom:
  add     dx, 0x0080

.skiprom:
  cmp     dx, 0xc800
  jb      .next

  pop     es
  pop     si
  pop     dx
  pop     ax
  ret


init_mem:
  push    ax
  push    cx
  push    bx
  push    si
  push    es

  mov     cx, 64
  mov     bx, 0x1000

.next:
  mov     es, bx

  mov     ax, 0xaa55

  xchg    [es:0], al
  xchg    [es:0], al
  xchg    [es:0], ah
  xchg    [es:0], ah

  cmp     ax, 0xaa55
  jne     .done

  xchg    [es:0 + 1023], al
  xchg    [es:0 + 1023], al
  xchg    [es:0 + 1023], ah
  xchg    [es:0 + 1023], ah

  cmp     ax, 0xaa55
  jne     .done

  inc     cx
  add     bx, 1024 / 16

  cmp     cx, 704
  jae     .done

  jmp     .next

.done:
  mov     si, msg_memchk1
  call    prt_string

  mov     ax, cx
  call    prt_uint16

  mov     si, msg_memchk2
  call    prt_string

  call    prt_nl

  mov     [0x0013], cx

  pop     es
  pop     si
  pop     bx
  pop     cx
  pop     ax
  ret


init_keyboard:
  mov     [0x0080], word 0x001e         ; keyboard buffer start
  mov     [0x0082], word 0x003e         ; keyboard buffer end

  mov     [0x001a], word 0x001e
  mov     [0x001c], word 0x001e

  mov     [0x0017], byte 0x00           ; keyboard status 1
  mov     [0x0018], byte 0x00           ; keyboard status 2

  ret


init_serport:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si

  ; get com info
  pceh    PCEH_GET_COM

  mov     [0x0000], ax
  mov     [0x0002], bx
  mov     [0x0004], cx
  mov     [0x0006], dx

  sub     ax, 1
  cmc
  mov     ax, 0
  adc     ax, 0

  sub     bx, 1
  cmc
  adc     ax, 0

  sub     cx, 1
  cmc
  adc     ax, 0

  sub     dx, 1
  cmc
  adc     ax, 0

  mov     si, msg_serial
  call    prt_string
  call    prt_uint16

  mov     cl, 9
  shl     ax, cl

  mov     dx, [0x0010]
  and     dx, ~0x0e00
  or      dx, ax
  mov     [0x0010], dx

  call    prt_nl

  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


init_parport:
  push    ax
  push    cx
  push    dx
  push    bx
  push    si

  ; get lpt info
  pceh    PCEH_GET_LPT

  mov     [0x0008], ax
  mov     [0x000a], bx
  mov     [0x000c], cx
  mov     [0x000e], dx

  sub     ax, 1
  cmc
  mov     ax, 0
  adc     ax, 0

  sub     bx, 1
  cmc
  adc     ax, 0

  sub     cx, 1
  cmc
  adc     ax, 0

  sub     dx, 1
  cmc
  adc     ax, 0

  mov     si, msg_parallel
  call    prt_string
  call    prt_uint16

  mov     cl, 14
  shl     ax, cl

  and     byte [0x0011], 0x3f
  or      word [0x0010], ax

  call    prt_nl

  pop     si
  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


get_bcd:
  push    dx
  xor     dx, dx
  mov     ah, al
  and     ax, 0xf00f
  shr     ah, 1
  add     dl, ah
  shr     ah, 1
  shr     ah, 1
  add     dl, ah
  add     dl, al
  xchg    ax, dx
  pop     dx
  ret

init_time:
  push    ax
  push    dx
  push    si
  push    di

  mov     ah, 2
  int     0x1a

  ; 18 * seconds
  mov     al, dh
  call    get_bcd
  mov     dx, 18
  mul     dx
  mov     si, ax
  mov     di, dx

  ; 1092 * minutes
  mov     al, cl
  call    get_bcd
  mov     dx, 1092
  mul     dx
  add     si, ax
  adc     di, dx

  ; 65539 * hours
  mov     al, ch
  call    get_bcd
  add     di, ax
  mov     dx, ax
  shl     ax, 1
  add     ax, dx
  add     si, ax
  adc     di, 0

  mov     [0x006c], si
  mov     [0x006e], di
  mov     [0x0070], byte 0

  pop     di
  pop     si
  pop     dx
  pop     ax
  ret

set_bios_ds:
  mov     ds, [cs:.bios_ds]
  ret

.bios_ds:
  dw      0x0040


; print string at CS:SI
prt_string:
  push    ax
  push    bx
  push    si

  xor     bx, bx

.next:
  cs      lodsb
  or      al, al
  jz      .done

  mov     ah, 0x0e
  int     0x10

  jmp     short .next

.done:
  pop     si
  pop     bx
  pop     ax
  ret


; print newline
prt_nl:
  push    ax
  push    bx

  mov     ax, 0x0e0d
  xor     bx, bx
  int     0x10

  mov     ax, 0x0e0a
  int     0x10

  pop     bx
  pop     ax
  ret


; print a 16 bit unsigned integer in ax
prt_uint16:
  push    ax
  push    cx
  push    dx
  push    bx

  mov     bx, 10
  xor     cx, cx

.next1:
  xor     dx, dx

  div     bx
  add     dl, '0'
  push    dx
  inc     cx

  or      ax, ax
  jnz     .next1

.next2:
  pop     ax
  mov     ah, 0x0e
  xor     bx, bx
  int     0x10
  loop    .next2

  pop     bx
  pop     dx
  pop     cx
  pop     ax
  ret


;-----------------------------------------------------------------------------

inttab:
  dw      int_00, 0xf000
  dw      int_01, 0xf000
  dw      0xe2c3, 0xf000 ;int_02, 0xf000
  dw      int_03, 0xf000
  dw      int_04, 0xf000
  dw      0xff54, 0xf000 ;int_05, 0xf000
  dw      int_06, 0xf000
  dw      int_07, 0xf000
  dw      0xfea5, 0xf000 ;int_08, 0xf000
  dw      0xe987, 0xf000 ;int_09, 0xf000
  dw      int_0a, 0xf000
  dw      int_0b, 0xf000
  dw      int_0c, 0xf000
  dw      int_0d, 0xf000
  dw      int_0e, 0xf000
  dw      int_0f, 0xf000
  dw      0xf065, 0xf000 ;0xf065, 0xf000
  dw      0xf84d, 0xf000 ;int_11, 0xf000
  dw      0xf841, 0xf000 ;int_12, 0xf000
  dw      int_13, 0xf000
  dw      0xe739, 0xf000 ;int_14, 0xf000
  dw      int_15, 0xf000 ;0xf859, 0xf000
  dw      0xe82e, 0xf000 ;int_16, 0xf000
  dw      0xefd2, 0xf000 ;int_17, 0xf000
  dw      int_18, 0xf000
  dw      int_19, 0xf000 ;0xe6f2, 0xf000
  dw      int_1a, 0xf000 ;0xfe6e, 0xf000
  dw      int_1b, 0xf000
  dw      int_default, 0xf000
  dw      0xf0a4, 0xf000 ;int_1d, 0xf000
  dw      0xefc7, 0xf000 ;int_1e, 0xf000
  dw      int_1f, 0xf000


int_default:
  iret


;-----------------------------------------------------------------------------

int_00:
  pceh    PCEH_INT, 0x00
  iret

int_01:
  pceh    PCEH_INT, 0x01
  iret

int_02:
  pceh    PCEH_INT, 0x02
  iret

int_03:
  pceh    PCEH_INT, 0x03
  iret

int_04:
  pceh    PCEH_INT, 0x04
  iret

int_05:
  pceh    PCEH_INT, 0x05
  iret

int_06:
  pceh    PCEH_INT, 0x06
  iret

int_07:
  pceh    PCEH_INT, 0x07
  iret

int_09:
  pceh    PCEH_INT, 0x09
  iret

int_0a:
  pceh    PCEH_INT, 0x0a
  iret

int_0b:
  pceh    PCEH_INT, 0x0b
  iret

int_0c:
  pceh    PCEH_INT, 0x0c
  iret

int_0d:
  pceh    PCEH_INT, 0x0d
  iret

int_0e:
  pceh    PCEH_INT, 0x0e
  iret

int_0f:
  pceh    PCEH_INT, 0x0f
  iret

int_10:
  pceh    PCEH_INT, 0x10
  iret

int_11:
  pceh    PCEH_INT, 0x11
  iret

int_12:
  pceh    PCEH_INT, 0x12
  iret

int_13:
  push    bp
  mov     bp, sp
  push    word [bp + 6]                 ; flags
  popf
  pop     bp
  pceh    PCEH_INT, 0x13
  retf    2

int_14:
  pceh    PCEH_INT, 0x14
  iret

int_15:
  pceh    PCEH_INT, 0x15
  iret

int_17:
  pceh    PCEH_INT, 0x17
  iret

int_18:
  jmp     0xf600:0x0000
  iret

int_1b:
  pceh    PCEH_INT, 0x1b
  iret

int_1d:
  pceh    PCEH_INT, 0x1d
  iret

int_1e:
  pceh    PCEH_INT, 0x1e
  iret

int_1f:
  pceh    PCEH_INT, 0x1f
  iret


;-----------------------------------------------------------------------------


int_19:
  ; get boot drive in AL
  pceh    PCEH_GET_BOOT
  mov     dl, al

  xor     bx, bx
  mov     es, bx
  mov     bx, 0x7c00

  mov     ax, 0x0201
  mov     cx, 0x0001
  mov     dh, 0x00
  int     0x13
  jc      .fail

  cmp     [es:0x7dfe], word 0xaa55
;  jne     .fail

  xor     ax, ax
  mov     ds, ax
  mov     es, ax
  jmp     0x0000:0x7c00

.fail:
  iret


;-----------------------------------------------------------------------------

int_1a:
  cmp     ah, 2
  jae     .hook

  jmp     0xf000:0xfe6e

.hook:
  pceh    PCEH_INT, 0x1a

  retf    2


;-----------------------------------------------------------------------------

%ifndef NOFILL
  set_pos (0x6000)
%endif
