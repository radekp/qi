.globl _start
_start:	b       start_code

start_code:	
	ldr	sp,=1024*4

	/* turn off the watchdog */
#  define pWTCON		0x53000000
	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]

.extern blink_led
	bl	blink_led
.global delay
delay:
	subs	r0,r0,#0x1
	bne	delay
	mov	pc,lr

