#include "idt.h"
#include "../drivers/screen.h"
#include "../util/util.h"
#include "../util/low_level.h"
#include "../util/debug.h"

IdtEntryStruct idtEntries[256];
IdtPointerStruct idtPtr;

extern void idt_flush(unsigned long x);
void syshalt(char *errorMessage, InterruptRegisters *regs);

void init_idt()
{
    idtPtr.limit = sizeof(IdtEntryStruct) * 256 - 1;
    idtPtr.base = (unsigned long)&idtEntries;
    memset(&idtEntries, 0, sizeof(IdtEntryStruct) * 256);

    // (There are 2 PICs)
    // 0x20 commands & 0x21 data
    // 0xA0 commands & 0xA1 data
    // Fun fact, the slave pic is connected to the master PIC
    // via the IRQ2 line, so that's why you can never get an IRQ2.
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

    set_idt_gate(0, (unsigned long)&isr0, 0x08, 0x8E);
    set_idt_gate(1, (unsigned long)&isr1, 0x08, 0x8E);
    set_idt_gate(2, (unsigned long)&isr2, 0x08, 0x8E);
    set_idt_gate(3, (unsigned long)&isr3, 0x08, 0x8E);
    set_idt_gate(4, (unsigned long)&isr4, 0x08, 0x8E);
    set_idt_gate(5, (unsigned long)&isr5, 0x08, 0x8E);
    set_idt_gate(6, (unsigned long)&isr6, 0x08, 0x8E);
    set_idt_gate(7, (unsigned long)&isr7, 0x08, 0x8E);
    set_idt_gate(8, (unsigned long)&isr8, 0x08, 0x8E);
    set_idt_gate(9, (unsigned long)&isr9, 0x08, 0x8E);
    set_idt_gate(10, (unsigned long)&isr10, 0x08, 0x8E);
    set_idt_gate(11, (unsigned long)&isr11, 0x08, 0x8E);
    set_idt_gate(12, (unsigned long)&isr12, 0x08, 0x8E);
    set_idt_gate(13, (unsigned long)&isr13, 0x08, 0x8E);
    set_idt_gate(14, (unsigned long)&isr14, 0x08, 0x8E);
    set_idt_gate(15, (unsigned long)&isr15, 0x08, 0x8E);
    set_idt_gate(16, (unsigned long)&isr16, 0x08, 0x8E);
    set_idt_gate(17, (unsigned long)&isr17, 0x08, 0x8E);
    set_idt_gate(18, (unsigned long)&isr18, 0x08, 0x8E);
    set_idt_gate(19, (unsigned long)&isr19, 0x08, 0x8E);
    set_idt_gate(20, (unsigned long)&isr20, 0x08, 0x8E);
    set_idt_gate(21, (unsigned long)&isr21, 0x08, 0x8E);
    set_idt_gate(22, (unsigned long)&isr22, 0x08, 0x8E);
    set_idt_gate(23, (unsigned long)&isr23, 0x08, 0x8E);
    set_idt_gate(24, (unsigned long)&isr24, 0x08, 0x8E);
    set_idt_gate(25, (unsigned long)&isr25, 0x08, 0x8E);
    set_idt_gate(26, (unsigned long)&isr26, 0x08, 0x8E);
    set_idt_gate(27, (unsigned long)&isr27, 0x08, 0x8E);
    set_idt_gate(28, (unsigned long)&isr28, 0x08, 0x8E);
    set_idt_gate(29, (unsigned long)&isr29, 0x08, 0x8E);
    set_idt_gate(30, (unsigned long)&isr30, 0x08, 0x8E);
    set_idt_gate(31, (unsigned long)&isr31, 0x08, 0x8E);

    set_idt_gate(32, (unsigned long)&irq0, 0x08, 0x8E);
    set_idt_gate(33, (unsigned long)&irq1, 0x08, 0x8E);
    set_idt_gate(34, (unsigned long)&irq2, 0x08, 0x8E);
    set_idt_gate(35, (unsigned long)&irq3, 0x08, 0x8E);
    set_idt_gate(36, (unsigned long)&irq4, 0x08, 0x8E);
    set_idt_gate(37, (unsigned long)&irq5, 0x08, 0x8E);
    set_idt_gate(38, (unsigned long)&irq6, 0x08, 0x8E);
    set_idt_gate(39, (unsigned long)&irq7, 0x08, 0x8E);
    set_idt_gate(40, (unsigned long)&irq8, 0x08, 0x8E);
    set_idt_gate(41, (unsigned long)&irq9, 0x08, 0x8E);
    set_idt_gate(42, (unsigned long)&irq10, 0x08, 0x8E);
    set_idt_gate(43, (unsigned long)&irq11, 0x08, 0x8E);
    set_idt_gate(44, (unsigned long)&irq12, 0x08, 0x8E);
    set_idt_gate(45, (unsigned long)&irq13, 0x08, 0x8E);
    set_idt_gate(46, (unsigned long)&irq14, 0x08, 0x8E);
    set_idt_gate(47, (unsigned long)&irq15, 0x08, 0x8E);

    // System calls
    set_idt_gate(128, (unsigned long)isr128, 0x08, 0x8E);
    set_idt_gate(177, (unsigned long)isr177, 0x08, 0x8E);

    idt_flush((unsigned long)&idtPtr);
}

void set_idt_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
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
    "Detected Overflow",
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

void isr_handler(InterruptRegisters *regs)
{
    // println(uitoa(regs->int_no));
    if (regs->int_no < 32)
    {
        syshalt((exception_messages[regs->int_no]), regs);
    }
}

void panic_intern(char *errorMessage, InterruptRegisters *regs)
{
    syshalt(errorMessage, regs);
}

void syshalt(char *errorMessage, InterruptRegisters *regs)
{
    set_fg(WHITE);
    set_bg(BLUE);
    clear_screen();
    println("Kernel has panicked!!! Error message:\n");
    print(" ");
    println(errorMessage);
    println("");
    if (regs)
    {
        print("--------------------------------------------------------------------------------");
        println("GP Regs: ");
        print("EAX: ");
        println(uitohp(regs->eax, 8));
        print("EBX: ");
        println(uitohp(regs->ebx, 8));
        print("ECX: ");
        println(uitohp(regs->ecx, 8));
        print("EDX: ");
        println(uitohp(regs->edx, 8));
        print("--------------------------------------------------------------------------------");
        print("EFLAGS: ");
        println(uitohp(regs->eflags, 8));
        print("--------------------------------------------------------------------------------");
        print("ESP: ");
        println(uitohp(regs->esp, 8));
        print("--------------------------------------------------------------------------------");
        print("EIP: ");
        println(uitohp(regs->eip, 8));
    }
    print("--------------------------------------------------------------------------------");
    println("Please reboot your computer");
    disable_cursor();
    cli();
    hlt();
}

void *irqRoutines[17] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0};

// Essentialy the equivalent to registering an event
void irq_install_handler(int irq, void (*handler)(InterruptRegisters *r))
{
    irqRoutines[irq] = handler;
}

// De register event
void irq_uninstall_handler(int irq)
{
    irqRoutines[irq] = 0;
}

void irq_handler(InterruptRegisters *regs)
{
    void (*handler)(InterruptRegisters *regs);

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
