/*
 * Main entry point for CPU code.
 *
 */

#define __USE_BSD

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <cpu.h>
#include <opcodes.h>
#include <interrupts.h>
#include <int_handler.h>

#define MSG_SIG_ILL "Illegal instruction"

int
check_flag(int flag, uint8_t *flags)
{
	int ret;
	ret = ((flags[0]) & (1 << flag));
	if (ret)
		return 1;
	return 0;
}

void
do_movc(uint8_t *r8_1, uint8_t c8)
{
	r8_1[0] = c8;
}

void
do_mov(uint8_t *r8_1, uint8_t *r8_2)
{
	r8_1[0] = r8_2[0];
}

void
do_inc(uint8_t *r8_1, uint8_t *flags)
{
	if (r8_1[0] <= 0xff) {
		r8_1[0] = r8_1[0] + 1;
	} else {
		r8_1[0] = 0x00;
		flags[0] |= 1UL << FLAG_OVERFLOW;
	}
}

void
do_dec(uint8_t *r8_1, uint8_t *flags)
{
	if (r8_1[0] > 0) {
		r8_1[0]--;
	} else {
		r8_1[0] = 0xff;
		flags[0] |= 1UL << FLAG_OVERFLOW;
	}
}

void
do_add(uint8_t *r8_1, uint8_t *r8_2, uint8_t *flags)
{
	if ((r8_1[0] + r8_2[0]) <= 0xff) {
		r8_1[0] = (r8_1[0] + r8_2[0]);
	} else {
		r8_1[0] = 0x00;
		flags[0] |= 1UL << FLAG_OVERFLOW;
	}
}

void
do_sub(uint8_t *r8_1, uint8_t *r8_2, uint8_t *flags)
{
	if ((r8_1[0] - r8_2[0]) >= 0) {
		r8_1[0] = (r8_1[0] - r8_2[0]);
	} else {
		r8_1[0] = 0xff;
		flags[0] |= 1UL << FLAG_OVERFLOW;
	}
}

void
do_jmp(uint8_t addr, uint16_t *pc, uint8_t *al)
{
	uint16_t tgt_addr;

	tgt_addr = ((uint16_t)al[0]) << 8;
	tgt_addr += addr;

#if defined(DEBUG)
	printf(" ** JMP: 0x%x\n", tgt_addr);
#endif

	if (tgt_addr > 0x1000) {
		pc[0] = 0x1001;
	} else {
		pc[0] = --tgt_addr;
	}
}

void
do_cmp(uint8_t *r8_1, uint8_t *r8_2, uint8_t *flags)
{
	if (r8_1[0] == r8_2[0]) {
		flags[0] |= 1UL << FLAG_EQUAL;
	}
}

void
do_je(uint8_t addr, uint16_t *pc, uint8_t *flags, uint8_t *al)
{
	if (check_flag(FLAG_EQUAL, flags))
		do_jmp(addr, pc, al);
}

void
do_jne(uint8_t addr, uint16_t *pc, uint8_t *flags, uint8_t *al)
{
	if (!check_flag(FLAG_EQUAL, flags))
		do_jmp(addr, pc, al);
}

void
do_push(uint8_t *r8_1, struct cpu_registers *regs)
{
	if (regs->sp[0] > 0x1000 || regs->sp[0] == 0) {
		return;
	}
	regs->real_sp[regs->sp[0]] = r8_1[0];
	regs->sp[0]++;
}

/* Vulnerability 1, ability to POP all of the memory */
void
do_pop(uint8_t *r8_1, struct cpu_registers *regs)
{
	if (regs->sp[0] > 0x1000|| regs->sp[0] == 0) {
		return;
	}
	r8_1[0] = regs->real_sp[regs->sp[0]];
	regs->sp[0]--;
}

void
do_int(struct cpu_registers *regs, char *bootloader)
{
	interrupt_handler(regs, bootloader);
}

