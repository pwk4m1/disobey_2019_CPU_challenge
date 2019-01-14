/*
 * cpu.h
 *
 * This file contains structures & constants for
 * cpu.c
 */

#if !defined(__cpu_h__)
#define __cpu_h__

#include <stdint.h>

#define FLAG_OVERFLOW	1
#define FLAG_ZERO	2
#define FLAG_EQUAL	3
#define FLAG_CARRY	4

#define STACK_SIZE	0x1000

/* Register structure */
struct
cpu_registers
{
	/* 8-bit general purpose regisers */
	uint8_t		*al;
	uint8_t		*bl;
	uint8_t		*cl;
	uint8_t		*dl;

	/* special registers */
	uint8_t		*si;
	uint8_t		*di;
	uint8_t		*flags;
	uint16_t	*sp;
	uint16_t	*rp;
	uint16_t	*pc;

	/* pointer to allocated memory acting as stack */
	uint8_t		*real_sp;
};

/* Entrypoint for cpu */
void
cpu_run(struct cpu_registers *regs, char *bootloader);

#endif

