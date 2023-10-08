C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
OBJ = ${C_SOURCES:.c=.o}
HEADERS = $(wildcard kernel/*.h drivers/*.h)
all: LacOS
LacOS: kernel.bin boot_sect.bin
	cat boot_sect.bin kernel.bin > LacOS
boot_sect.bin: boot/boot_sect.asm
	nasm boot/boot_sect.asm -f bin -o boot_sect.bin
# Build the kernel binary
kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ}
	ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary
# Build the kernel object file
kernel/kernel.o: kernel/kernel.c
	gcc -fno-pie -ffreestanding -m32 -c kernel/kernel.c -o kernel/kernel.o
# Build the kernel entry object file .
kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm kernel/kernel_entry.asm -f elf -o kernel/kernel_entry.o
clean:
	rm -fr *.bin *.dis *.o *.ini
	rm -fr kernel/*.o boot/*.bin drivers/*.o

%.o: %.c ${HEADERS}
	gcc -fno-pie -ffreestanding -m32 -c $< -o $@