OUTPUT_FORMAT(binary)

SECTIONS {
    .text 0x101F0: {
        *(.text) *(.rodata)
    }
    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }

    kernel_end = .;
}