#include "util.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../interrupts/idt.h"

void except(char *msg)
{
    except_intern(msg);
}

// Memory manager stuff
// HOW IT WORKS (first fit):
// The idea that I've got for the memory manager, it will work as some sort of linked list, with linked
// used blocks. The idea is to have a header with every block, with a bunch of fields like size, status, etc.
// One of the fields will be an adress to the next block, and an adress to the previous block, so that we can
// navigate all the free blocks.

// We will need to defrag eventually when the memory gets full, which will take a long time.

// (you can check the header struct in the header file for this)
// The header will have this information: notation -> <nameOfField>(sizeInBits)
// <isBlockFree>(8)|<blockSizeInBytes>(32)|<previousBlockAdress>(32)|<nextBlockAdress>(32)

// There's always gonna be a block at the memory start adress pointing to the next adress after it

// Private function, types and variable declarations:

typedef struct MemoryBlockHeader MemoryBlockHeader;
void writeHeader(struct MemoryBlockHeader *adress,
                 unsigned char isFree,
                 unsigned long blockSize,
                 MemoryBlockHeader *previousBlockAdress,
                 MemoryBlockHeader *nextBlockAdress);

static unsigned long long memSize = 0;
static unsigned long long memStartAdress = 0;
static char memoryInitialized = 0;

// Initializes memory manager, should only be ran once by the kernel at startup.
void initmm()
{
    int entryIndex = 0;
    unsigned char *mapPointer = (char *)0x1001;
    struct MapEntryStruct *structPointer = (struct MapEntryStruct *)0x1002;
    for (int i = 0; i < *mapPointer; i++) // Once for each entry:
    {
        if (structPointer->type == (unsigned long)1) // If entry represents a usable block
        {
            if (structPointer->lenght >= memSize) // If it's bigger than the biggest recorded one
            {
                memSize = structPointer->lenght;
                memStartAdress = structPointer->base;
            }
        }
        structPointer++;
        entryIndex++;
    }
    print("Total usable memory size: ");
    print(uitoa(memSize / 1024));
    println("kb");

    print("Starting at address: ");
    println(uitoh(memStartAdress));
    // Write the base header at start of memory
    writeHeader((MemoryBlockHeader *)(unsigned long)memStartAdress, 0, 0, (MemoryBlockHeader *)0, ((MemoryBlockHeader *)0));
    memoryInitialized = 1;
}

// Writes a header corresponding to a memory block in the given adress
void writeHeader(MemoryBlockHeader *adress, unsigned char isFree, unsigned long blockSize, MemoryBlockHeader *previousBlockAdress, MemoryBlockHeader *nextBlockAdress)
{
    adress->magicNumber = 0x69;
    adress->isBlockFree = isFree;
    adress->blockSize = blockSize;
    adress->previousBlockAdress = previousBlockAdress;
    adress->nextBlockAdress = nextBlockAdress;
}

// Allocates blockLength bytes of memory, returning a void pointer to it.
// Keep in mind the contents of the memory returned may be garbage.
// Returns a NULL pointer on failure.
void *malloc(unsigned long blockLenghth)
{
    if(blockLenghth == 0 || blockLenghth > memSize)
    {
        return NULL;
    }
    MemoryBlockHeader *blockPointer = (MemoryBlockHeader *)(unsigned long)memStartAdress;
    while (1)
    {
        if(blockPointer->magicNumber != 0x69)
        {
            except("Weird things have happened");
        }
        // The block is free and big enough!
        if (blockPointer->isBlockFree == 1 && blockPointer->blockSize > blockLenghth + sizeof(MemoryBlockHeader))
        {
            blockPointer->isBlockFree = 0;
            break;
        }
        else if (blockPointer->nextBlockAdress == 0) // There's no next block, create one
        {
            MemoryBlockHeader *newBlockPointer = (MemoryBlockHeader *)((char *)blockPointer + blockPointer->blockSize + sizeof(MemoryBlockHeader));
            newBlockPointer->blockSize = blockLenghth;
            newBlockPointer->isBlockFree = 0;
            newBlockPointer->nextBlockAdress = 0;
            newBlockPointer->previousBlockAdress = blockPointer;
            newBlockPointer->magicNumber = 0x69;
            blockPointer->nextBlockAdress = newBlockPointer;
            blockPointer = newBlockPointer;
            break;
        }
        else if(blockPointer->magicNumber == 0x69) // There is a next block, and the current one isn't big enough or occupied
        {
            blockPointer = blockPointer->nextBlockAdress; // Go to next block lol
        }
    }
    // End of block would land outside of usable memory, return null
    if ((void *)((char *)blockPointer + sizeof(MemoryBlockHeader)) > (void *)(unsigned long)(memStartAdress + memSize - blockLenghth))
    {
        return NULL;
    }
    return (void *)((char *)blockPointer + sizeof(MemoryBlockHeader));
}

// Frees block pointed to by ptr.
void free(void *ptr)
{
    MemoryBlockHeader *blockPtr = (MemoryBlockHeader *)((char *)ptr - sizeof(MemoryBlockHeader)); // Get pointer to header instead of content
    
    if(blockPtr->magicNumber != 0x69)
    {
        except("Free called on an invalid adress.");
    }

    if (blockPtr->isBlockFree == 0)
    {
        blockPtr->isBlockFree = 1;
    }
    return;
}

// Frees a null terminated pointer array of strings recursively
void freearr_str(char **ptr)
{
    int i = 0;
    while (ptr[i] != 0)
    {
        free(ptr[i]);
        i++;
    }
    free(ptr);
    return;
}

// Reallocates provided block (ptr) with a new size (blockSize).
void *realloc(void *ptr, unsigned long blockSize)
{
    void *newPtr = malloc(blockSize);
    if (newPtr != NULL) // Only free memory
    {
        memcpy(ptr, newPtr, blockSize); // Copy contents
        free(ptr);
    }
    return newPtr;
}

// Other memory stuff

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
    if(len1 != len2)
    {
        return 0;
    }
    for(int i = 0; i < len1; i++)
    {
        if(str1[i] != str2[i])
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
    int j = 0; // Segment index
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
            res[currentElement][currentSize] = NULL;      // Add null terminator
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
char *uitoh(unsigned long i)
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

// Converts integer to string of its value.
char *itoa(long i)
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
char *uitoa(unsigned long i)
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
    if(a > b)
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
    if(a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

// Arr utils

// Gets lenght of a null terminated array of pointers
int arrlen(void **arr)
{
    int i = 0;
    while(arr[i] != NULL)
    {
        i++;
    }
    return i;
}