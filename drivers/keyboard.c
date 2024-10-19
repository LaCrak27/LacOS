#include "../interrupts/idt.h"
#include "../kernel/low_level.h"

void *keyboardRoutines[1] = {
    0
};

// Essentialy the equivalent to registering an event
void keyboardInstallHandler(int num, void (*handler)(char scanCode, char press))
{
    keyboardRoutines[num] = handler;
}

// De register event
void keyboardUninstallHandler(int num)
{
    keyboardRoutines[num] = 0;
}


void keyboardHandler(struct InterruptRegisters *regs){

    char scanCode = pByteIn(0x60) & 0x7F; //What key is pressed
    char press = pByteIn(0x60) & 0x80; //Press down, or released

    void (*handler)(char scanCode, char press);

    handler = keyboardRoutines[0];

    if (handler)
    {
        handler(scanCode, press);
    }
}

void initKeyboard()
{
    irqInstallHandler(1,&keyboardHandler);
}