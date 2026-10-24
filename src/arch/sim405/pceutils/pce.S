#-----------------------------------------------------------------------------
# pce
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# File name:    pce.S
# Created:      2018-12-22 by Hampa Hug <hampa@hampa.ch>
# Copyright:    (C) 2018 Hampa Hug <hampa@hampa.ch>
#-----------------------------------------------------------------------------

#define PCE_HOOK_CHECK           0x0000
#define PCE_HOOK_GET_VERSION     0x0001
#define PCE_HOOK_GET_VERSION_STR 0x0002
#define PCE_HOOK_INIT            0x0100
#define PCE_HOOK_PUTC            0x0101
#define PCE_HOOK_GETC            0x0102
#define PCE_HOOK_STOP            0x0200
#define PCE_HOOK_ABORT           0x0201
#define PCE_HOOK_MSG             0x0300
#define PCE_HOOK_GET_TIME_UNIX   0x0400
#define PCE_HOOK_READ_OPEN       0x0500
#define PCE_HOOK_READ_CLOSE      0x0501
#define PCE_HOOK_READ            0x0502
#define PCE_HOOK_WRITE_OPEN      0x0600
#define PCE_HOOK_WRITE_CLOSE     0x0601
#define PCE_HOOK_WRITE           0x0602

#define hook .long 0x44ce0002

	.global	pce_check
	.global	pce_get_version
	.global	pce_get_version_str
	.global	pce_init
	.global	pce_putc
	.global	pce_putc0
	.global	pce_puts
	.global	pce_puts0
	.global	pce_getc
	.global	pce_gets
	.global	pce_stop
	.global	pce_abort
	.global	pce_set_msg
	.global	pce_get_time_unix
	.global	pce_read_open
	.global	pce_read_close
	.global	pce_read16
	.global	pce_write_open
	.global	pce_write_close
	.global	pce_write16

	.text

#-----------------------------------------------------------------------------
# int pce_check (void)
#-----------------------------------------------------------------------------
pce_check:
	li	%r3, PCE_HOOK_CHECK
	hook

	cmpwi	%r3, 0xffffffce
	beq	1f

	li	%r3, -1
	blr
1:
	li	%r3, 0
	blr


#-----------------------------------------------------------------------------
# int pce_get_version (unsigned p[3])
#-----------------------------------------------------------------------------
pce_get_version:
	mr	%r7, %r3

	li	%r3, PCE_HOOK_GET_VERSION
	hook
	cmpwi	%r3, 0
	bnelr

	stw	%r4, 0(%r7)
	stw	%r5, 4(%r7)
	stw	%r6, 8(%r7)

	blr


#-----------------------------------------------------------------------------
# int pce_get_str (char *str, unsigned max)
#-----------------------------------------------------------------------------
pce_get_version_str:
	mflr	%r0
	stwu	%r1, -8(%r1)
	stw	%r0, 12(%r1)

	mr	%r5, %r3

	li	%r3, PCE_HOOK_GET_VERSION_STR
	hook
	cmpwi	%r3, 0
	bnelr

	mr	%r3, %r5
	bl	pce_gets

	lwz	%r0, 12(%r1)
	mtlr	%r0
	addi	%r1, %r1, 8
	blr


#-----------------------------------------------------------------------------
# int pce_init (void)
#-----------------------------------------------------------------------------
pce_init:
	li	%r3, PCE_HOOK_INIT
	hook
	blr


#-----------------------------------------------------------------------------
# int pce_putc0 (int c)
#-----------------------------------------------------------------------------
pce_putc0:
	mr	%r5, %r3
	li	%r3, PCE_HOOK_INIT
	hook
	cmpwi	%r3, 0
	bnelr
	mr	%r3, %r5
	# fall through


#-----------------------------------------------------------------------------
# int pce_putc (int c)
#-----------------------------------------------------------------------------
pce_putc:
	li	%r3, PCE_HOOK_PUTC
	hook
	blr


#-----------------------------------------------------------------------------
# int pce_puts0 (const char *s)
#-----------------------------------------------------------------------------
pce_puts0:
	mr	%r5, %r3
	li	%r3, PCE_HOOK_INIT
	hook
	cmpwi	%r3, 0
	bnelr
	mr	%r3, %r5
	# fall through


#-----------------------------------------------------------------------------
# int pce_puts (const char *s)
#-----------------------------------------------------------------------------
pce_puts:
	subi	%r5, %r3, 1
0:
	lbzu	%r4, 1(%r5)
	cmpwi	%r4, 0
	beq	1f

	li	%r3, PCE_HOOK_PUTC
	hook
	cmpwi	%r3, 0
	beq	0b
1:
	blr


