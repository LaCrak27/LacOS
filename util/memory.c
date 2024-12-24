#include "memory.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../interrupts/idt.h"
#include "util.h"
#include "debug.h"

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
void writeHeader(MemoryBlockHeader *adress,
                 unsigned char isFree,
                 unsigned long blockSize,
                 MemoryBlockHeader *previousBlockAdress,
                 MemoryBlockHeader *nextBlockAdress);

MemoryBlockHeader *get_block(void *ptr);

static unsigned long long memSize = 0;
static unsigned long long memStartAdress = 0;
static char memoryInitialized = 0;

// Initializes memory manager, should only be ran once by the kernel at startup.
unsigned long initmm()
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
    // Write the base header at start of memory
    writeHeader((MemoryBlockHeader *)(unsigned long)memStartAdress, 0, 0, (MemoryBlockHeader *)0, ((MemoryBlockHeader *)0));
    memoryInitialized = 1;
    return memStartAdress;
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
void *malloc(unsigned long blockLength)
{
    if (blockLength == 0 || blockLength > memSize)
        return NULL;
    MemoryBlockHeader *blockPointer = (MemoryBlockHeader *)(unsigned long)memStartAdress;
    while (1)
    {
        if (blockPointer->magicNumber != 0x69)
        {
            except("Malloc blockpointer pointed to invalid block.");
        }
        // The block is free, big enough and there is a next block
        if (blockPointer->isBlockFree == 1 && blockPointer->blockSize > blockLength + sizeof(MemoryBlockHeader) && blockPointer->nextBlockAdress != 0)
        {
            MemoryBlockHeader *new = (MemoryBlockHeader *)(((char *)blockPointer) + sizeof(MemoryBlockHeader) + blockLength);
            writeHeader(new,
                        1,
                        blockPointer->blockSize - blockLength - sizeof(MemoryBlockHeader),
                        blockPointer,
                        blockPointer->nextBlockAdress);
            blockPointer->nextBlockAdress->previousBlockAdress = new;
            blockPointer->nextBlockAdress = new;
            blockPointer->blockSize = blockLength;
            blockPointer->isBlockFree = 0;
            break;
        }
        else if (blockPointer->nextBlockAdress == NULL) // There's no next block, create one if it's used or extend it if not used
        {
            if (blockPointer->isBlockFree)
            {
                blockPointer->blockSize = blockLength;
                blockPointer->isBlockFree = 0;
            }
            else
            {
                MemoryBlockHeader *newBlockPointer = (MemoryBlockHeader *)((char *)blockPointer + blockPointer->blockSize + sizeof(MemoryBlockHeader));
                newBlockPointer->blockSize = blockLength;
                newBlockPointer->isBlockFree = 0;
                newBlockPointer->nextBlockAdress = 0;
                newBlockPointer->previousBlockAdress = blockPointer;
                newBlockPointer->magicNumber = 0x69;
                blockPointer->nextBlockAdress = newBlockPointer;
                blockPointer = newBlockPointer;
            }
            break;
        }
        else // There is a next block, and the current one isn't big enough or occupied
        {
            blockPointer = blockPointer->nextBlockAdress;
        }
    }
    // End of block would land outside of usable memory, return null
    if ((void *)((char *)blockPointer + sizeof(MemoryBlockHeader)) > (void *)(unsigned long)(memStartAdress + memSize - blockLength))
    {
        blockPointer->previousBlockAdress->nextBlockAdress = (MemoryBlockHeader *) 0;
        return NULL;
    }
    return (void *)((char *)blockPointer + sizeof(MemoryBlockHeader));
}

// Frees block pointed to by ptr.
void free(void *ptr)
{
    MemoryBlockHeader *blockPtr = get_block(ptr); // Get pointer to header instead of content

    if (blockPtr->magicNumber != 0x69)
        except("Free called on an invalid adress.");
    if (blockPtr->isBlockFree == 0)
    {
        blockPtr->isBlockFree = 1;
    }
    return;
}

// Gets block from void pointer returned
MemoryBlockHeader *get_block(void *ptr)
{
    return (MemoryBlockHeader *)((char *)ptr - sizeof(MemoryBlockHeader));
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

// Reallocates provided block (ptr) with a new size (blockLenght).
void *realloc(void *ptr, unsigned long block_lenght)
{
    MemoryBlockHeader *blockPtr = get_block(ptr); // Get pointer to header instead of content
    // If this is the last block in the linked list, just extend it
    if(blockPtr->nextBlockAdress == NULL)
    {
        blockPtr->blockSize = block_lenght;
        return ptr;
    }
    void *newptr = malloc(block_lenght);
    if (newptr != NULL) // Only free memory
    {
        memcpy(ptr, newptr, block_lenght); // Copy contents
        free(ptr);
    }
    return newptr;
}

// Gets various stats about the heap. Mainly the allocated size, the used size, and the total memory.
void memstat(unsigned long *alloc_size, unsigned long *used_size, unsigned long *avail_size)
{
    *alloc_size = 0;
    *used_size = 0;
    *avail_size = memSize;
    MemoryBlockHeader *blockPointer = (MemoryBlockHeader *)(unsigned long)memStartAdress;
    while (blockPointer->nextBlockAdress != NULL) // Go through the whole linked list
    {
        if(!blockPointer->isBlockFree)
        {
            *used_size += blockPointer->blockSize;
        }
        *alloc_size += blockPointer->blockSize;
        blockPointer = blockPointer->nextBlockAdress;
    }
    return;
}