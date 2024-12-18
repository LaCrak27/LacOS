#include "../drivers/screen.h"
#include "../drivers/floppy.h"
#include "../drivers/timer.h"
#include "../drivers/sound.h"
#include "../util/util.h"
#include "../util/memory.h"
#include "./shell.h"
#include "../interrupts/idt.h"
#include "../drivers/keyboard.h"
#include "../util/debug.h"

void main()
{
    clear_screen();
    // ASCII Art
    println(" __         ______     ______");
    println("/\\ \\       /\\  __ \\   /\\  ___\\");
    println("\\ \\ \\____  \\ \\  __ \\  \\ \\ \\____");
    println(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_____\\");
    println("  \\/_____/   \\/_/\\/_/   \\/_____/");
    println("");
    println(" ______     _____");
    println("/\\  __ \\   /\\  ___\\");
    println("\\ \\ \\_\\ \\  \\ \\___  \\");
    println(" \\ \\_____\\  \\/\\_____\\");
    println("  \\/_____/   \\/_____/    v0.5");
    println("(c) LaCrak27 2023-2024. No rights reserved.");
    print("Initializing IDT...          ");
    init_idt();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    print("Initializing timer...        ");
    init_timer();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    print("Initializing keyboard...     ");
    init_keyboard();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    println("Initializing floppy...       ");
    if (!init_floppy())
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
    if (*(char *)0x1000 == 0x69) // Check if memory detection was succesful
    {
        initmm();
    }
    else
    {
        except("Magic byte check failed, bootloader could not find out memory map.");
    }
    while (1) // Loop in case we exit shell somehow
    {
        init_shell();
    }
}
