void initmm();
void *malloc(unsigned long blockLenghth);
void except(char* msg);
void free(void *ptr);
void freearr_str(char **ptr);
void *realloc(void *ptr, unsigned long blockLenght);
void memstat(unsigned long *alloc_size, unsigned long *used_size, unsigned long *avail_size);
// <isBlockFree>(8)|<blockSizeInBytes>(32)|<previousBlockAdress>(32)|<nextBlockAdress>(32)
struct MemoryBlockHeader
{
    unsigned char magicNumber;
    unsigned char isBlockFree;
    unsigned long blockSize;
    struct MemoryBlockHeader *previousBlockAdress;
    struct MemoryBlockHeader *nextBlockAdress;
} __attribute__((packed));