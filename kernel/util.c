#include "util.h"
#include "../drivers/screen.h"
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

void writeHeader(MemoryBlockHeader *adress, unsigned char isFree, unsigned long blockSize, MemoryBlockHeader *previousBlockAdress, MemoryBlockHeader *nextBlockAdress)
{
    adress->isBlockFree = isFree;
    adress->blockSize = blockSize;
    adress->previousBlockAdress = previousBlockAdress;
    adress->nextBlockAdress = nextBlockAdress;
}

// Memory returned may be garbage
void *malloc(unsigned long blockLenghth)
{
    MemoryBlockHeader *blockPointer = (MemoryBlockHeader *)(unsigned long)memStartAdress;
    while (1)
    {
        if (blockPointer->isBlockFree == 1 && blockPointer->blockSize >= blockLenghth) // The block is free and big enough!
        {
            blockPointer->isBlockFree = 0;
            MemoryBlockHeader *nextBlock = (MemoryBlockHeader *)((char *)blockPointer + blockLenghth + sizeof(MemoryBlockHeader)); // Points to start of next block
            nextBlock->blockSize = blockPointer->blockSize - blockLenghth - sizeof(MemoryBlockHeader);
            nextBlock->isBlockFree = 1;
            nextBlock->nextBlockAdress = blockPointer->nextBlockAdress;
            nextBlock->previousBlockAdress = blockPointer;
            blockPointer->nextBlockAdress = nextBlock;
            break;
        }
        else if (blockPointer->nextBlockAdress == 0) // There's no next block, create one
        {
            MemoryBlockHeader *newBlockPointer = (MemoryBlockHeader *)((char *)blockPointer + blockPointer->blockSize + sizeof(MemoryBlockHeader));
            newBlockPointer->blockSize = blockLenghth;
            newBlockPointer->isBlockFree = 0;
            newBlockPointer->nextBlockAdress = 0;
            newBlockPointer->previousBlockAdress = blockPointer;
            blockPointer->nextBlockAdress = newBlockPointer;
            blockPointer = newBlockPointer;
            break;
        }
        else // There is a next block, and the current one isn't big enough or occupied
        {
            blockPointer = blockPointer->nextBlockAdress; // Go to next block lol
        }
    }
    // End of block would land outside of usable memory, return null
    if ((void *)((char *)blockPointer + sizeof(MemoryBlockHeader)) > (void*)(unsigned long)(memStartAdress + memSize - blockLenghth))
    {
        return NULL;
    }
    return (void *)((char *)blockPointer + sizeof(MemoryBlockHeader));
}

void free(void *ptr)
{
    MemoryBlockHeader *blockPtr = (MemoryBlockHeader *)((char *)ptr - sizeof(MemoryBlockHeader)); // Get pointer to header instead of content
    blockPtr->isBlockFree = 1;
    return;
}

void *realloc(void *ptr, unsigned long blockSize)
{
    void *newPtr = malloc(blockSize);
    if (newPtr != NULL)
    {
        memcpy(ptr, newPtr, blockSize); // Copy contents
        free(ptr);
    }
    return newPtr;
}

// Other memory stuff

void memcpy(char *source, char *dest, int no_bytes)
{
    int i;
    for (i = 0; i < no_bytes; i++)
    {
        *(dest + i) = *(source + i);
    }
}

void memset(void *dest, char val, unsigned long count)
{
    char *temp = (char *)dest;
    for (; count != 0; count--)
    {
        *temp++ = val;
    }
}

// X -> str functions

#define INT_DIGITS 19 // Works until 64bit
#define UINT_DIGITS 20
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

// Source for itoa and uitoa: apple.com/opensource
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