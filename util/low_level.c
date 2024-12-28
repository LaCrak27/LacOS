#include "low_level.h"
unsigned char inb(unsigned short port)
{
    //Reads a byte from the specified port
    unsigned char result;
    asm volatile("in al, dx" : "=a" (result) : "d" (port));
    return result;
}

void outb(unsigned short port, unsigned char data)
{
    asm volatile("out dx, al" : :"a" (data), "d" (port));
}

unsigned short inw(unsigned short port)
{
    unsigned short result;
    asm volatile("in ax, dx" : "=a" (result) : "d" (port));
    return result;
}

void outw(unsigned short port, unsigned short data)
{
    asm volatile("out dx, ax" : : "a" (data), "d" (port));
}

void cli()
{
    asm volatile("cli");
}

void sti()
{
    asm volatile("sti");
}