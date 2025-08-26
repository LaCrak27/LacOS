#include "../drivers/screen.h"
#include "../util/util.h"
#include "../util/memory.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../drivers/sound.h"
#include "../drivers/floppy.h"
#include "../util/debug.h"
#include "../util/stream.h"

char *read_line();
int exec_line(int argc, char **argv);
int sh_cls(int argc, char **argv);
int sh_setfg(int argc, char **argv);
int sh_setbg(int argc, char **argv);
int sh_hlp(int argc, char **argv);
int sh_slp(int argc, char **argv);
int sh_echo(int argc, char **argv);
int sh_millis(int argc, char **argv);
int sh_fdump(int argc, char **argv);
int sh_meminfo(int argc, char **argv);
int sh_logo(int argc, char **argv);
int sh_graphics(int argc, char **argv);
int sh_bdpl(int argc, char **argv);
int sh_beep(int argc, char **argv);
int sh_reset(int argc, char **argv);
int sh_crash(int argc, char **argv);

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
    "logo",
    "graphics",
    "bdpl",
    "beep",
    "reset",
    "crash",
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
    &sh_meminfo,
    &sh_logo,
    &sh_graphics,
    &sh_bdpl,
    &sh_beep,
    &sh_reset,
    &sh_crash};

char lastLine[MAX_COLS - 2] = {0};
void init_shell()
{
    println("Shell started correctly!");
    char *line;
    char **args;
    while (1)
    {
        line = read_line();
        args = strsplt(line, ' ');
        if (line[0] != 0) // If line is not null, process command, otherwise just make new line
        {
            int argc = arrlen((void **)args);
            exec_line(argc, args);
        }
        strcpy(line, lastLine);
        free(line);
        freearr_str(args);
    }
}

