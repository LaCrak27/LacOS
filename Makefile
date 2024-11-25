C_SOURCES = $(wildcard kernel/*.c drivers/*.c interrupts/*.c)
OBJ = ${C_SOURCES:.c=.o}
SOBJ = ${ASM_SOURCES:.asm=.o}
HEADERS = $(wildcard kernel/*.h drivers/*.h interrupts/*.h)
ASM_SOURCES = $(wildcard interrupts/*.asm)

all: clean LacOS.img LacOS.iso
bochsdbg: clean LacOS.bin
	bochsdbg.exe -f debug.bxrc -q
start: clean LacOS.img
	qemu-system-x86_64.exe -fda LacOS.img
debug: clean LacOS.bin
	qemu-system-x86_64 -s -fda LacOS.bin
LacOS.iso: LacOS.img
	mkisofs -pad -b LacOS.img -R -o LacOS.iso LacOS.img
LacOS.img: LacOS.bin
	dd if=/dev/zero of=LacOS.img bs=512 count=2880
	dd if=LacOS.bin of=LacOS.img conv=notrunc
LacOS.bin: kernel.bin boot_sect.bin
	cat boot_sect.bin kernel.bin > LacOS.bin
boot_sect.bin: boot/boot_sect.asm
	nasm boot/boot_sect.asm -f bin -o boot_sect.bin
kernel.bin: kernel/kernel_entry.o kernel/kernel.o ${OBJ} ${SOBJ}
	ld -m elf_i386 -o $@ -Ttext 0x1500 $^ --oformat binary
kernel/kernel_entry.o: kernel/kernel_entry.asm
	nasm kernel/kernel_entry.asm -f elf -o kernel/kernel_entry.o
clean:
	rm -fr *.bin *.dis *.o *.ini *.img
	rm -fr kernel/*.o boot/*.bin drivers/*.o

%.o: %.c ${HEADERS}
	gcc -fno-pie -ffreestanding -m32 -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@
