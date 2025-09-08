#ifndef __UTIL_H__
#define __UTIL_H__

#define NULL 0
#define TRUE 1
#define FALSE 0
#define CHAR_MAX 127
#define CHAR_MIN -128
#define UCHAR_MAX 255
#define INT_MIN -2147483648
#define INT_MAX 2147483647
#define UINT_MAX 4294967295
#define size(x) (sizeof(x)/sizeof(x[0]))
void panic(char *msg);
void memcpy(char *dest, char *source, int n);
int strlen(char* str);
int strcmp(char *str1, char *str2);
void strcpy(char *dest, char *src);
void strcat(char *str1, char *str2);
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
void reboot();
void reset();

typedef struct
{
    unsigned long long base;
    unsigned long long lenght;
    unsigned long type;
    unsigned long acpi;
} __attribute__((packed)) MapEntryStruct;

struct StackFrame 
{
    struct StackFrame* ebp;
    unsigned long eip;
} __attribute__((packed));

typedef struct StackFrame StackFrame;

#endif // __UTIL_H__