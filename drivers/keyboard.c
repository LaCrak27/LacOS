#include "../interrupts/idt.h"
#include "../util/low_level.h"
#include "screen.h"
#include "keyboard.h"
#include "serial.h"
#include "../util/util.h"

void e_kpress(char scanCode, char press);

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
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};

const unsigned long uppercase[128] = {
    UNKNOWN, ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', CTRL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', LSHFT, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', RSHFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLCK, SCRLCK, HOME, UP, PGUP, '-',
    LEFT, UNKNOWN, RIGHT, '+', END, DOWN, PGDOWN, INS, DEL, UNKNOWN, UNKNOWN, UNKNOWN, F11, F12, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};

char waitingForKey = 0;
char resKey;
// Reads a key from the keyboard. Blocking operation.
// The reason why it returns an unsigned long instead of a char is because the keyboard can also send special keys, and they are unsigned longs.
// A quick check for seeing if a key is special or not is to bitshift to the right by 8 and checking if the result is zero.
unsigned long read_key()
{
    waitingForKey = 1;
    resKey = 0;
    while (waitingForKey)
    {
        if (serial_available())
        {
            if (inb(COM1_PORT + 5) & 1)
            {
                char r = inb(COM1_PORT);
                switch (r)
                {
                case 13:
                    r = '\n';
                    break;

                case 127:
                    r = '\b';
                    break;

                default:
                    break;
                }
                return r;
            }
        }
    }
    return resKey;
}

void keyboard_handler(InterruptRegisters *regs)
{
    char scanCode = inb(0x60) & 0x7F; // What key is pressed
    char press = inb(0x60) & 0x80;    // Press down, or released

    e_kpress(scanCode, press);
}

void init_keyboard()
{
    irq_install_handler(1, &keyboard_handler);
}

void e_kpress(char scanCode, char press)
{
    // press is inverted for some reason
    static char capsOn = 0;
    static char capsLock = 0;
    static char ctrlPressed = 0;
    switch (scanCode)
    {
    case 29: // LCtrl
        if(!press)
        {
            ctrlPressed = 1;
        }
        else
        {
            ctrlPressed = 0;
        }
        break;
    case 42:
        // shift key
        if (!press)
        {
            capsOn = 1;
        }
        else
        {
            capsOn = 0;
        }
        break;
    case 58:
        if (!capsLock && !press)
        {
            capsLock = 1;
        }
        else if (capsLock && !press)
        {
            capsLock = 0;
        }
        break;
    default:
        if (!press)
        {
            if(lowercase[scanCode] == 'c' && ctrlPressed)
            {
                outb(0x20, 0x20); // Aknowledge interrupt to the PIC
                reset();
            }
            if (capsOn || capsLock)
            {
                resKey = (uppercase[scanCode]);
                waitingForKey = 0;
            }
            else
            {
                resKey = (lowercase[scanCode]);
                waitingForKey = 0;
            }
        }
        break;
    }
}