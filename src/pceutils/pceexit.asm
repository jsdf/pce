;*****************************************************************************
;* pce                                                                       *
;*****************************************************************************

;*****************************************************************************
;* File name:     pceexit.asm                                                *
;* Created:       2003-04-25 by Hampa Hug <hampa@hampa.ch>                   *
;* Last modified: 2003-04-25 by Hampa Hug <hampa@hampa.ch>                   *
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

; $Id: pceexit.asm,v 1.1 2003/08/23 12:35:29 hampa Exp $

section text

  org     0x100

;  jmp     start

start:
  db      0x66, 0x66, 0x00, 0x01

  mov     ax, 0x4c00
  int     0x21
