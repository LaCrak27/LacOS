#ifndef __IDT_H__
#define __IDT_H__

typedef struct 
{
    unsigned short baseLow;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short baseHigh;
}__attribute__((packed)) IdtEntryStruct;

typedef struct
{
    unsigned short limit;
    unsigned long base;
}__attribute__((packed)) IdtPointerStruct;

typedef struct
{
    unsigned long cr2;
    unsigned long ds;
    unsigned long edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned long int_no, err_code;
    unsigned long eip, csm, eflags, eseresp, ss;
} InterruptRegisters;

void init_idt();
void set_idt_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

void isr_handler(InterruptRegisters* regs);
void irq_install_handler(int irq, void (*handler)(InterruptRegisters *r));
void irq_uninstall_handler(int irq);
void except_intern(char* errorMessage);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr128();
extern void isr177();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void idt_flush(unsigned long idtptr);

#endif // __IDT_H__