#ifndef COMP_DATE
#define COMP_DATE "Unknown Date"
#endif
#include "../drivers/screen.h"
#include "../drivers/serial.h"
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
    init_serial(0, COM1_PORT);
    clear_screen();
    write_serial('\r', COM1_PORT);
    // ASCII Art
    println(" __         ______     ______");
    println("/\\ \\       /\\  __ \\   /\\  ___\\");
    println("\\ \\ \\____  \\ \\  __ \\  \\ \\ \\____");
    println(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_____\\");
    println("  \\/_____/   \\/_/\\/_/   \\/_____/");
    println("");
    println(" ______     ______");
    println("/\\  __ \\   /\\  ___\\");
    println("\\ \\ \\_\\ \\  \\ \\___  \\");
    println(" \\ \\_____\\  \\/\\_____\\");
    println("  \\/_____/   \\/_____/");
    println("(c) LaCrak27 2023-2024. No rights reserved.");
    print("Compiled at: ");
    println(COMP_DATE);
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
    get_font();
    set_font();
    while (1) // Loop in case we exit shell somehow
    {
        init_shell();
    }
}
