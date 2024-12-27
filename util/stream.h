#ifndef __STREAM_H__
#define __STREAM_H__
typedef struct
{
    unsigned char *buffer;
    unsigned long position;
} Stream;

Stream *create_stream(unsigned char *buffer);
unsigned char s_get_byte(Stream *stream);
unsigned short s_get_short(Stream *stream);
void free_stream(Stream *stream);

#endif // __STREAM_H__