#include "../util/low_level.h"

int is_transmit_empty(unsigned short port);

static int available = 0;
int serial_available() 
{
    return available;
}

int init_serial(unsigned short baud, unsigned short port)
{
    outb(port + 1, 0x00);                   // Disable all interrupts
    outb(port + 3, 0x80);                   // Enable DLAB (set baud rate divisor)
    outb(port, baud & 0xFF);                // Set baud
    outb(port + 1, (baud >> 8) & 0xFF);   
    outb(port + 3, 0x03);                   // 8 bits, no parity, one stop bit
    outb(port + 2, 0xC7);                   // Enable FIFO, clear them, with 14-byte threshold
    outb(port + 4, 0x0B);                   // IRQs enabled, RTS/DSR set
    outb(port + 4, 0x1E);                   // Set in loopback mode to test the serial chip
    outb(port, 0xAE);     

    // Check if serial is faulty
    if (inb(port) != 0xAE)
    {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + 4, 0x0F);
    available = 1;
    return 0;
}

int is_transmit_empty(unsigned short port)
{
    return inb(port + 5) & 0x20;
}

void write_serial(char c, unsigned short port)
{
    while (is_transmit_empty(port) == 0)
        ;

    outb(port, c);
}