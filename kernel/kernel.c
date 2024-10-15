#include "../drivers/screen.h"
#include "../drivers/sound.h"
#include "./util.h"
unsigned long fib(unsigned long n);

void main()
{
    unsigned long i = 0;
    int playing = 0;
    while(1)
    {
        if(!playing) 
        {
            play_sound(1000);
            playing = 1;
        }
        else
        {
            nosound();
            playing = 0;
        }
        unsigned long res = fib(i);
        print(ultoa(res));
        print("\n");
        i++;
    }
}

unsigned long fib(unsigned long n)
{
    if (n == 0)
    {
        return 1;
    }
    if (n == 1)
    {
        return 1;
    }
    return fib(n-1) + fib(n-2);
}