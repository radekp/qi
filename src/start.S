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
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:		.word software_interrupt
_prefetch_abort:		.word prefetch_abort
_data_abort:			.word data_abort
_not_used:			.word not_used
_irq:					.word irq
_fiq:					.word fiq

	.balignl 16,0xdeadbeef
/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

.globl	preboot_override
preboot_override:
	.word	0

/* Must follow preboot_override , so we get a well-known address ! */
.globl	env_override
env_override:
	.word	0

/* we want to be able to start kboot directly from within NAND flash */
.globl	booted_from_nand
booted_from_nand:
	.word	0
_booted_from_nand:
	.word	booted_from_nand


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

/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 	0x0badc0de


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
/*
 * Size of malloc() pool
 */
#define CFG_ENV_SIZE			0x40000		/* 128k Total Size of Environment Sector */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 400*1024)
										/* >> CFG_VIDEO_LOGO_MAX_SIZE */
#define CFG_GBL_DATA_SIZE		128			/* size in bytes reserved for initial data */
#define CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#define CONFIG_STACKSIZE_IRQ	(8*1024)		/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)		/* FIQ stack */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
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
	.word blink_led

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

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)  @ set base 2 words into abort stack
	ldmia	r2, {r2 - r3}			@ get pc, cpsr
	add	r0, sp, #S_FRAME_SIZE		@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add     r7, sp, #S_PC
	stmdb   r7, {sp, lr}^                   @ Calling SP, LR
	str     lr, [r7, #0]                    @ Save calling PC
	mrs     r6, spsr
	str     r6, [r7, #4]                    @ Save CPSR
	str     r0, [r7, #8]                    @ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r13, r13, #(CFG_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str     lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align  5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	//bl 	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
	//bl 	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	//bl 	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	//bl 	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	//bl 	do_not_used

/* CONFIG_USE_IRQ*/
	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	//bl 	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	//bl 	do_fiq
	irq_restore_user_regs
