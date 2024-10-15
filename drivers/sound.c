#include "../kernel/low_level.h"
#include "../kernel/util.h"

// Source: osdev.org
void play_sound(long nFrequence)
{
    long Div;
    unsigned char tmp;

    // Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    port_byte_out(0x43, 0xb6);
    port_byte_out((unsigned short)0x42, (unsigned char)(Div));
    port_byte_out((unsigned short)0x42, (unsigned char)(Div >> 8));

    // And play the sound using the PC speaker
    tmp = port_byte_in(0x61);
    if (tmp != (tmp | 3))
    {
        port_byte_out(0x61, tmp | 3);
    }
}

void nosound()
{
    unsigned char tmp = port_byte_in(0x61) & 0xFC;
    port_byte_out(0x61, tmp);
}

void beep()
{
    play_sound(1000);
    //timer_wait(10);
    nosound();
}