;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pce.asm                                                    *
;* Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-04-19 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id: pce.asm,v 1.3 2003/04/25 02:29:56 hampa Exp $


%macro set_pos 1
  times %1 - ($ - $$) db 0
%endmacro


section .text


;-----------------------------------------------------------------------------

start:
  cli
  mov     ax, 0x0050
  mov     ss, ax
  mov     sp, 1024

  call    set_bios_ds

  call    init_int
  call    init_ppi
  call    init_pic

  sti

  call    init_video
  call    init_mem
  call    init_misc
  call    init_keyboard
  call    init_time

  call    prt_nl

  push    cs
  pop     ds

  int     0x19
  int     0x18

done:
  jmp     done


msg_init:
  db      "PC BIOS (hacked up version for PCE)", 13, 10, 13, 10, 0

msg_memchk1:
  db      "Memory check... ", 0

msg_memchk2:
  db      "KB", 13, 0

msg_mda:
  db      "MDA", 0

msg_cga:
  db      "CGA", 0

msg_video:
  db      " video adapter initialized", 13, 10, 0


;-----------------------------------------------------------------------------

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

  pop     ds
  pop     es
  pop     di
  pop     si
  pop     cx
  pop     ax
  ret


init_ppi:
  push    ax
  mov     al, 0x99
  out     0x63, al                      ; set up ppi ports

  mov     al, 0xfc
  out     0x61, al

  in      al, 0x60                      ; get config word
  mov     ah, 0

  mov     [0x0010], ax                  ; equipment word

  mov     al, 0x7c
  out     0x61, al

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


init_video:
  push    ax
  push    si

  mov     ax, [0x0010]
  and     al, 0x30

  cmp     al, 0x30
  je      .mda

  mov     ax, 0x0003
  int     0x10
  mov     ax, msg_cga
  jmp     .done

.mda:
  mov     ax, 0x0007
  int     0x10
  mov     ax, msg_mda

.done:
  mov     si, msg_init
  call    prt_string

  mov     si, ax
  call    prt_string

  mov     si, msg_video
  call    prt_string

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

  xchg    [es:0x7fff], al
  xchg    [es:0x7fff], al
  xchg    [es:0x7fff], ah
  xchg    [es:0x7fff], ah

  cmp     ax, 0xaa55
  jne     .done

  add     cx, 32
  add     bx, 0x800

  mov     si, msg_memchk1
  call    prt_string

  mov     ax, cx
  call    prt_uint16

  mov     si, msg_memchk2
  call    prt_string

  cmp     cx, 704
  jae     .done

  jmp     .next

.done:
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


inttab:
  dw      int_00, 0xf000
  dw      int_01, 0xf000
  dw      int_02, 0xf000
  dw      int_03, 0xf000
  dw      int_04, 0xf000
  dw      int_05, 0xf000
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
  dw      0xf065, 0xf000 ;int_10, 0xf000
  dw      0xf84d, 0xf000 ;int_11, 0xf000
  dw      0xf841, 0xf000 ;int_12, 0xf000
  dw      int_13, 0xf000
  dw      0xe739, 0xf000 ;int_14, 0xf000
  dw      int_default, 0xf000
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
  db      0x66, 0x66, 0xcd, 0x00
  iret

int_01:
  db      0x66, 0x66, 0xcd, 0x01
  iret

int_02:
  db      0x66, 0x66, 0xcd, 0x02
  iret

int_03:
  db      0x66, 0x66, 0xcd, 0x03
  iret

int_04:
  db      0x66, 0x66, 0xcd, 0x04
  iret

int_05:
  db      0x66, 0x66, 0xcd, 0x05
  iret

int_06:
  db      0x66, 0x66, 0xcd, 0x06
  iret

int_07:
  db      0x66, 0x66, 0xcd, 0x07
  iret

int_09:
  db      0x66, 0x66, 0xcd, 0x09
  iret

int_0a:
  db      0x66, 0x66, 0xcd, 0x0a
  iret

int_0b:
  db      0x66, 0x66, 0xcd, 0x0b
  iret

int_0c:
  db      0x66, 0x66, 0xcd, 0x0c
  iret

int_0d:
  db      0x66, 0x66, 0xcd, 0x0d
  iret

int_0e:
  db      0x66, 0x66, 0xcd, 0x0e
  iret

int_0f:
  db      0x66, 0x66, 0xcd, 0x0f
  iret

int_10:
  db      0x66, 0x66, 0xcd, 0x10
  iret

int_11:
  db      0x66, 0x66, 0xcd, 0x11
  iret

int_12:
  db      0x66, 0x66, 0xcd, 0x12
  iret

int_14:
  db      0x66, 0x66, 0xcd, 0x14
  iret

int_15:
  db      0x66, 0x66, 0xcd, 0x15
  iret

int_17:
  db      0x66, 0x66, 0xcd, 0x17
  iret

int_1b:
  db      0x66, 0x66, 0xcd, 0x1b
  iret

int_1d:
  db      0x66, 0x66, 0xcd, 0x1d
  iret

int_1e:
  db      0x66, 0x66, 0xcd, 0x1e
  iret

int_1f:
  db      0x66, 0x66, 0xcd, 0x1f
  iret


;-----------------------------------------------------------------------------

int_13:
  db      0x66, 0x66, 0xcd, 0x13
  retf    2


int_18:
  jmp     0xf600:0x0000
  iret


int_19:
  xor     bx, bx
  mov     es, bx
  mov     bx, 0x7c00

;  jmp     .try_fd

  mov     ax, 0x0201
  mov     cx, 0x0001
  mov     dx, 0x0080
  int     0x13
  jc      .try_fd

  cmp     [es:0x7dfe], word 0xaa55
  je      .boot

.try_fd:
  mov     ax, 0x0201
  mov     cx, 0x0001
  mov     dx, 0x0000
  int     0x13
  jc      .fail

  cmp     [es:0x7dfe], word 0xaa55
  je      .boot

.fail:
  iret

.boot:
  jmp     0x0000:0x7c00


int_1a:
  cmp     ah, 2
  jae     .hook

  jmp     0xf000:0xfe6e

.hook:
  db      0x66, 0x66, 0xcd, 0x1a

  retf    2


;-----------------------------------------------------------------------------


pce_test:
  ret

;-----------------------------------------------------------------------------

  set_pos (0x6000)
