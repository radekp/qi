_start: 
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	r2, .L4
	mov	r3, #5
	str	r3, [r2, #0]
	ldr	r2, .L4+4
	ldr	r3, .L4+8
	str	r3, [r2, #0]
.L2:
	ldr	r2, .L4+12
	ldr	r3, .L4+12
	ldr	r3, [r3, #0]
	bic	r3, r3, #1
	str	r3, [r2, #0]
	ldr	r0, .L4+8
	bl	delay
	ldr	r2, .L4+12
	ldr	r3, .L4+12
	ldr	r3, [r3, #0]
	orr	r3, r3, #1
	str	r3, [r2, #0]
	ldr	r0, .L4+8
	bl	delay
	b	.L2
.L5:
	.align	2
.L4:
	.word	1442840592
	.word	1442840600
	.word	65535
	.word	1442840596
 delay:
         subs    r0,r0,#0x1
         bne     delay
         mov     pc,lr
