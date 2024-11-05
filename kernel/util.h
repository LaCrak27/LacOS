#define NULL 0
void initmm();
void *malloc(unsigned long blockLenghth);
void free(void *ptr);
void *realloc(void *ptr, unsigned long newLenght);
void memcpy(char *source, char *dest, int no_bytes);
void memset(void *dest, char val, unsigned long count);
char *uitoh(unsigned long i);
char *itoa(long i);
char *uitoa(unsigned long i);

struct MapEntryStruct
{
    unsigned long long base;
    unsigned long long lenght;
    unsigned long type;
    unsigned long acpi;
} __attribute__((packed));

// <isBlockFree>(8)|<blockSizeInBytes>(32)|<previousBlockAdress>(32)|<nextBlockAdress>(32)
struct MemoryBlockHeader
{
    unsigned char isBlockFree;
    unsigned long blockSize;
    struct MemoryBlockHeader *previousBlockAdress;
    struct MemoryBlockHeader *nextBlockAdress;
} __attribute__((packed));
