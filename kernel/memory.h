void initmm();
void *malloc(unsigned long blockLenghth);
void except(char* msg);
void free(void *ptr);
void freearr_str(char **ptr);
void *realloc(void *ptr, unsigned long newLenght);
// <isBlockFree>(8)|<blockSizeInBytes>(32)|<previousBlockAdress>(32)|<nextBlockAdress>(32)
struct MemoryBlockHeader
{
    unsigned char magicNumber;
    unsigned char isBlockFree;
    unsigned long blockSize;
    struct MemoryBlockHeader *previousBlockAdress;
    struct MemoryBlockHeader *nextBlockAdress;
} __attribute__((packed));