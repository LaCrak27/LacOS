#include "util.h"
#include "../drivers/screen.h"
#define INT_DIGITS 19 // Works until 64bit
#define UINT_DIGITS 20

unsigned long long memSize = 0;
unsigned long long memStartAdress = 0;

void printMemoryMap()
{
    int entryIndex = 0;
    unsigned char *mapPointer = (char *)0x1001;
    struct MapEntryStruct *structPointer = (struct MapEntryStruct *)0x1002;
    // Ignore ACPI for now lmao
    for (int i = 0; i < *mapPointer; i++) // Once for each entry:
    {
        if (structPointer->lenght == 0)
        {
            continue;
        }
        if(structPointer->type == (unsigned long) 1)
        {
            if(structPointer->lenght >= memSize)
            {
                memSize = structPointer->lenght;
                memStartAdress = structPointer->base;
            }
        }
        structPointer++;
        entryIndex++;
    }
    print("Total usable memory size: ");
    print(ultoa((unsigned long) memSize / 1024));
    println("kb");

    print("Starting at adress: ");
    println(getHexULL(memStartAdress));
}

void memCopy(char *source, char *dest, int no_bytes)
{
    int i;
    for (i = 0; i < no_bytes; i++)
    {
        *(dest + i) = *(source + i);
    }
}

void memSet(void *dest, char val, unsigned long count)
{
    char *temp = (char *)dest;
    for (; count != 0; count--)
    {
        *temp++ = val;
    }
}

char* getHexULL(unsigned long long i)
{
    static char buf[19];
    char *p = buf + 18; // Points to terminating 0
    while (i != 0)
    {
        if ((i % 16) <= 9) // We got a number, represent with regular 0-9
        {
            *--p = '0' + (i % 16);
        }
        else
        {
            *--p = 'A' + ((i % 16) - 10);
        }
        i /= 16;
    }
    *--p = 'x';
    *--p = '0';
    return p;
}

// Source for itoa and uitoa: apple.com/opensource
char *itoa(int i)
{
    /* Room for INT_DIGITS digits, - and '\0' */
    static char buf[INT_DIGITS + 2] = {0};
    char *p = buf + INT_DIGITS + 1; /* points to terminating '\0' */
    if (i >= 0)
    {
        do
        {
            *--p = '0' + (i % 10);
            i /= 10;
        } while (i != 0);
        return p;
    }
    else
    { /* i < 0 */
        do
        {
            *--p = '0' - (i % 10);
            i /= 10;
        } while (i != 0);
        *--p = '-';
    }
    return p;
}

char *uitoa(unsigned int i)
{
    /* Room for UINT_DIGITS digits and '\0' */
    static char buf[UINT_DIGITS + 1] = {0};
    char *p = buf + UINT_DIGITS; /* points to terminating '\0' */
    do
    {
        *--p = '0' + (i % 10);
        i /= 10;
    } while (i != 0);
    return p;
}

char *ltoa(long i)
{
    /* Room for INT_DIGITS digits, - and '\0' */
    static char buf[INT_DIGITS + 2] = {0};
    char *p = buf + INT_DIGITS + 1; /* points to terminating '\0' */
    if (i >= 0)
    {
        do
        {
            *--p = '0' + (i % 10);
            i /= 10;
        } while (i != 0);
        return p;
    }
    else
    { /* i < 0 */
        do
        {
            *--p = '0' - (i % 10);
            i /= 10;
        } while (i != 0);
        *--p = '-';
    }
    return p;
}

char *ultoa(unsigned long i)
{
    /* Room for UINT_DIGITS digits and '\0' */
    static char buf[UINT_DIGITS + 1];
    char *p = buf + UINT_DIGITS; /* points to terminating '\0' */
    do
    {
        *--p = '0' + (i % 10);
        i /= 10;
    } while (i != 0);
    return p;
}