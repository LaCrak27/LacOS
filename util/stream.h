typedef struct
{
    unsigned char *buffer;
    unsigned long position;
} Stream;

Stream *create_stream(unsigned char *buffer);
unsigned char s_get_byte(Stream *stream);
unsigned short s_get_short(Stream *stream);