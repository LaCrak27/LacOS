.SILENT:
.PHONY: clean, all, pre_build, LacOS.img
C_SOURCES = $(wildcard kernel/*.c drivers/*.c interrupts/*.c util/*.c)
C_FLAGS = -masm=intel -D COMP_DATE='"$(shell date)"' -g -fsanitize=null -mno-mmx -mno-sse -mno-sse2 -fno-pie -ffreestanding -m32
args = `arg="$(filter-out $@,$(MAKECMDGOALS))" && echo $${arg:-${1}}`
OBJ = ${C_SOURCES:.c=.o}
SOBJ = ${ASM_SOURCES:.asm=.o}
ASM_SOURCES = $(wildcard interrupts/*.asm)

RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

ifeq ($(findstring with-com1-shell, $(args)), with-com1-shell)
	C_FLAGS += "-D COM1_SHELL"
	COM1_SHELL_ENABLED = $(GREEN)true$(NC)
else
	COM1_SHELL_ENABLED = $(RED)false$(NC)
endif

ifneq ($(findstring Linux,  $(shell uname -s)), Linux)
$(error Building LacOS is only supported on Linux!!) 
endif


all: LacOS.img

clean:
	echo "Cleaning enviroment..."
	find . -name *.o -delete
	rm -rf *.bin *.img

pre_build: clean 
	echo "Building LacOS..."
	echo "Extras:"
	echo -e "	- COM1 Shell support: $(COM1_SHELL_ENABLED)"
	echo "Compiling C files..."

print_compiling_asm:
	echo "Compiling assembly files..."

bochsdbg: LacOS.img
	echo "Launching bochs debugger..." 
	bochs -dbg_gui -f debug.bxrc -q 

start: LacOS.img
	echo "Launching qemu..." 
	qemu-system-x86_64 -fda LacOS.img -serial stdio

debug: LacOS.img
	echo "Launching qemu with debug logging" 
	qemu-system-x86_64 -fda LacOS.img -d invalid_mem,guest_errors,cpu_reset

headless: LacOS.img
	echo "Launching headlessly" 
	qemu-system-x86_64 -fda LacOS.img -d guest_errors -nographic

LacOS.iso: LacOS.img # This is a DOS, the ISO was experimental. A lot of things won't work.
	mkisofs -pad -b LacOS.img -R -o LacOS.iso LacOS.img

LacOS.img: LacOS.bin
	echo "Creating floppy drive image..." 
	dd if=/dev/zero of=LacOS.img bs=512 count=2880 2> /dev/null
	dd if=LacOS.bin of=LacOS.img conv=notrunc 2> /dev/null
	echo -e "$(GREEN)Done!$(NC)"

LacOS.bin: kernel.bin boot_sect.bin	
	echo -e "$(GREEN)Done building!$(NC)"
	cat boot_sect.bin kernel.bin > LacOS.bin


boot_sect.bin: boot/boot_sect.asm
	nasm boot/boot_sect.asm -f bin -o boot_sect.bin

kernel.bin: pre_build ${OBJ} print_compiling_asm ${SOBJ} kernel/kernel_entry.o
	echo "Linking..."
	ld -m elf_i386 -T link.ld -o $@ kernel/kernel_entry.o $(filter %.o,$^)

%.o: %.c
	gcc $(C_FLAGS) -c $< -o $@

kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm $< -f elf -o $@

%.o: %.asm
	nasm $< -f elf -o $@