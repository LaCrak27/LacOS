#include "../drivers/screen.h"
#include "../util/util.h"
#include "../util/memory.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../drivers/sound.h"
#include "../drivers/floppy.h"

char *readLine();
int execLine(int argc, char **argv);
int sh_cls(int argc, char **argv);
int sh_setfg(int argc, char **argv);
int sh_setbg(int argc, char **argv);
int sh_hlp(int argc, char **argv);
int sh_slp(int argc, char **argv);
int sh_echo(int argc, char **argv);
int sh_millis(int argc, char **argv);
int sh_fdump(int argc, char **argv);
int sh_meminfo(int argc, char **argv);

char *builtin_cmds[] = {
    "clear",
    "setfg",
    "setbg",
    "help",
    "sleep",
    "echo",
    "millis",
    "fdump",
    "meminfo",
    NULL};

int (*builtin_func[])(int, char **) = {
    &sh_cls,
    &sh_setfg,
    &sh_setbg,
    &sh_hlp,
    &sh_slp,
    &sh_echo,
    &sh_millis,
    &sh_fdump,
    &sh_meminfo};

char lastLine[MAX_COLS - 2] = {0};
void initShell()
{
    println("Shell started correctly!");
    char *line;
    char **args;
    int status;
    while (1)
    {
        line = readLine();
        args = strsplt(line, ' ');
        if (line[0] != 0) // If line is not null, process command, otherwise just make new line
        {
            int argc = arrlen((void **)args);
            execLine(argc, args);
        }
        strcpy(line, lastLine);
        free(line);
        freearr_str(args);
    }
}

char *readLine()
{
    print("$>");
    int currentLineLenght = 0;
    char *lineContent = (char *)malloc(sizeof(char) * (MAX_COLS - 2)); // Allocate one line worth of data
    if (!lineContent)
    {
        except("Error allocating line.");
    }
    memset(lineContent, 0, (MAX_COLS - 3) * sizeof(char)); // Clear buffer memory
    while (1)
    {
        unsigned long pressedKey = readKey();
        if (pressedKey == '\b')
        {
            if (get_cursor_col() <= 2) // 0-indexed, if we get to the $>
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
        if (currentLineLenght < (MAX_COLS - 3))
        {
            switch (pressedKey)
            {
            case UP:
                strcpy(lastLine, lineContent);
                while (get_cursor_col() > 2)
                {
                    erase_char();
                }
                currentLineLenght = strlen(lineContent);
                print(lineContent);
                break;
            default:
                break;
            }
            if (pressedKey >> 8 == 0) // If Key isn't a special char
            {
                printc(pressedKey);
                lineContent[currentLineLenght] = pressedKey;
                currentLineLenght++;
            }
        }
    }
}

int execLine(int argc, char **argv)
{
    int retCode = 1;
    int i = 0;
    while (builtin_cmds[i] != NULL)
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

char *colors[] = {
    "black",
    "blue",
    "green",
    "cyan",
    "red",
    "purple",
    "brown",
    "gray",
    "darkgray",
    "lightblue",
    "lightgreen",
    "lightcyan",
    "lightred",
    "lightpurple",
    "yellow",
    "white"};

int sh_setfg(int argc, char **argv)
{
    if (argc != 2)
    {
        println("Incorrect usage. Correct usage is 'setfg <color>', where <color> can be:\nblack\nblue\ngreen\ncyan\nred\npurple\nbrown\ngray\ndarkgray\nlightblue\nlightgreen\nlightcyan\nlightred\nlightpurple\nyellow\nwhite");
        return 1;
    }
    for (char i = 0; i < 16; i++)
    {
        if (strcmp(argv[1], colors[i]))
        {
            set_fg(i);
            print("Foreground succesfully set to ");
            print(argv[1]);
            print("(");
            print(itoa(i));
            print(")");
            println(".");
            return 0;
        }
    }
    println("Incorrect usage. Correct usage is 'setfg <color>', where <color> can be:\nblack\nblue\ngreen\ncyan\nred\npurple\nbrown\ngray\ndarkgray\nlightblue\nlightgreen\nlightcyan\nlightred\nlightpurple\nyellow\nwhite");
    return 1;
}

int sh_setbg(int argc, char **argv)
{
    if (argc != 2)
    {
        println("Incorrect usage. Correct usage is 'setbg <color>', where <color> can be:\nblack\nblue\ngreen\ncyan\nred\npurple\nbrown\ngray\ndarkgray\nlightblue\nlightgreen\nlightcyan\nlightred\nlightpurple\nyellow\nwhite");
        return 1;
    }
    for (char i = 0; i < 16; i++)
    {
        if (strcmp(argv[1], colors[i]))
        {
            set_bg(i);
            print("Background succesfully set to ");
            print(argv[1]);
            println(".");
            return 0;
        }
    }
    println("Incorrect usage. Correct usage is 'setbg <color>', where <color> can be:\nblack\nblue\ngreen\ncyan\nred\npurple\nbrown\ngray\ndarkgray\nlightblue\nlightgreen\nlightcyan\nlightred\nlightpurple\nyellow\nwhite");
    return 1;
}

int sh_hlp(int argc, char **argv)
{
    println("Welcome to LacOS!\nFor now, this is a very very simple operating system.\nHere are the builtin commands:");
    int i = 0;
    while (builtin_cmds[i] != NULL)
    {
        println(builtin_cmds[i]);
        i++;
    }
    println("You can run each command to get extra information on it.\nOn help prompts, an argument surrounded with <> is mandatory, while an argument surrounded with [] is optional.");
    return 0;
}

int sh_slp(int argc, char **argv)
{
    int millis = atoi(argv[1]);
    if (millis < 0)
    {
        println("Incorrect usage. Correct usage is 'sleep <millis>', where millis is the number of milliseconds to sleep.");
        return 1;
    }
    else
    {
        sleep(millis);
    }
    return 0;
}

int sh_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        print(argv[i]);
        printc(' ');
    }
    printc('\n');
    return 0;
}

