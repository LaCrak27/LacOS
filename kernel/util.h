#define NULL 0
#define TRUE 1
#define FALSE 0
#define CHAR_MAX 127
#define CHAR_MIN -128
#define UCHAR_MAX 255
#define INT_MIN -2147483648
#define INT_MAX 2147483647
#define UINT_MAX 4294967295
void memcpy(char *source, char *dest, int n);
int strlen(char* str);
int strcmp(char *str1, char *str2);
void strcpy(char *source, char *dest);
void memset(void *dest, char val, unsigned long n);
void trim(char *str, char trim);
char **strsplt(char *str, char delim);
char *uitoh(unsigned int i);
char *uitohp(unsigned int i, int digits);
char *uctoh(unsigned char c);
char *itoa(int i);
char *uitoa(unsigned int i);
int atoi(char *str);
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

