void memCopy(char* source, char* dest, int no_bytes);
void memSet(void *dest, char val, unsigned long count);
char * getHexULL(unsigned long long i);
char * itoa(int i);
char * uitoa(unsigned int i);
char * ltoa(long i);
char * ultoa(unsigned long i);
void printMemoryMap();

struct MapEntryStruct
{
    unsigned long long base;
    unsigned long long lenght;
    unsigned long type;
    unsigned long acpi;
}__attribute__((packed));
