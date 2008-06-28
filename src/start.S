/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * 
 * Configuation settings for the FIC Neo GTA02 Linux GSM phone
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <neo_gta02.h>

.globl _start
_start:	b       start_code

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

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
	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]
	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
# define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
# define INTSUBMSK	0x4A00001C
# define INTSUBMSK_val	0xffff
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]

	ldr	r1, =INTSUBMSK_val
	ldr	r0, =INTSUBMSK
	str	r1, [r0]

	/* Make sure we get FCLK:HCLK:PCLK = 1:3:6 */
# define CAMDIVN		0x4C000018
	ldr	r0, =CAMDIVN
	mov	r1, #0
	str	r1, [r0]

	/* Clock asynchronous mode */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0xc0000000
	mcr	p15, 0, r1, c1, c0, 0

# define LOCKTIME		0x4c000000
# define UPLLCON		0x4c000008
# define UPLLCON_val	(( 88 << 12) + (8 << 4) + 2)
# define MPLLCON		0x4c000004
# define MPLLCON_val	((142 << 12) + (7 << 4) + 1)

	ldr	r0, =LOCKTIME
	mov	r1, #0xffffff
	str	r1, [r0]

	ldr	r0, =UPLLCON
	ldr	r1, =UPLLCON_val
	str	r1, [r0]

	/* Page 7-23 in s3c2442B manual, seven nops between UPLL and MPLL */
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	ldr	r0, =MPLLCON
	ldr	r1, =MPLLCON_val
	str	r1, [r0]		/* MPLLCON */

# define CLKDIVN		0x4C000014	/* clock divisor register */
# define CLKDIVN_val  	7 			/* FCLK:HCLK:PCLK = 1:3:6 */
	ldr	r0, =CLKDIVN
	mov	r1, #CLKDIVN_val
	str	r1, [r0]

	/* enable uart */
	ldr	r0, =0x4c00000c	/* clkcon */
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

	bl	cpu_init_crit
										/* >> CFG_VIDEO_LOGO_MAX_SIZE */
#define CFG_GBL_DATA_SIZE		128			/* size in bytes reserved for initial data */
#define CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#define CONFIG_STACKSIZE_IRQ	(8*1024)		/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)		/* FIQ stack */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_GBL_DATA_SIZE 	/* bdinfo                        */
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000	/* clear                            */

clbss_l:
	str	r2, [r0]			/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	/* blink led */
	ldr	pc, _start_armboot

_start_armboot:	
	.word 	start_armboot

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */

cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip, lr
 
	bl	lowlevel_init

	mov	lr, ip
	mov	pc, lr

