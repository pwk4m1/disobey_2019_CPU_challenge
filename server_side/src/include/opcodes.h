/*
 * opcodes.h
 *
 * This file defines operation codes used by emulator.
 *
 * Changelog:
 *
 * k4m1 (11/12/18) add first opcodes.
 */

#if !defined(__opcodes_h__)
#define __opcodes_h__

/*
 * movc: move constant to reg8
 * movc		reg8, <const>
 * mov		reg8, reg8
 *
 * inc		reg8
 * dec		reg8
 *
 * add		reg8, reg8
 * sub		reg8, reg8
 *
 * jmp		addr
 *
 * cmp		<const>, reg8
 * je		addr
 * jne		addr
 *
 * push		reg8
 * pop		reg8
 *
 * int
 * hlt
 *
 */

#define op_movc	0x01
#define op_mov	0x02
#define	op_inc	0x03
#define op_dec	0x04
#define op_add	0x05
#define op_sub	0x06
#define op_jmp	0x07
#define op_cmp	0x08
#define op_je	0x09
#define op_jne	0x0a
#define	op_push	0x0b
#define op_pop	0x0c
#define op_int	0x0d
#define op_hlt	0x0e
#define op_and	0x11
#define op_or	0x12
#define op_xor	0x13
#define op_call 0x14
#define op_ret	0x15
#define op_jc   0x16
#define op_jo	0x17

/* Registers */
#define reg_al	0x20
#define reg_bl	0x21
#define reg_cl	0x22
#define reg_dl	0x23
#define	reg_si	0x24
#define reg_di	0x25
#define reg_pc	0x26
#define reg_sp	0x27

#endif