char *read_line()
{
    print("$>");
    int currentLineLenght = 0;
    char *lineContent = (char *)malloc(sizeof(char) * (MAX_COLS - 2)); // Allocate one line worth of data
    if (!lineContent)
    {
        panic("Error allocating line.");
    }
    memset(lineContent, 0, (MAX_COLS - 3) * sizeof(char)); // Clear buffer memory
    while (1)
    {
        unsigned long pressedKey = read_key();
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

int exec_line(int argc, char **argv)
{
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
    for (unsigned char i = 0; i < 16; i++)
    {
        if (strcmp(argv[1], colors[i]))
        {
            set_fg(i);
            refresh_attr();
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
    for (unsigned char i = 0; i < 16; i++)
    {
        if (strcmp(argv[1], colors[i]))
        {
            set_bg(i);
            refresh_attr();
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
    if (!flp_avail())
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
    unsigned char *fd = malloc(floppy_dmalen);
    if (!fd)
        panic("Error allocating memory for dump.");

    flp_raw_read_cyl(cyl, fd);
    println("- - - - - - - - - - - - - - - CYLINDER DUMP - - - - - - - - - - - - - - -");
    println("C.ADDR  |  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F");
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
            if (read_key() == 'q')
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

int sh_graphics(int argc, char **argv)
{
    switch_graphics();
    unsigned char temp = 0;
    unsigned char temp2 = 0;
    temp = 0;
    for (int i = 0; i < 320 * 200; i++)
    {
        g_put_pixel_linear(i, temp);
        if (!((i + temp2) % 4000))
            temp++;
    }
    temp2++;
    read_key();
    switch_text();
    return 0;
}

int sh_logo(int argc, char **argv)
{
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
    println("  \\/_____/   \\/_____/");

    return 0;
}

/// TODO: SAFETY, THIS IS JUST A PROOF OF CONCEPT AND WILL ALMOST 100% FUCK UP THE MEMORY
int sh_bdpl(int argc, char **argv)
{
    clear_screen();
    set_fg(YELLOW);
    println("BDPL Player for LacOS 1.0");
    println("(c) LaCrak27");
    set_fg(GRAY);

    if (argc != 2 || atoi(argv[1]) == -1)
    {
        set_fg(RED);
        println("Incorrect input. Correct usage: ");
        println("bdpl <bufferSize>");
        set_fg(GRAY);
        return 1;
    }

    unsigned long size = atoi(argv[1]);
    // Buffer with actual data.
    unsigned char *buffer = (char *)malloc(size + 5);
    // Buffer from floppy read.
    unsigned char *fbuffer = (char *)malloc(floppy_dmalen);
    if (!buffer || !fbuffer)
    {
        println("Error allocating buffer");
        println("Hint: Check if you have enough memory by using meminfo");
        free(buffer);
        free(fbuffer);
        return 2;
    }
    unsigned long position = 0;
    int cylToRead = 0;
    println("Please insert disk and press any key to continue.");
    read_key();
    int ogsize = size;
    while (size > 0)
    {
        if (cylToRead > 79)
        {
            println("Please insert the next disk and press any key to continue, or press q to abort");
            if (read_key() == 'q')
            {
                free(fbuffer);
                free(buffer);
                return 3;
            }
            cylToRead = 0;
        }
        print("Remaining cylinders: ");
        println(itoa(size / floppy_dmalen));
        print("Reading cylinder: ");
        println(itoa(cylToRead));
        println("------------");
        while (flp_raw_read_cyl(cylToRead, fbuffer))
        {
            println("Retrying read...");
        }
        size -= floppy_dmalen;
        if (size >= 0)
        {
            memcpy(fbuffer, buffer + position, floppy_dmalen);
        }
        else
        {
            memcpy(fbuffer, buffer + position, floppy_dmalen + size);
        }
        position += floppy_dmalen;
        cylToRead++;
    }
    buffer[ogsize] = 0xFC;
    buffer[ogsize + 1] = 0xFF;
    buffer[ogsize + 2] = 0x69; // This could be any value
    buffer[ogsize + 3] = 0xFC;
    buffer[ogsize + 4] = 0xFF; // Terminate buffer (In case of misaligment, make it so it always detects it,
                               // even if the end of the file is messed up)
    clear_screen();
    set_fg(GREEN);
    println("Done loading!, hashing...");
    set_fg(GRAY);
    free(fbuffer);
    // Hash to check reading errors
    unsigned long hash = 0;
    position = 0;
    while (1)
    {
        if ((buffer[position] == 0xFC && buffer[position + 1] == 0xFF))
        {
            break;
        }
        hash += buffer[position] * 33;
        position++;
    }
    print("Hash: ");
    println(uitoa(hash));
    print("First 3 bytes: ");
    print(uctoh(buffer[0]));
    printc(' ');
    print(uctoh(buffer[1]));
    printc(' ');
    print(uctoh(buffer[2]));
    printc(' ');
    print("\nBuffer address: ");
    print(uitoh((unsigned)buffer));
    printc('\n');
    println("Press any key to start playback");
    read_key();
    switch_graphics();
    g_cls();
    position = 2;                // Skip 1st 0xFFFF
    unsigned long ft = millis(); // Frametime
    char c = 0;                  // Current color: 0->Black, 7->White
    unsigned fn = 0;             // Frame number
    while (1)
    {
        c = buffer[position] == 1 ? WHITE : BLACK; // Set starting frame color
        position++;
        unsigned short framePos = 0;
        while (1) // Start of new frame
        {
            unsigned short currShort = buffer[position] | buffer[position + 1] << 8;
            position += 2;
            if (currShort == 0xFFFF)
            {
                while (1)
                {
                    if (millis() >= ft + (33 * fn))
                        break;
                }
                fn++;
                break;
            }
            if (currShort == 0xFFFC)
            {
                switch_text();
                free(buffer);
                free(fbuffer);
                return 0;
            }
            for (int i = 0; i < currShort; i++)
            {
                g_put_pixel_linear(framePos, c);
                framePos++;
            }
            c = !c ? WHITE : BLACK; // Change value of c
        }
    }
    return 0;
}

int sh_beep(int argc, char **argv)
{
    int ms = atoi(argv[2]);
    int freq = atoi(argv[1]);
    if (freq == -1 || ms == -1)
    {
        println("Incorrect usage. Correct usage is 'beep <freq> <millis>.");
        return 1;
    }
    beep(freq, ms);
    return 0;
}

int sh_reset(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-hard"))
        {
            reboot();
        }
    }
    reset();
}

int sh_crash(int argc, char **argv)
{
    int funni = 0x69 / *(char *)0x7DF0;
}