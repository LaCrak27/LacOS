#include "util.h"
#include "memory.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../interrupts/idt.h"
#include "debug.h"

void panic(char *msg)
{
    // TODO: Stack trace
    InterruptRegisters *regs = (InterruptRegisters *)malloc(sizeof(InterruptRegisters));

    asm("mov %0, eax" : "=rm" (regs->eax) : );
    asm("mov %0, ebx" : "=rm" (regs->ebx) : );
    asm("mov %0, ecx" : "=rm" (regs->ecx) : );
    asm("mov %0, edx" : "=rm" (regs->edx) : );

    print(uitoh((unsigned long) regs));
    BochsBreak();

    panic_intern(msg, regs);
}

// For the stack protector
void __stack_chk_fail(void)
{
    panic("Stack canary deleted!");
}

// Memory stuff

// Copies n bytes from source to dest.
void memcpy(char *dest, char *source, int n)
{
    for (; n != 0; n--)
    {
        *dest++ = *source++;
    }
}

// Sets n consecutives bytes starting at dest to val
void memset(void *dest, char val, unsigned long n)
{
    char *temp = (char *)dest;
    for (; n != 0; n--)
    {
        *temp++ = val;
    }
}

// Str utils

// Adds str2 at the end of str1.
// Make sure that the str1 has enough allocated space to hold all of str2 (incuding the null terminator!).
void strcat(char *str1, char *str2)
{
    int str1len = strlen(str1);
    int str2len = strlen(str2);
    memcpy(str1 + str1len, str2, str2len);
    str1[str1len + str2len] = 0;
}

// Gets length of null terminated string.
int strlen(char *str)
{
    int i = 0;
    while (str[i] != 0)
    {
        i++;
    }
    return i;
}

// Trims instances of trim char at the start and end of str
void trim(char *str, char trim)
{
    int len = strlen(str);
    int i = 0;
    while (str[i] == trim)
    {
        i++;
    }
    memcpy(str, str + i, len - i + 1); // Remove leading characters by shifting the whole string
    i = len - i - 1;                   // Make i point to last char
    while (str[i] == trim)             // Remove trailing chars
    {
        str[i] = 0;
        i--;
    }
}

// Compares str1 and str2.
int strcmp(char *str1, char *str2)
{
    int i = 0;
    for (; str1[i] != NULL; i++)
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
    }
    if (str2[i] != NULL)
        return 0;
    return 1;
}

// Copies source into dest
void strcpy(char *dest, char *source)
{
    int i = 0;
    while (source[i] != NULL)
    {
        dest[i] = source[i];
        i++;
    }
    dest[i] = NULL;
    return;
}

// Returns an array of strings that contains the different parts of the string that str has,
// delimeted by delim.
char **strsplt(char *str, char delim)
{
    trim(str, delim);
    int elementCount = 1; // Amount of elements
    int i = 0;            // String index
    while (str[i] != 0)
    {
        if (str[i] == delim)
        {
            elementCount++;
        }
        i++;
    }
    char *buffer = malloc(i + 1); // Buffer to hold current element
    if (!buffer)
    {
        panic("Error allocating buffer");
    }
    char **res = (char **)malloc((elementCount + 1) * sizeof(char *)); // Array of strings to return
    if (!res)
    {
        panic("Error allocating res array");
    }
    res[elementCount] = NULL;
    int currentElement = 0; // Current element index
    int currentSize = 0;    // Current element size
    i = 0;
    int j = 0; // Segment index
    while (str[i] != 0)
    {
        if (str[i] == delim)
        {
            res[currentElement] = (char *)malloc(sizeof(char) * (currentSize + 1));
            if (!res[currentElement])
            {
                panic("Error allocating element");
            }
            memcpy(res[currentElement], buffer, currentSize); // Store segment
            res[currentElement][currentSize] = NULL;          // Add null terminator
            currentElement++;
            currentSize = 0;
            j = 0;
        }
        else
        {
            buffer[j] = str[i];
            currentSize++;
            j++;
        }
        i++;
    }
    // Write last segment on end
    res[currentElement] = (char *)malloc(sizeof(char) * (currentSize + 1));
    if (!res[currentElement])
    {
        panic("Error allocating element");
    }
    memcpy(res[currentElement], buffer, currentSize); // Store segment
    res[currentElement][currentSize] = NULL;          // Add null terminator
    free(buffer);
    return res;
}

// X -> str functions

#define INT_DIGITS 19 // Works until 64bit
#define UINT_DIGITS 20

// Converts unsigned int to it's hex string representation.
char *uitoh(unsigned int i)
{
    static char buf[19];
    char *p = buf + 18; // Points to terminating 0
    if (i == 0)
    {
        *--p = '0';
    }
    while (i != 0)
    {
        if ((i % 16) <= 9)
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

// Converts unsigned long to it's hex string representation.
// Padded (0x23 & digits = 4) -> 0x0023
char *uitohp(unsigned int i, int digits)
{
    if (digits > 15)
        digits = 15;
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
        digits--;
    }
    while (digits > 0)
    {
        *--p = '0';
        digits--;
    }
    *--p = 'x';
    *--p = '0';
    return p;
}

// Converts unsigned char to hex representation (no 0x, use uitoh for that)
char *uctoh(unsigned char c)
{
    static char buf[3];
    char *p = buf + 2; // Points to terminating 0
    for (int i = 0; i < 2; i++)
    {
        if ((c % 16) <= 9) // We got a number, represent with regular 0-9
        {
            *--p = '0' + (c % 16);
        }
        else
        {
            *--p = 'A' + ((c % 16) - 10);
        }
        c /= 16;
    }
    return p;
}

// Converts integer to string of its value.
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

// Converts unsigned integer to string of its value.
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

// Int utils

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

// Parses (unsigned) int in string str, trims leading and trailing whitespaces.
// Returns -1 on failure (incompatible chars or out of range).
int atoi(char *str)
{
    trim(str, ' ');
    int res = 0;
    int i = 0;
    while (str[i] != NULL)
    {
        if (str[i] < '0' || str[i] > '9') // Character is not a number
        {
            return -1;
        }
        else
        {
            if (res > INT_MAX / 10)
            {
                return -1;
            }
            res += (str[i] - '0'); // That converts char to its numerical value
            res *= 10;
        }
        i++;
    }
    return res / 10;
}

// Arr utils

// Gets lenght of a null terminated array of pointers
int arrlen(void **arr)
{
    int i = 0;
    while (arr[i] != NULL)
    {
        i++;
    }
    return i;
}

// Hard reboots the machine
void reboot()
{
    unsigned char good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    while (1)
    {
    }
}

// Undefined behavior san
struct source_location
{
    char *file;
    unsigned long line;
    unsigned long column;
};

struct type_descriptor
{
    unsigned short kind;
    unsigned short info;
    char name[];
};

struct type_mismatch_info
{
    struct source_location location;
    struct type_descriptor *type;
    unsigned int *alignment;
    unsigned char type_check_kind;
};

void __ubsan_handle_type_mismatch_v1(struct type_mismatch_info *type_mismatch, unsigned int *pointer)
{
    char msg[100] = "Undefined behavior detected in file ";
    strcat(msg, type_mismatch->location.file);
    strcat(msg, ", line ");
    strcat(msg, uitoa(type_mismatch->location.line));
    strcat(msg, ".");
    panic(msg);
}

// Soft reboots the machine, not actually turning it off
void reset()
{
    asm(
        "mov ebp, 0x90000\n"
        "mov esp, ebp\n" // Clear stack
        "jmp 0x101F0"    // Jump to kmain
    );
}