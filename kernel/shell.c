#include "../drivers/screen.h"
#include "./util.h"
#include "../interrupts/idt.h"
#include "../drivers/keyboard.h"

char processCommand = 0;
char shellActive = 1;
void initShell()
{
    println("Shell started correctly!");
    print("$>");
    while (!processCommand)
    {
        char pressedKey = readKey();
        if (pressedKey == '\n')
        {
            printc('\n');
            print("$>");
            continue;
        }
        if (pressedKey == '\b')
        {
            if (get_cursor_col() <= 2) // 0-indexed
            {
                continue;
            }
        }
        printc(pressedKey);
    }
}
