

#define __USE_BSD

#include <sys/types.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cpu.h>
#include <opcodes.h>

#define STACK_SIZE 0x1000

#define FLAG_ONE "ctf{COOL_PROCESSORS_ARE_COOL}"

void
free_cpu_registers(struct cpu_registers *regs)
{
	if (regs) {
		if (regs->al) free(regs->al);
		if (regs->bl) free(regs->bl);
		if (regs->cl) free(regs->cl);
		if (regs->dl) free(regs->dl);
		if (regs->si) free(regs->si);
		if (regs->di) free(regs->di);
		if (regs->flags) free(regs->flags);
		if (regs->sp) free(regs->sp);
		if (regs->pc) free(regs->pc);
		if (regs->rp) free(regs->rp);
		if (regs->real_sp) free(regs->real_sp);
		free(regs);
	}
}

/*
 * This function initializes all variables needed to run the
 * emulator code. (aka cpu registers)
 */ 
int
init_cpu_registers(struct cpu_registers *regs)
{
	int ret;

	ret = 0;
	
	regs->al = (uint8_t*)malloc(1);
	regs->bl = (uint8_t*)malloc(1);
	regs->cl = (uint8_t*)malloc(1);
	regs->dl = (uint8_t*)malloc(1);
	regs->si = (uint8_t*)malloc(1);
	regs->di = (uint8_t*)malloc(1);
	regs->flags = (uint8_t*)malloc(1);

	regs->sp = (uint16_t*)malloc(2);
	regs->pc = (uint16_t*)malloc(2);
	regs->rp = (uint16_t*)malloc(2);

	regs->real_sp = (uint8_t*)malloc(STACK_SIZE);

	if (!regs->al || !regs->bl || !regs->cl || !regs->dl ||
			!regs->si || !regs->di || !regs->sp ||
			!regs->flags || !regs->pc || !regs->real_sp ||
			!regs->rp) {
		ret = -1;
		goto fail;
	}
	regs->al[0] = 0;
	regs->bl[0] = 0;
	regs->cl[0] = 0;
	regs->dl[0] = 0;
	regs->flags[0] = 0x00;

	regs->si[0] = 0x00;
	regs->di[0] = 0x00;
	regs->sp[0] = 0x0000;
	regs->pc[0] = 0x0000;

	memset(regs->real_sp, 0, STACK_SIZE);
	memset(regs->real_sp, 0x30, 0x00A0);
	memset(regs->real_sp+0x00A0, 0x41, 0x00ff);
	strcpy((char*)regs->real_sp, FLAG_ONE);
	goto end;
fail:
	free_cpu_registers(regs);
end:
	return ret;
}

int
usage(char *name)
{
	printf("Syntax: %s <path/to/bootloader/binary>\n", name);
	return 0;
}

int
read_bootloader(char *out, char *path)
{
	FILE *fp;

	fp = fopen(path, "rb");
	if (!fp)
		return -1;

	fread(out, 0xdff, 1, fp);
	fclose(fp);
	return 0;
}


/* Code entrypoint */
int
main(int argc, char *argv[])
{
	struct cpu_registers *regs;
	char *bootloader;

	if (argc != 2)
		return usage(argv[0]);

	regs = (struct cpu_registers *)malloc(sizeof(struct cpu_registers));
	if (!regs) {
		printf("Malloc() failed\n");
		return -1;
	}
	bootloader = (char *)malloc(0xdff);
	if (!bootloader) {
		printf("Malloc() failed\n");
		return -1;
	}
	memset(bootloader, 0x00, 0xdff);

	if (!regs || !bootloader) {
		printf("Failed to allocate memory\n");
		return -1;
	}

	if (read_bootloader(bootloader, argv[1])) {
		free(regs);
		free(bootloader);
		printf("Failed to read binary from %s\n", argv[1]);
		return -1;
	}

	if (!init_cpu_registers(regs))
		cpu_run(regs, bootloader);
	else 
		printf("Malloc failed! Abort.\n");
	return 0;
}


