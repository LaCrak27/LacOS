#include "low_level.h"
unsigned char pByteIn(unsigned short port)
{
    //Reads a byte from the specified port
    unsigned char result;
    asm volatile("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void pByteOut(unsigned short port, unsigned char data)
{
    asm volatile("out %%al, %%dx" : :"a" (data), "d" (port));
}

unsigned short pWordIn(unsigned short port)
{
    unsigned short result;
    asm volatile("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void pWordOut(unsigned short port, unsigned short data)
{
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port));
}