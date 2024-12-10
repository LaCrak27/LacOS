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
println(" __         _____     ____");
println("/\\ \\       /\  __ \\   /\  ___\\");
println("\\ \\ \\____  \\ \\  __ \\  \\ \\ \\____");
println(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_____\\");
println("  \\/_____/   \\/_/\\/_/   \\/_____/");
println("");
println(" ______     _____");
println("/\\  __ \\   /\  ___\\");
println("\\ \\ \\_\\ \\  \\ \\___  \\");
println(" \\ \\_____\\  \\/\\_____\\");
println("  \\/_____/   \\/_____/    v0.5");
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
    print("Initializing keyboard...     ");
    initKeyboard();
    set_fg(GREEN);
    println("Done!");
    set_fg(GRAY);
    println("Initializing floppy...       ");
    if (!initFloppy())
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
        except("Magic byte check failed, bootloader could not find out memory map. Halting...");
    }
    while (1) // Loop in case we exit shell somehow
    {
        initShell();
    }
}
