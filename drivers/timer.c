#include "../kernel/low_level.h"
#include "../interrupts/idt.h"
#include "timer.h"
void irq0_handler_intern(struct InterruptRegisters *regs);

static unsigned long irqCount = 0;
void sleep(unsigned long millis)
{
    irqCount = 0;
    irqInstallHandler(0, &irq0_handler_intern);
    pByteOut(0x43, 0b00110100); // Channel 0, lo/hi access, Mode 2, Binary mode
    pByteOut(0x40, (PIT_FREQ / 1000) & 0xFF);
    pByteOut(0x40, (((PIT_FREQ / 1000) & 0xFF00)) >> 8); // Sets PIT to 1ms intervals
    while (irqCount < millis)
    {
        // Wait
    }
    irqUninstallHandler(0);
    return;
}

void irq0_handler_intern(struct InterruptRegisters *regs)
{
    irqCount++;
}