int sh_millis(int argc, char **argv)
{
    print("Time in ms since the machine has booted up: ");
    println(uitoa(millis()));
    return 0;
}

int sh_fdump(int argc, char **argv)
{
    if (!isFloppyAvailable())
    {
        println("Floppy not available. Please restart and try again");
        return 0xFF;
    }
    int paged = FALSE;
    int cyl = -1;
    for (int i = 1; i < argc; i++) // First arg is name of command
    {
        if (strcmp(argv[i], "-paged"))
        {
            paged = TRUE;
        }
        else
        {
            int readCyl = atoi(argv[i]);
            if (readCyl >= 0)
            {
                cyl = readCyl;
            }
        }
    }
    if (cyl < 0)
    {
        println("Incorrect usage, correct usage: fdump <cyl> [-paged].\nIf using paged view, press q to abort.");
        return 1;
    }
    println("Reading from floppy, please wait...");
    unsigned char *fd = malloc(floppy_dmalen * sizeof(unsigned char));
    if (!fd)
        except("Error allocating memory for dump.");
    floppyRawReadCyl(cyl, fd);
    println("- - - - - - - - - - - - - - - CYLINDER DUMP - - - - - - - - - - - - - - -");
    println("C.ADDR  |  00  01  02  03  04  55  06  07  08  09  0A  0B  0C  0D  0E  0F");
    println("-------------------------------------------------------------------------");
    for (int i = 0; i < floppy_dmalen; i += 16)
    {
        print(uitohp(i, 4));
        print("  |");
        for (int j = 0; j < 16; j++)
        {
            print("  ");
            print(uctoh(fd[i + j]));
        }
        if (paged)
        {
            if (readKey() == 'q')
            {
                println("\nq was pressed. Aborting...");
                free(fd);
                return 0;
            }
        }
        printc('\n');
    }
    free(fd);
    println("-------------------------------------------------------------------------");
    return 0;
}

int sh_meminfo(int argc, char **argv)
{
    unsigned long alloc;
    unsigned long total;
    unsigned long used;
    memstat(&alloc, &used, &total);
    println("Memory stats: ");
    print("Available memory: ");
    print(uitoa(total));
    println(" bytes.");

    print("Allocated memory: ");
    print(uitoa(alloc));
    print(" bytes (");
    print(itoa((int)(((double)alloc * 100) / total)));
    println("%).");

    print("Used memory: ");
    print(uitoa(used));
    print(" bytes (");
    print(itoa((int)(((double)used * 100) / total)));
    println("%).");
    return 0;
}