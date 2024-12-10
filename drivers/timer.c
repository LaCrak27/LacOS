#include "../util/low_level.h"
#include "../interrupts/idt.h"
#include "timer.h"
void irq0_handler_intern(struct InterruptRegisters *regs);

static unsigned long ms = 0;
void initTimer()
{
    ms = 0;
    irqInstallHandler(0, &irq0_handler_intern);
    outb(0x43, 0b00110100); // Channel 0, lo/hi access, Mode 2, Binary mode
    outb(0x40, (PIT_FREQ / 1000) & 0xFF);
    outb(0x40, (((PIT_FREQ / 1000) & 0xFF00)) >> 8); // Sets PIT to 1ms intervals
    return;
}

void sleep(unsigned long millis)
{
    unsigned long msToWaitTo = ms + millis;
    while (ms < msToWaitTo)
    {
        // Wait
    }
    return;
}

void irq0_handler_intern(struct InterruptRegisters *regs)
{
    ms++;
}

// Get number of milisecond since system start
unsigned long millis()
{
    return ms;
}
