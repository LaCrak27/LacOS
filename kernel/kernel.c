#include "../drivers/screen.h"
#include "../drivers/floppy.h"
#include "../drivers/timer.h"
#include "../drivers/sound.h"
#include "./util.h"
#include "./memory.h"
#include "./shell.h"
#include "../interrupts/idt.h"
#include "../drivers/keyboard.h"
#include "./debug.h"

void main()
{
    clear_screen();
    println("LacOS v0.3");
    println("(c) LaCrak27 2023-2024. No rights reserved.");
    print("Initializing IDT...          ");
    initIdt();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    print("Initializing timer...        ");
    initTimer();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    println("Initializing floppy...       ");
    //initFloppy();
    if (1)
    {
        set_fg(GREEN);
        println("Floppy initialization done!");
        set_fg(GRAY);
    }
    else
    {
        set_fg(RED);
        println("Floppy initialization failed!");
        set_fg(GRAY);
    }
    print("Initializing keyboard...     ");
    initKeyboard();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    unsigned char *magicNumberPointer = (char *)0x1000;
    if (*magicNumberPointer == 0x69)
    {
        initmm();
    }
    else
    {
        except("Magic byte check failed, bootloader could not find out memory map. Halting...");
    }
    while (1) // Loop in case we exit shell somehow
    {
        initShell();
    }
}
