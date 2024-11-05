#include "../drivers/screen.h"
#include "./util.h"
#include "../drivers/keyboard.h"

char processCommand = 0;
char shellActive = 1;
void initShell()
{
    clear_screen();
    int currentLineLenght = 0;
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
            currentLineLenght--;
            printc('\b');
            continue;
        }
        if(currentLineLenght <= 76)
        {
            printc(pressedKey);
            currentLineLenght++;
        }
    }
}
