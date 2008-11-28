/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * 
 * Configuation settings for the OPENMOKO Neo GTA02 Linux GSM phone
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

#define __ASM_MODE__
#define __ASSEMBLY__

#include <neo_gta01.h>

#define S3C2410_MISCCR_nEN_SCLK0    (1 << 17)
#define S3C2410_MISCCR_nEN_SCLK1    (1 << 18)
#define S3C2410_MISCCR_nEN_SCLKE    (1 << 19)


.globl _start, processor_id, is_jtag

_start:	b       start_code
/* if we are injected by JTAG, the script sets _istag content to nonzero */
is_jtag:
	.word	0

/* it's at a fixed address (+0x8) so we can breakpoint it in the JTAG script
 * we need to go through this hassle because before this moment, SDRAM is not
 * working so we can't prep it from JTAG
 */

_steppingstone_done:
	ldr	pc, _start_armboot

_start_armboot:
	.word 	start_qi

_TEXT_BASE:
	.word	TEXT_BASE

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end
/*
 * we have a stack in steppingstone because we can want to run full memory
 * memory tests
 */

	.fill   128
.globl _ss_stack
_ss_stack:


start_code:	
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

# define pWTCON		0x53000000

	ldr     r0, =pWTCON
	mov     r1, #0x0
	str     r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
# define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
# define INTSUBMSK	0x4A00001C
# define INTSUBMSK_val 	0x000007ff

	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]

	ldr	r1, =INTSUBMSK_val
	ldr	r0, =INTSUBMSK
	str	r1, [r0]

	/* Clock asynchronous mode */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0xc0000000
	mcr	p15, 0, r1, c1, c0, 0

#define LOCKTIME	0x4c000000

	ldr	r0, =LOCKTIME
	mov	r1, #0xffffff
	str	r1, [r0]
	
# define UPLLCON	0x4c000008	
# define MPLLCON_val    ((0x90 << 12) + (0x7 << 4) + 0x0)       /* 202 MHz */
# define UPLLCON_val    ((0x78 << 12) + (0x2 << 4) + 0x3)

	ldr	r0, =UPLLCON
	ldr	r1, =UPLLCON_val
	str	r1, [r0]

	/* Page 7-19, seven nops between UPLL and MPLL */
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	ldr	r1, =MPLLCON_val
	str	r1, [r0, #-4]		/* MPLLCON */

# define CLKDIVN	0x4C000014	/* clock divisor register */
# define CLKDIVN_val    3 /* FCLK:HCLK:PCLK = 1:2:4 */

	/* FCLK:HCLK:PCLK = 1:3:6 */
	ldr	r0, =CLKDIVN
	mov	r1, #CLKDIVN_val
	str	r1, [r0]

	/* enable only CPU peripheral block clocks we actually use */
	ldr	r0, =0x4c00000c		/* clkcon */
	ldr	r1, =0x3f10	/* uart, pwm, gpio, nand, sdi clocks on */
	str	r1, [r0]

	/* gpio UART0 init */
	ldr	r0, =0x56000070
	ldr	r1, =0x000000AA
	str	r1, [r0]

	/* init uart0 */
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

/* reset nand controller, or it is dead to us */

#define oNFCONF			0x00
#define oNFCMD			0x04
#define oNFSTAT			0x10

	mov	r1, #0x4E000000
	ldr	r2, =0xf842		@ initial value enable tacls=3,rph0=6,rph1=0
	str	r2, [r1, #oNFCONF]
	ldr	r2, [r1, #oNFCONF]
	bic	r2, r2, #0x800		@ enable chip
	str	r2, [r1, #oNFCONF]
	mov	r2, #0xff		@ RESET command
	strb	r2, [r1, #oNFCMD]
	mov	r3, #0			@ wait
1:	add	r3, r3, #0x1
	cmp	r3, #0xa
	blt	1b
2:	ldr	r2, [r1, #oNFSTAT]	@ wait ready
	tst	r2, #0x1
	beq	2b
	ldr	r2, [r1, #oNFCONF]
	orr	r2, r2, #0x800		@ disable chip
	str	r2, [r1, #oNFCONF]

	/* take sdram out of power down */
	ldr	r0, =0x56000080		/* misccr */
	ldr	r1, [ r0 ]
	bic	r1, r1, #(S3C2410_MISCCR_nEN_SCLK0 | S3C2410_MISCCR_nEN_SCLK1 | S3C2410_MISCCR_nEN_SCLKE)
	str	r1, [ r0 ]

	/* ensure signals stabalise */
	mov	r1, #128
3:	subs	r1, r1, #1
	bpl	3b

	bl	cpu_init_crit

	/* ensure some refresh has happened */
	ldr	r1, =0xfffff
4:	subs	r1, r1, #1
	bpl	4b

	/* capture full EINT situation into gstatus 4 */

	ldr	r0, =0x4A000000 /* SRCPND */
	ldr	r1, [ r0 ]
	and	r1, r1, #0xf

	ldr	r0, =0x560000BC /* gstatus4 */
	str	r1, [ r0 ]

	ldr	r0, =0x560000A8 /* EINTPEND */
	ldr	r1, [ r0 ]
	ldr	r0, =0xfff0
	and	r1, r1, r0
	ldr	r0, =0x560000BC /* gstatus4 */
	ldr     r0, [ r0 ]
	orr	r1, r1, r0
	ldr	r0, =0x560000BC /* gstatus4 */
	str	r1, [ r0 ]

	/* test for resume */

	ldr	r1, =0x560000B4		/* gstatus2 */
	ldr	r0, [ r1 ]
	tst	r0, #0x02		/* is this resume from power down */
					/* well, if it was, we are going to jump to
					 * whatever address we stashed in gstatus3,
					 * and gstatus4 will hold the wake interrupt
					 * source for the OS to look at
					 */
	ldrne	pc, [r1, #4]


								/* >> CFG_VIDEO_LOGO_MAX_SIZE */
#define CFG_GBL_DATA_SIZE		128			/* size in bytes reserved for initial data */

stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_GBL_DATA_SIZE 	/* bdinfo                        */
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

/* we are going to jump into the C part of the init now */
spin:
	b	_steppingstone_done

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