#-----------------------------------------------------------------------------
# int pce_getc (void)
#-----------------------------------------------------------------------------
pce_getc:
	li	%r3, PCE_HOOK_GETC
	hook
	blr


#-----------------------------------------------------------------------------
# int pce_gets (const char *s, unsigned max)
#-----------------------------------------------------------------------------
pce_gets:
	subi	%r5, %r3, 1

	mtctr	%r4
0:
	li	%r3, PCE_HOOK_GETC
	hook
	cmpwi	%r3, 0
	blt	1f

	stbu	%r3, 1(%r5)
	bdnz	0b

	li	%r3, -1
	blr
1:
	li	%r3, 0
	stbu	%r3, 1(%r5)
	blr


#-----------------------------------------------------------------------------
# void pce_stop (void)
#-----------------------------------------------------------------------------
pce_stop:
	li	%r3, PCE_HOOK_STOP
	hook
	blr


#-----------------------------------------------------------------------------
# void pce_abort (void)
#-----------------------------------------------------------------------------
pce_abort:
	li	%r3, PCE_HOOK_ABORT
	hook
	blr


#-----------------------------------------------------------------------------
# int pce_set_msg (const char *msg, const char *val)
#-----------------------------------------------------------------------------
pce_set_msg:
	mflr	%r0
	stwu	%r1, -16(%r1)
	stw	%r0, 20(%r1)
	stw	%r31, 12(%r1)

	mr	%r31, %r4

	bl	pce_puts0
	cmpwi	%r3, 0
	bne	1f

	li	%r3, 0
	bl	pce_putc
	cmpwi	%r3, 0
	bne	1f

	mr	%r3, %r31
	bl	pce_puts
	cmpwi	%r3, 0
	bne	1f

	li	%r3, PCE_HOOK_MSG
	hook
1:
	lwz	%r31, 12(%r1)
	lwz	%r0, 20(%r1)
	mtlr	%r0
	addi	%r1, %r1, 16
	blr


#-----------------------------------------------------------------------------
# long long pce_get_time_unix (void)
#-----------------------------------------------------------------------------
pce_get_time_unix:
	li	%r3, PCE_HOOK_GET_TIME_UNIX
	hook

	cmpwi	%r3, 0
	bne	1f

	mr	%r3, %r4
	mr	%r4, %r5
	blr
1:
	li	%r3, 0
	li	%r4, 0
	blr


#-----------------------------------------------------------------------------
# int pce_read_open (const char *str)
#-----------------------------------------------------------------------------
pce_read_open:
	mflr	%r0
	stwu	%r1, -8(%r1)
	stw	%r0, 12(%r1)

	bl	pce_puts0
	cmpwi	%r3, 0
	bne	1f

	li	%r3, PCE_HOOK_READ_OPEN
	hook
1:
	lwz	%r0, 12(%r1)
	mtlr	%r0
	addi	%r1, %r1, 8
	blr


#-----------------------------------------------------------------------------
# int pce_read_close (void)
#-----------------------------------------------------------------------------
pce_read_close:
	li	%r3, PCE_HOOK_READ_CLOSE
	hook
	blr


#-----------------------------------------------------------------------------
# int pce_read16 (void *buf)
#-----------------------------------------------------------------------------
pce_read16:
	mr	%r8, %r3		# buf

	li	%r4, 0
	li	%r5, 0
	li	%r6, 0
	li	%r7, 0

	li	%r3, PCE_HOOK_READ
	hook

	stw	%r4, 0(%r8)
	stw	%r5, 4(%r8)
	stw	%r6, 8(%r8)
	stw	%r7, 12(%r8)

	blr


#-----------------------------------------------------------------------------
# int pce_write_open (const char *str)
#-----------------------------------------------------------------------------
pce_write_open:
	mflr	%r0
	stwu	%r1, -8(%r1)
	stw	%r0, 12(%r1)

	bl	pce_puts0
	cmpwi	%r3, 0
	bne	1f

	li	%r3, PCE_HOOK_WRITE_OPEN
	hook
1:
	lwz	%r0, 12(%r1)
	mtlr	%r0
	addi	%r1, %r1, 8
	blr


#-----------------------------------------------------------------------------
# int pce_write_close (void)
#-----------------------------------------------------------------------------
pce_write_close:
	li	%r3, PCE_HOOK_WRITE_CLOSE
	hook
	blr


#-----------------------------------------------------------------------------
# int pce_write16 (void *buf, unsigned cnt)
#-----------------------------------------------------------------------------
pce_write16:
	mr	%r8, %r4		# cnt

	lwz	%r4, 0(%r3)
	lwz	%r5, 4(%r3)
	lwz	%r6, 8(%r3)
	lwz	%r7, 12(%r3)

	li	%r3, PCE_HOOK_WRITE
	hook

	blr
