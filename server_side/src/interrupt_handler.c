
/*
 * This file provides functionality for interrupt handler.
 */

#define __USE_BSD

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <cpu.h>
#include <interrupts.h>
#include <int_handler.h>

#define FLAGTWO "ctf{SPI_ST0RAGE_F0R_TH3_W1N}"

void
__write_screen(struct cpu_registers *regs)
{
	uint8_t out;
	uint16_t addr;

	addr = ((regs->bl[0]) << 8);
	addr += regs->si[0];

	if ((addr >= 0x1000) && (addr < 0x00A1)) {
		regs->flags[0] |= 1UL << FLAG_CARRY;
		return;
	}

	for (; addr < 0x1000; addr++) {
		out = regs->real_sp[addr];
		printf("%c", out);
		if (out == 0x00) {
			break;
		}
	}

#if defined(DEBUG)
	printf("\tinterrupt __write_screen()\n");
	printf("\tReturn code: 0x%x\n", regs->al[0]);
	printf("\tAddr: 0x%x\n", addr);
#endif
}

void
__clear_screen()
{
	int i;
	
	for (i = 0; i < 160; i++) {
		printf("\n");
	}
}

void
__reset_cpu(struct cpu_registers *regs)
{
	regs->al[0] = 0x00;
	regs->bl[0] = 0x00;
	regs->cl[0] = 0x00;
	regs->dl[0] = 0x00;
	regs->si[0] = 0x00;
	regs->di[0] = 0x00;
	regs->sp[0] = 0x0100;
	regs->pc[0] = 0x01ff;
}

void
__get_uuid(struct cpu_registers *regs)
{
	regs->cl[0] = 0xf0;
	regs->dl[0] = 0x0f;
}

void
__get_serial(struct cpu_registers *regs)
{
	regs->al[0] = 'e';
	regs->bl[0] = 'm';
	regs->cl[0] = 'u';
	regs->dl[0] = '1';
}

void
__sleep(struct cpu_registers *regs)
{
	uint8_t t;
	t = 0;
	while (t < regs->bl[0]) {
		t++;
		sleep(1);
	}
}

void
__do_rx(struct cpu_registers *regs)
{
	regs->flags[0] |= 1UL << FLAG_CARRY;
}

void
__do_tx(struct cpu_registers *regs)
{
	regs->flags[0] |= 1UL << FLAG_CARRY;
}

void
__i2c_write(struct cpu_registers *regs, char *bootloader)
{
	uint16_t addr;

	addr = ((uint16_t)regs->bl[0]) << 8;
	addr += regs->di[0];

#if defined(DEBUG)
	printf(" ** __i2c_write\n");
#endif


	/* Deadly bug here :) */
	if (addr > 0x0200)
		bootloader[addr] = regs->si[0];
	else
		regs->flags[0] |= 1UL << FLAG_CARRY;
}

void
__i2c_read(struct cpu_registers *regs, char *bootloader)
{
	uint16_t addr;

	addr = ((uint16_t) regs->bl[0]) << 8;
	addr += regs->si[0];

	if (addr < 0x0200)
		regs->flags[0] |= 1UL << FLAG_CARRY;
	else
		regs->di[0] = bootloader[addr];
}

void
__spi_write(struct cpu_registers *regs)
{
	regs->flags[0] |= 1UL << FLAG_CARRY;
}

void
__spi_read(struct cpu_registers *regs)
{
	if (regs->pc[0] < 0x0200) {
		if (regs->bl[0] <= strlen(FLAGTWO))
			regs->di[0] = FLAGTWO[regs->bl[0]];
		else
			regs->flags[0] |= 1UL <<FLAG_CARRY;
	} else {
		regs->flags[0] |= 1UL << FLAG_CARRY;
	}
}


void
interrupt_handler(struct cpu_registers *regs, char *bootloader)
{
#if defined(DEBUG)
	printf(" ** Interrupt handler\n");
	printf("    Code: 0x%x\n", regs->al[0]);
#endif
	/* Figure out which interrupt this is */
	switch (regs->al[0]) {
		/* Documented, free-to-use interrupts */
		case (__int_screen_write):
			__write_screen(regs);
			break;
		case (__int_screen_clear):
			__clear_screen();
			break;
		case (__int_reset_cpu):
			__reset_cpu(regs);
			break;
		case (__int_poweroff):
			regs->pc[0] = 0x1001;
			break;
		case (__int_get_uuid):
			__get_uuid(regs);
			break;
		case (__int_get_serial):
			__get_serial(regs);
			break;
		case (__int_delay):
			__sleep(regs);
			break;
		case (__int_rx):
			__do_rx(regs);
			break;
		case (__int_tx):
			__do_tx(regs);
			break;
		case (__int_spi_read):
			__spi_read(regs);
			break;
		case (__int_spi_write):
			__spi_write(regs);
			break;
		case (__int_i2c_read):
			__i2c_read(regs, bootloader);
			break;
		case (__int_i2c_write):
			__i2c_write(regs, bootloader);
			break;
		default:
			regs->flags[0] |= 1UL << FLAG_CARRY;
			break;
	}
#if defined(DEBUG)
	printf(" ** Return from Interrupt handler\n");
#endif
}




