#include "idt.h"
#include "../drivers/screen.h"
#include "../kernel/util.h"
#include "../kernel/low_level.h"

struct IdtEntryStruct idtEntries[256];
struct IdtPointerStruct idtPtr;

extern void idtFlush(unsigned long x);
void syshalt(char *errorMessage);

void initIdt()
{
    idtPtr.limit = sizeof(struct IdtEntryStruct) * 256 - 1;
    idtPtr.base = (unsigned long)&idtEntries;
    memset(&idtEntries, 0, sizeof(struct IdtEntryStruct) * 256);

    // (There are 2 PICs)
    // 0x20 commands & 0x21 data
    // 0xA0 commands & 0xA1 data
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    setIdtGate(0, (unsigned long)isr0, 0x08, 0x8E);
    setIdtGate(1, (unsigned long)isr1, 0x08, 0x8E);
    setIdtGate(2, (unsigned long)isr2, 0x08, 0x8E);
    setIdtGate(3, (unsigned long)isr3, 0x08, 0x8E);
    setIdtGate(4, (unsigned long)isr4, 0x08, 0x8E);
    setIdtGate(5, (unsigned long)isr5, 0x08, 0x8E);
    setIdtGate(6, (unsigned long)isr6, 0x08, 0x8E);
    setIdtGate(7, (unsigned long)isr7, 0x08, 0x8E);
    setIdtGate(8, (unsigned long)isr8, 0x08, 0x8E);
    setIdtGate(9, (unsigned long)isr9, 0x08, 0x8E);
    setIdtGate(10, (unsigned long)isr10, 0x08, 0x8E);
    setIdtGate(11, (unsigned long)isr11, 0x08, 0x8E);
    setIdtGate(12, (unsigned long)isr12, 0x08, 0x8E);
    setIdtGate(13, (unsigned long)isr13, 0x08, 0x8E);
    setIdtGate(14, (unsigned long)isr14, 0x08, 0x8E);
    setIdtGate(15, (unsigned long)isr15, 0x08, 0x8E);
    setIdtGate(16, (unsigned long)isr16, 0x08, 0x8E);
    setIdtGate(17, (unsigned long)isr17, 0x08, 0x8E);
    setIdtGate(18, (unsigned long)isr18, 0x08, 0x8E);
    setIdtGate(19, (unsigned long)isr19, 0x08, 0x8E);
    setIdtGate(20, (unsigned long)isr20, 0x08, 0x8E);
    setIdtGate(21, (unsigned long)isr21, 0x08, 0x8E);
    setIdtGate(22, (unsigned long)isr22, 0x08, 0x8E);
    setIdtGate(23, (unsigned long)isr23, 0x08, 0x8E);
    setIdtGate(24, (unsigned long)isr24, 0x08, 0x8E);
    setIdtGate(25, (unsigned long)isr25, 0x08, 0x8E);
    setIdtGate(26, (unsigned long)isr26, 0x08, 0x8E);
    setIdtGate(27, (unsigned long)isr27, 0x08, 0x8E);
    setIdtGate(28, (unsigned long)isr28, 0x08, 0x8E);
    setIdtGate(29, (unsigned long)isr29, 0x08, 0x8E);
    setIdtGate(30, (unsigned long)isr30, 0x08, 0x8E);
    setIdtGate(31, (unsigned long)isr31, 0x08, 0x8E);

    setIdtGate(32, (unsigned long)irq0, 0x08, 0x8E);
    setIdtGate(33, (unsigned long)irq1, 0x08, 0x8E);
    setIdtGate(34, (unsigned long)irq2, 0x08, 0x8E);
    setIdtGate(35, (unsigned long)irq3, 0x08, 0x8E);
    setIdtGate(36, (unsigned long)irq4, 0x08, 0x8E);
    setIdtGate(37, (unsigned long)irq5, 0x08, 0x8E);
    setIdtGate(38, (unsigned long)irq6, 0x08, 0x8E);
    setIdtGate(39, (unsigned long)irq7, 0x08, 0x8E);
    setIdtGate(40, (unsigned long)irq8, 0x08, 0x8E);
    setIdtGate(41, (unsigned long)irq9, 0x08, 0x8E);
    setIdtGate(42, (unsigned long)irq10, 0x08, 0x8E);
    setIdtGate(43, (unsigned long)irq11, 0x08, 0x8E);
    setIdtGate(44, (unsigned long)irq12, 0x08, 0x8E);
    setIdtGate(45, (unsigned long)irq13, 0x08, 0x8E);
    setIdtGate(46, (unsigned long)irq14, 0x08, 0x8E);
    setIdtGate(47, (unsigned long)irq15, 0x08, 0x8E);

    // System calls
    setIdtGate(128, (unsigned long)isr128, 0x08, 0x8E);
    setIdtGate(177, (unsigned long)isr177, 0x08, 0x8E);

    idt_flush((unsigned long)&idtPtr);
}

void setIdtGate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    idtEntries[num].baseLow = base & 0xFFFF;
    idtEntries[num].baseHigh = (base >> 16) & 0xFFFF;
    idtEntries[num].sel = sel;
    idtEntries[num].always0 = 0; // duh
    idtEntries[num].flags = flags | 0x60;
}

char *exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Fault",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"};

void isr_handler(struct InterruptRegisters *regs)
{
    if (regs->int_no < 32)
    {
        syshalt((exception_messages[regs->int_no]));
    }
}

void except_intern(char* errorMessage)
{
    syshalt(errorMessage);
}

void syshalt(char *errorMessage)
{
    println("SYSTEM HALTED!!! Error:");
    print(errorMessage);
    disable_cursor();
    for (;;)
        ;
}

void *irqRoutines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

// Essentialy the equivalent to registering an event
void irqInstallHandler(int irq, void (*handler)(struct InterruptRegisters *r))
{
    irqRoutines[irq] = handler;
}

// De register event
void irqUninstallHandler(int irq)
{
    irqRoutines[irq] = 0;
}

void irq_handler(struct InterruptRegisters *regs)
{
    void (*handler)(struct InterruptRegisters *regs);

    handler = irqRoutines[regs->int_no - 32];

    if (handler)
    {
        handler(regs);
    }

    if (regs->int_no >= 40)
    {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}

void cli()
{
    asm volatile("cli");
}

void sti()
{
    asm volatile("sti");
}
