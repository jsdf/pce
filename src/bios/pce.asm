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

; $Id: pce.asm,v 1.2 2003/04/15 04:14:03 hampa Exp $


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

  call    init_int
  call    init_misc
  call    init_keyboard

  mov     ax, 0x0007
  int     0x10

  push    cs
  pop     ds

  mov     si, msg_init
  call    prt_string

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
  mov     [0x0010], word 0x0031         ; equipment word
  mov     [0x0013], word 512            ; ram size

  mov     [0x006c], word 0x0000
  mov     [0x006e], word 0x0000
  mov     [0x0070], byte 0x00

  ret


init_keyboard:
  mov     [0x0080], word 0x001e         ; keyboard buffer start
  mov     [0x0082], word 0x003e         ; keyboard buffer end

  mov     [0x001a], word 0x001e
  mov     [0x001c], word 0x001e

  ret


set_bios_ds:
  mov     ds, [cs:.bios_ds]
  ret

.bios_ds:
  dw      0x0040

inttab:
  dw      int_00, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      0xf065, 0xf000 ;int_10, 0xf000
  dw      0xf84d, 0xf000 ;int_11, 0xf000
  dw      0xf841, 0xf000 ;int_12, 0xf000
  dw      int_13, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      0xe82e, 0xf000 ;int_16, 0xf000
;  dw      int_16, 0xf000
  dw      int_default, 0xf000
  dw      int_18, 0xf000
  dw      0xe6f2, 0xf000 ;int_19, 0xf000
  dw      0xfe6e, 0xf000 ;int_1a, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000
  dw      0xf0a4, 0xf000 ;int_1d, 0xf000
  dw      int_default, 0xf000
  dw      int_default, 0xf000


int_default:
  iret


;-----------------------------------------------------------------------------

int_00:
  iret


;-----------------------------------------------------------------------------

int_13:
  db      0x66, 0x66, 0xcd, 0x13
  retf    2


int_16:
  mov     ax, 0x3062
  or      ax, ax
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

  set_pos (0x6000)
