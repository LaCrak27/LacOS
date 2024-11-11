#include "../drivers/screen.h"
#include "./util.h"
#include "../drivers/keyboard.h"

char *readLine();
int execLine(int argc, char **argv);
int sh_cls(int argc, char **argv);

char *builtin_cmds[] = {
    "clear",
    NULL
};

int (*builtin_func[]) (int, char **) = {
  &sh_cls
};

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
        if (line[0] != 0) // If line is not null, process command, otherwise just make new line
        {
            int argc = arrlen((void**)args);
            execLine(argc, args);
        }
        free(line);
        freearr_str(args);
    }
}

char *readLine()
{
    int currentLineLenght = 0;
    char *lineContent = (char *)malloc(sizeof(char) * 77); // Allocate one line worth of data
    if (!lineContent)
    {
        except("Error allocating line");
    }
    memset(lineContent, 0, 77 * sizeof(char)); // Clear buffer memory
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

int execLine(int argc, char **argv)
{
    int retCode = 1;
    int i = 0;
    while(builtin_cmds[i] != NULL)
    {
        if (strcmp(builtin_cmds[i], argv[0]))
        {
            int r = (*builtin_func[i])(argc, argv);
            print("Command returned with code ");
            println(itoa(r));
            return 0;
        }
        i++;
    }
    print("Command '");
    print(argv[0]);
    println("' not found.");
    return 1;
}

int sh_cls(int argc, char **argv)
{
    clear_screen();
    return 0;
}