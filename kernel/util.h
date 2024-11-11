#define NULL 0
#define TRUE 1
#define FALSE 0
void initmm();
void *malloc(unsigned long blockLenghth);
void except(char* msg);
void free(void *ptr);
void freearr_str(char **ptr);
void *realloc(void *ptr, unsigned long newLenght);
void memcpy(char *source, char *dest, int n);
int strlen(char* str);
int strcmp(char *str1, char *str2);
void memset(void *dest, char val, unsigned long n);
void trim(char *str, char trim);
char **strsplt(char *str, char delim);
char *uitoh(unsigned long i);
char *itoa(long i);
char *uitoa(unsigned long i);
int max(int a, int b);
int min(int a, int b);
int arrlen(void **arr);

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
    unsigned char magicNumber;
    unsigned char isBlockFree;
    unsigned long blockSize;
    struct MemoryBlockHeader *previousBlockAdress;
    struct MemoryBlockHeader *nextBlockAdress;
} __attribute__((packed));
