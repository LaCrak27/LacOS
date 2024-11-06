#include "../drivers/screen.h"
#include "./util.h"
#include "../drivers/keyboard.h"
char *readLine();
char **splitLine(char *line);

void initShell()
{
    clear_screen();
    println("Shell started correctly!");
    char *line;
    char **args;
    int status;
    while (1)
    {
        print("$>");
        line = readLine();
        if (line[0] != 0) // If line is not null
        {
            print("Command '");
            print(line);
            println("' not found.");
        }

        free(line);
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

char **splitLine(char *line)
{
    int numberOfWords = 0;
    int i = 0;
    while (1)
    {
        if(line[i] == 0) // If we've hit EOL (End Of Line)
        {
            break;
        }
        if (line[i] == ' ')
        {
            numberOfWords++;
        }
        i++;
    }
    char **args = (char**)malloc(sizeof(char*) * numberOfWords);
    char *currentWord = malloc(sizeof(char) * 76);
    while (1)
    {
        if(line[i] == 0)
        {
            break;
        }
    }
    
}