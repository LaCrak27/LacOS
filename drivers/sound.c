#include "../util/low_level.h"
#include "timer.h"

void play_sound(long nFrequence)
{
    long Div;

    // Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    outb(0x43, 0xb6);
    outb(0x42, (unsigned char)(Div));
    outb(0x42, (unsigned char)(Div >> 8));

    // And play the sound using the PC speaker
    unsigned char tmp;
    tmp = inb(0x61);
    if (tmp != (tmp | 3))
    {
        outb(0x61, tmp | 3);
    }
}

void nosound()
{
    unsigned char tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void beep()
{
    play_sound(1500);
    sleep(333);
    nosound();
}