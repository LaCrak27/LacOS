#include "../interrupts/idt.h"
#include "../kernel/low_level.h"
#include "screen.h"

typedef struct InterruptRegisters InterruptRegisters;

void onKeyPress(char scanCode, char press);

const unsigned long UNKNOWN = 0xFFFFFFFF;
const unsigned long ESC = 0xFFFFFFFF - 1;
const unsigned long CTRL = 0xFFFFFFFF - 2;
const unsigned long LSHFT = 0xFFFFFFFF - 3;
const unsigned long RSHFT = 0xFFFFFFFF - 4;
const unsigned long ALT = 0xFFFFFFFF - 5;
const unsigned long F1 = 0xFFFFFFFF - 6;
const unsigned long F2 = 0xFFFFFFFF - 7;
const unsigned long F3 = 0xFFFFFFFF - 8;
const unsigned long F4 = 0xFFFFFFFF - 9;
const unsigned long F5 = 0xFFFFFFFF - 10;
const unsigned long F6 = 0xFFFFFFFF - 11;
const unsigned long F7 = 0xFFFFFFFF - 12;
const unsigned long F8 = 0xFFFFFFFF - 13;
const unsigned long F9 = 0xFFFFFFFF - 14;
const unsigned long F10 = 0xFFFFFFFF - 15;
const unsigned long F11 = 0xFFFFFFFF - 16;
const unsigned long F12 = 0xFFFFFFFF - 17;
const unsigned long SCRLCK = 0xFFFFFFFF - 18;
const unsigned long HOME = 0xFFFFFFFF - 19;
const unsigned long UP = 0xFFFFFFFF - 20;
const unsigned long LEFT = 0xFFFFFFFF - 21;
const unsigned long RIGHT = 0xFFFFFFFF - 22;
const unsigned long DOWN = 0xFFFFFFFF - 23;
const unsigned long PGUP = 0xFFFFFFFF - 24;
const unsigned long PGDOWN = 0xFFFFFFFF - 25;
const unsigned long END = 0xFFFFFFFF - 26;
const unsigned long INS = 0xFFFFFFFF - 27;
const unsigned long DEL = 0xFFFFFFFF - 28;
const unsigned long CAPS = 0xFFFFFFFF - 29;
const unsigned long NONE = 0xFFFFFFFF - 30;
const unsigned long ALTGR = 0xFFFFFFFF - 31;
const unsigned long NUMLCK = 0xFFFFFFFF - 32;

const unsigned long lowercase[128] = {
    UNKNOWN, ESC, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', CTRL,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', LSHFT, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/', RSHFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLCK, SCRLCK, HOME, UP, PGUP, '-', LEFT, UNKNOWN, RIGHT,
    '+', END, DOWN, PGDOWN, INS, DEL, UNKNOWN, UNKNOWN, UNKNOWN, F11, F12, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN
    };

const unsigned long uppercase[128] = {
    UNKNOWN, ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', LSHFT, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', RSHFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLCK, SCRLCK, HOME, UP, PGUP, '-',
    LEFT, UNKNOWN, RIGHT, '+', END, DOWN, PGDOWN, INS, DEL, UNKNOWN, UNKNOWN, UNKNOWN, F11, F12, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};

char waitingForKey = 0;
char readChar;
char readKey()
{
    waitingForKey = 1;
    readChar = 0;
    while (waitingForKey)
    {
        // Wait for key lol
    }
    return readChar;
}

void keyboardHandler(InterruptRegisters *regs)
{
    char scanCode = inb(0x60) & 0x7F; //What key is pressed
    char press = inb(0x60) & 0x80; //Press down, or released

    onKeyPress(scanCode, press);
}

void initKeyboard()
{
    irqInstallHandler(1,&keyboardHandler);
}

void onKeyPress(char scanCode, char press)
{
    static char capsOn = 0;
    static char capsLock = 0;
    switch (scanCode)
    {
    case 42:
        // shift key
        if (press == 0)
        {
            capsOn = 1;
        }
        else
        {
            capsOn = 0;
        }
        break;
    case 58:
        if (!capsLock && press == 0)
        {
            capsLock = 1;
        }
        else if (capsLock && press == 0)
        {
            capsLock = 0;
        }
        break;
    default:
        if (press == 0)
        {
            if (capsOn || capsLock)
            {
                readChar = (uppercase[scanCode]);
                waitingForKey = 0;
            }
            else
            {
                readChar = (lowercase[scanCode]);
                waitingForKey = 0;
            }
        }
        break;
    }
}