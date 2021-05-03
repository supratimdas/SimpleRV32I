CC=/opt/riscv32i/bin/riscv32-unknown-elf-gcc
CC_OPTS+=-static 
CC_OPTS+=-nostdlib 
CC_OPTS+=-nostartfiles 
CC_OPTS+=-o test

LD_SCRIPT=test.ld

OBJCOPY=/opt/riscv32i/bin/riscv32-unknown-elf-objcopy
OBJCOPY_OPTS+=-O binary
OBJCOPY_OPTS+=-S


OBJDUMP=/opt/riscv32i/bin/riscv32-unknown-elf-objdump


test: test.S
	$(CC) $(CC_OPTS) -T$(LD_SCRIPT) test.S

test.bin: test
	$(OBJCOPY) $(OBJCOPY_OPTS) test test.bin	

test.hex: test.bin
	cat test.bin | hexdump -v -e '/4 "%08X\n"' > test.hex

.PHONY: 

show_dump: test
	$(OBJDUMP) -s test	

show_bin_dump: test.bin
	cat test.bin | hexdump -v -e '/4 "%08X\n"'

clean:
	rm -Rf test test.bin test.hex