void
do_or(uint8_t *r8_1, uint8_t *r8_2, uint8_t *flags)
{
	uint8_t r;
	r = r8_1[0] | r8_2[0];
	if (!r)
		flags[0] |= 1UL << FLAG_ZERO;
	r8_1[0] = r;
}

void
do_xor(uint8_t *r8_1, uint8_t *r8_2, uint8_t *flags)
{
	uint8_t r;
	r = r8_1[0] ^ r8_2[0];
	if (!r)
		flags[0] |= 1UL << FLAG_ZERO;
	r8_1[0] = r;
}

void
do_and(uint8_t *r8_1, uint8_t *r8_2, uint8_t *flags)
{
	uint8_t r;
	r = r8_1[0] & r8_2[0];
	if (!r)
		flags[0] |= 1UL << FLAG_ZERO;
	r8_1[0] = r;
}

void
do_call(uint8_t addr, uint16_t *pc, uint16_t *rp, uint8_t* al)
{
	rp[0] = ++pc[0];
	do_jmp(addr, pc, al);
}

void
do_ret(uint16_t *pc, uint16_t *rp)
{
	pc[0] = rp[0];
}

void
do_jc(uint8_t addr, struct cpu_registers *regs)
{
	if (check_flag(FLAG_CARRY, regs->flags))
		do_jmp(addr, regs->pc, regs->al);
}

void
do_jo(uint8_t addr, struct cpu_registers *regs)
{
	if (check_flag(FLAG_CARRY, regs->flags))
		do_jmp(addr, regs->pc, regs->al);
}


uint8_t *
get_reg(struct cpu_registers *regs, char byte)
{
	uint8_t *ret;

	switch (byte) {
		case (reg_al):
			ret = regs->al;
			break;
		case (reg_bl):
			ret = regs->bl;
			break;
		case (reg_cl):
			ret = regs->cl;
			break;
		case (reg_dl):
			ret = regs->dl;
			break;
		case (reg_si):
			ret = regs->si;
			break;
		case (reg_di):
			ret = regs->di;
			break;
		default:
			ret = 0x00;
	}
	return ret;
}

void
fault(struct cpu_registers *regs, char *reason)
{
	printf(" ** CPU Fault: %s\n", reason);
	printf("===========================\n");
	printf("| AL: 0x%x | BL: 0x%x | CL: 0x%x\t\n", regs->al[0],
			regs->bl[0], regs->cl[0]);
	printf("| DL: 0x%x | SI: 0x%x | DI: 0x%x\t\n", regs->dl[0],
			regs->si[0], regs->di[0]);
	printf("| PC: 0x%x | SP: 0x%x | RP: 0x%x\n", regs->pc[0],
			regs->sp[0],
			regs->rp[0]);

	printf("| Flags (OZEC) : %d%d%d%d\n",
			check_flag(FLAG_OVERFLOW, regs->flags),
			check_flag(FLAG_ZERO, regs->flags),
			check_flag(FLAG_EQUAL, regs->flags),
			check_flag(FLAG_CARRY, regs->flags));
}
			
