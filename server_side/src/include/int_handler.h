
/*
 * Interrupt handler.
 */

#if !defined (__int_handler_h__)
#define __int_handler_h__

#include <cpu.h>

void
interrupt_handler(struct cpu_registers *regs, char *bootloader);

#endif

