;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pce.asm                                                    *
;* Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-04-15 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id: pce.asm,v 1.5 2003/04/17 14:15:56 hampa Exp $


%macro set_pos 1
  times %1 - ($ - $$) db 0
%endmacro


section .text


;-----------------------------------------------------------------------------

start:
  mov     ax, 0x0050
  mov     ss, ax
  mov     sp, 1024

  call    set_bios_ds

;  call    pce_test

  call    init_ppi
  call    init_int
  call    init_misc
  call    init_keyboard

  mov     ax, 0x0007
  int     0x10

  push    cs
  pop     ds

  mov     si, msg_init
  call    prt_string

;  int     0x18
  int     0x19

done:
  jmp     done


testdiv   dw 13

msg_init:
  db      "PC BIOS", 13, 10
  db      "(hacked up version for PCE)", 13, 10, 0


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


init_misc:
  xor     ax, ax

  mov     [0x0010], word 0x017d         ; equipment word
  mov     [0x0013], word 512            ; ram size

  mov     [0x006c], ax
  mov     [0x006e], ax
  mov     [0x0070], al

  mov     [0x0000], ax                  ; COM1
  mov     [0x0000], ax
  mov     [0x0000], ax
  mov     [0x0000], ax
  mov     [0x0000], ax                  ; LPT1
  mov     [0x0000], ax
  mov     [0x0000], ax
  mov     [0x0000], ax

  ret


init_keyboard:
  mov     [0x0080], word 0x001e         ; keyboard buffer start
  mov     [0x0082], word 0x003e         ; keyboard buffer end

  mov     [0x001a], word 0x001e
  mov     [0x001c], word 0x001e

  ret

init_ppi:
  mov     al, 0x99
  out     0x63, al                      ; set up ppi ports

  mov     al, 0xfc
  out     0x61, al

  in      al, 0x60                      ; get config word

  ret

set_bios_ds:
  mov     ds, [cs:.bios_ds]
  ret

.bios_ds:
  dw      0x0040

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
  dw      int_09, 0xf000
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
  dw      0xe6f2, 0xf000 ;int_19, 0xf000
  dw      0xfe6e, 0xf000 ;int_1a, 0xf000
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

int_19:
  db      0x66, 0x66, 0xcd, 0x19
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


prt_string:
  push    ax
  push    si

.next
  lodsb
  or      al, al
  jz      .done

  mov     ah, 0x0e
  int     0x10

  jmp     short .next

.done
  pop     si
  pop     ax
  ret

;-----------------------------------------------------------------------------

test_imul:
  mov     ax, 1
  mov     cx, 0
  xor     si, si
  mov     di, si

.next1:
  imul    cx
  adc     si, ax
  adc     di, dx
  inc     ax
  loop    .next1

  mov     al, 1
  mov     cx, 0
  xor     si, si
  mov     di, si

.next2:
  imul    cl
  adc     si, ax
  adc     di, 0
  inc     al
  loop    .next2

  ret


pce_test:
  call    test_imul
  ret

;-----------------------------------------------------------------------------

  set_pos (0x6000)
