.global _start
_start:	
	ldr	sp,=1024*4
	
.extern main
	bl	main
.global delay
delay:
	subs	r0,r0,#0x1
	bne	delay
	mov	pc,lr

