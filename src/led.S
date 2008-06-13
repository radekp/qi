.globl _start
_start:	b       start_code

start_code:	

	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0
	
	/* turn off the watchdog */
# define pWTCON		0x53000000
# define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
# define INTSUBMSK	0x4A00001C
# define INTSUBMSK_val	0xffff
# define CAMDIVN		0x4C000018

	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]
	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]

	ldr	r1, =INTSUBMSK_val
	ldr	r0, =INTSUBMSK
	str	r1, [r0]

	/* Make sure we get FCLK:HCLK:PCLK = 1:3:6 */
	ldr	r0, =CAMDIVN
	mov	r1, #0
	str	r1, [r0]

	/* Clock asynchronous mode */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0xc0000000
	mcr	p15, 0, r1, c1, c0, 0

	/* enable uart */
	ldr	r0, =0x4c00000c		/* clkcon */
	ldr	r1, =0x7fff0		/* all clocks on */
	str	r1, [r0]

	/* gpio UART0 init */
	ldr	r0, =0x56000070
	mov	r1, #0xaa
	str	r1, [r0]

	/* init uart */
	ldr	r0, =0x50000000
	mov	r1, #0x03
	str	r1, [r0]
	ldr	r1, =0x245
	str	r1, [r0, #0x04]
	mov	r1, #0x01
	str	r1, [r0, #0x08]
	mov	r1, #0x00
	str	r1, [r0, #0x0c]
	mov	r1, #0x1a
	str	r1, [r0, #0x28]

	/* blink led */
	ldr	sp,=1024*4
.extern blink_led
	bl	blink_led

.global delay
delay:
	subs	r0,r0,#0x1
	bne	delay
	mov	pc,lr

