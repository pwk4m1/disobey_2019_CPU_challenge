
cc=gcc
cflags=-Wall -Wextra -I./src/include

emu=src/emulator_main.c src/cpu.c src/interrupt_handler.c
emu_target=bin/emulator

all: check clean install-emulator 

emulator-debug:
	$(cc) $(cflags) -o $(emu_target) -D DEBUG $(emu)

check:
	@command -v $(cc)>/dev/null 2>&1 || { \
		echo "missing $(cc)"; \
		exit 1; \
	}

clean:
	rm -rf bin/*

install-emulator:
	$(cc) $(cflags) -o $(emu_target) $(emu)




