#include "util.h"
#include "memory.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../interrupts/idt.h"

void except(char *msg)
{
    except_intern(msg);
}

// Memory stuff

// Copies n bytes from source to dest.
void memcpy(char *source, char *dest, int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        *(dest + i) = *(source + i);
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
    memcpy(str + i, str, len - i + 1); // Remove leading characters by shifting the whole string
    i = len - i - 1;                   // Make i point to last char
    while (str[i] == trim)             // Remove trailing chars
    {
        str[i] = 0;
        i--;
    }
}

int strcmp(char *str1, char *str2)
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if (len1 != len2)
    {
        return 0;
    }
    for (int i = 0; i < len1; i++)
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
    }
    return 1;
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
    char *buffer = malloc(i + 1 * sizeof(char)); // Buffer to hold current element
    if (!buffer)
    {
        except("Error allocating buffer");
    }
    char **res = (char **)malloc((elementCount + 1) * sizeof(char *)); // Array of strings to return
    if (!res)
    {
        except("Error allocating res array");
    }
    res[elementCount] = NULL;
    int currentElement = 0; // Current element index
    int currentSize = 0;    // Current element size
    i = 0;
    int j = 0;              // Segment index
    while (str[i] != 0)
    {
        if (str[i] == delim)
        {
            res[currentElement] = (char *)malloc(sizeof(char) * (currentSize + 1));
            if (!res[currentElement])
            {
                except("Error allocating element");
            }
            memcpy(buffer, res[currentElement], currentSize); // Store segment
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
        except("Error allocating element");
    }
    memcpy(buffer, res[currentElement], currentSize); // Store segment
    res[currentElement][currentSize] = NULL;          // Add null terminator
    free(buffer);
    return res;
}

// X -> str functions

#define INT_DIGITS 19 // Works until 64bit
#define UINT_DIGITS 20

// Converts unsigned long to it's hex string representation.
char *uitoh(unsigned int i)
{
    static char buf[19];
    char *p = buf + 18; // Points to terminating 0
    if(i == 0)
    {
        *--p = '0';
    }
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

// Converts unsigned long to it's hex string representation.
// Padded (0x23 & digits = 4) -> 0x0023
char *uitohp(unsigned int i, int digits)
{
    if(digits > 15) digits = 15;
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
    for(int i = 0; i < 2; i++)
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
    while(str[i] != NULL)
    {
        if(str[i] < '0' || str[i] > '9') // Character is not a number
        {
            return -1;
        }
        else
        {
            if(res > INT_MAX / 10)
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