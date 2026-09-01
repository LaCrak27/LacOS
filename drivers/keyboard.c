#include "../interrupts/idt.h"
#include "../util/low_level.h"
#include "screen.h"
#include "keyboard.h"
#include "serial.h"
#include "../util/util.h"

void e_kpress(unsigned char scanCode, char press);

static const unsigned long lowercase[128] = {
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

static const unsigned long uppercase[128] = {
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
#ifdef COM1_SHELL
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
#endif
    }
    return resKey;
}

void keyboard_handler(InterruptRegisters *regs)
{
    unsigned char input = inb(0x60); 
    char press = !(input & 0x80);           // Press down, or released
    unsigned char scanCode = input & 0x7F;  // What key is pressed
    e_kpress(scanCode, press);
}

void init_keyboard()
{
    irq_install_handler(1, &keyboard_handler);
}

static char rShift = 0;
static char lShift = 0;
static char capsOn = 0;
static char capsLock = 0;
static char lCtrlPressed = 0;
static char rCtrlPressed = 0;
static char ctrlPressed = 0;
static char altPressed = 0;
static char deletePressed = 0;

void e_kpress(unsigned char scanCode, char press)
{
    static char extendedKey = 0;
    if (extendedKey)
    {
        switch (scanCode)
        {
        case 0x53: // Delete
            deletePressed = press;
            break;

        case 0x1D: // RCtrl
            rCtrlPressed = press;
            ctrlPressed = lCtrlPressed || rCtrlPressed;
            break;

        default:
            break;
        }
        extendedKey = 0;
    }
    else
    {
        switch (scanCode)
        {
        case 0x60:
            extendedKey = 1;
            break;
        case 0x1D: // LCtrl
            lCtrlPressed = press;
            ctrlPressed = lCtrlPressed || rCtrlPressed;
            break;
        case 0x38: // LAlt
            altPressed = press;
            break;
        case 0x2A:
            // LShift key
            lShift = press;
            capsOn = lShift || rShift;
            break;
        case 0x36:
            // RShift key
            rShift = press;
            capsOn = lShift || rShift;
            break;
        case 0x3A:
            if (!capsLock && press)
            {
                capsLock = 1;
            }
            else if (capsLock && press)
            {
                capsLock = 0;
            }
            break;
        default:
            if (!press || scanCode > 127)
                break;
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

            break;
        }
    }
    if (ctrlPressed && altPressed && deletePressed)
    {
        outb(0x20, 0x20); // Aknowledge interrupt to the PIC
        reboot();
    }
}