;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pcever.asm                                                 *
;* Created:       2003-10-13 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-10-13 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id$


%include "pce.inc"


section .text

  org     0x100

  jmp     start


msg_vers  db "PCE version ", 0
msg_rel   db " (release)", 0
msg_dev   db " (dev)", 0


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

prt_char:
  push    ax
  push    bx

  mov     ah, 0x0e
  xor     bx, bx
  int     0x10

  pop     bx
  pop     ax
  ret

prt_string:
  push    ax
  push    si

.next:
  mov     ah, 0x0e
  lodsb
  or      al, al
  jz      .done
  int     0x10
  jmp     .next

.done:
  pop     si
  pop     ax
  ret


start:
  mov     ax, cs
  mov     ds, ax

  mov     si, msg_vers
  call    prt_string

  pceh    PCEH_GET_VERS
  mov     cx, ax

  mov     ah, 0
  mov     al, ch
  call    prt_uint16

  mov     al, '.'
  call    prt_char

  mov     al, cl
  call    prt_uint16

  mov     al, '.'
  call    prt_char

  mov     al, dh
  call    prt_uint16

  mov     si, msg_rel
  or      dl, dl
  jnz     .release
  mov     si, msg_dev

.release
  call    prt_string

  mov     al, 0x0d
  call    prt_char
  mov     al, 0x0a
  call    prt_char

  mov     ax, 0x4c00
  int     0x21
