#include "../drivers/screen.h"
#include "./util.h"
#include "./shell.h"
#include "../interrupts/idt.h"
#include "../drivers/keyboard.h"
#include "./debug.h"

void main()
{
    clear_screen();
    println("LacOS v0.3");
    println("(c) LaCrak27 2023-2024. No rights reserved.");
    initIdt();
    initKeyboard();
    unsigned char *magicNumberPointer = (char *)0x1000;
    if (*magicNumberPointer == 0x69)
    {        
        initmm();
        for (int i = 0; i < 9999; i++)
        {
            
        }
    }
    else
    {
        println("Magic byte check failed, bootloader could not find out memory map. Halting...");
        disable_cursor();
        for (;;)
        ;
    }
    initShell();
    for (;;)
        ;
}
