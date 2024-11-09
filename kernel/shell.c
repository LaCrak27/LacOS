#include "../drivers/screen.h"
#include "./util.h"
#include "../drivers/keyboard.h"
char *readLine();

void initShell()
{
    println("Shell started correctly!");
    char *line;
    char **args;
    int status;
    while (1)
    {
        print("$>");
        line = readLine();
        args = strsplt(line, ' ');
        if (line[0] != 0) // If line is not null
        {
            print("Command '");
            print(args[0]);
            println("' not found.");
        }
        free(line);
        freearr_str(args);
    }
}

char *readLine()
{
    int currentLineLenght = 0;
    char *lineContent = (char *)malloc(sizeof(char) * 77); // Allocate one line worth of data
    memset(lineContent, 0, 77 * sizeof(char));             // Clear buffer memory
    while (1)
    {
        char pressedKey = readKey();
        if (pressedKey == '\b')
        {
            if (get_cursor_col() <= 2) // 0-indexed
            {
                continue;
            }
            currentLineLenght--;
            lineContent[currentLineLenght] = 0;
            printc('\b');
            continue;
        }
        if (pressedKey == '\n')
        {
            printc('\n');
            return lineContent;
        }
        if (currentLineLenght < 77)
        {
            printc(pressedKey);
            lineContent[currentLineLenght] = pressedKey;
            currentLineLenght++;
        }
    }
}