/* Function to handle cpu main loop */
void
cpu_run(struct cpu_registers *regs, char *bootloader_code)
{
	/* Memory address space */
	char *bootloader;

	/* Constants */
	uint8_t		c8;

	/* registers */
	uint8_t		*r8_1;
	uint8_t		*r8_2;

	/* Addresses */
	uint8_t		addr;

	/* opcode counter */
	int		opc_cnt;

	bootloader = (char *)malloc(0x1000);
	if (!bootloader) {
		printf("Malloc() failed\n");
	}
	memset(bootloader, 0, 0x1000);
	memset(bootloader, 0x30, 0x01ff);
	memcpy((void*)bootloader+0x0200, bootloader_code, 0xdff);

	regs->sp[0] = 0x0100;
	regs->pc[0] = 0x0200;
	regs->rp[0] = 0x0200;
	opc_cnt     = 0;
	
	/* Main cpu loop */
	while (regs->pc[0] < 0x1000) {
		/* We only allow this many instructions to be run */
		if (opc_cnt++ == 0x4000) {
			fault(regs, "Infinite loop is infinite :(\n");
			break;
		}
		switch (bootloader[regs->pc[0]]) {
		case (op_movc):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			c8 = bootloader[++regs->pc[0]];
			if (!r8_1)
				goto sig_ill;
			do_movc(r8_1, c8);
			break;
		case (op_mov):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_mov(r8_1, r8_2);
			break;
		case (op_inc):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1)
				goto sig_ill;
			do_inc(r8_1, regs->flags);
			break;
		case (op_dec):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1)
				goto sig_ill;
			do_dec(r8_1, regs->flags);
			break;
		case (op_add):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_add(r8_1, r8_2, regs->flags);
			break;
		case (op_sub):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_sub(r8_1, r8_2, regs->flags);
			break;
		case (op_jmp):
			addr = bootloader[++regs->pc[0]];
			do_jmp(addr, regs->pc, regs->al);
			break;
		case (op_cmp):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_cmp(r8_1, r8_2, regs->flags);
			break;
		case (op_je):
			addr = bootloader[++regs->pc[0]];
			do_je(addr, regs->pc, regs->flags, regs->al);
			break;
		case (op_jne):
			addr = bootloader[++regs->pc[0]];
			do_jne(addr, regs->pc, regs->flags, regs->al);
			break;
		case (op_push):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1)
				goto sig_ill;
			do_push(r8_1, regs);
			break;
		case (op_pop):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1)
				goto sig_ill;
			do_pop(r8_1, regs);
			break;
		case (op_int):
			do_int(regs, bootloader);
			break;
		case (op_hlt):
			regs->pc[0] = 0x1001;
			break;
		case (op_and):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_and(r8_1, r8_2, regs->flags);
			break;
		case (op_or):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_or(r8_1, r8_2, regs->flags);
			break;
		case (op_xor):
			r8_1 = get_reg(regs, bootloader[++regs->pc[0]]);
			r8_2 = get_reg(regs, bootloader[++regs->pc[0]]);
			if (!r8_1 || !r8_2)
				goto sig_ill;
			do_xor(r8_1, r8_2, regs->flags);
			break;
		case (op_call):
			addr = bootloader[++regs->pc[0]];
			do_call(addr, regs->pc, regs->rp, regs->al);
			break;
		case (op_ret):
			do_ret(regs->pc, regs->rp);
			break;
		case (op_jc):
			addr = bootloader[++regs->pc[0]];
			do_jc(addr, regs);
			break;
		case (op_jo):
			addr = bootloader[++regs->pc[0]];
			do_jo(addr, regs);
			break;
		case (0x30):
#if defined(DEBUG)
			printf(" ** Opcode 0x30, reset CPU\n");
#endif
			regs->pc[0] = 0x01ff;
			break;
		default:
			printf("Pc[0] = %x\n", bootloader[regs->pc[0]]);
			goto sig_ill;
			break;
		}
		++regs->pc[0];
#if defined(DEBUG)
		printf("--------------------------\n");
		printf("PC: 0x%x | SP: 0x%x\n", regs->pc[0], regs->sp[0]);
		printf("BP: 0x%x\n", regs->rp[0]);
		printf("AL: 0x%x | BL: 0x%x | CL: 0x%x\n", regs->al[0],
				regs->bl[0],
				regs->cl[0]);
		printf("DL: 0x%x | SI: 0x%x | DI: 0x%x\n", regs->dl[0],
				regs->si[0],
				regs->di[0]);
		printf("FLAGS (OZCE): %d%d%d%d\n",
				check_flag(FLAG_OVERFLOW, regs->flags),
				check_flag(FLAG_ZERO, regs->flags),
				check_flag(FLAG_CARRY, regs->flags),
				check_flag(FLAG_EQUAL, regs->flags)
		      );
#endif
	}
	return;
sig_ill:
	fault(regs, MSG_SIG_ILL);

}


