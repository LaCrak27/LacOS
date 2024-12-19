#include "stream.h"
#include "memory.h"

Stream *create_stream(unsigned char *base_buffer)
{
    Stream *new = (Stream *)malloc(sizeof(Stream));
    new->buffer = base_buffer;
    new->position;
    return new;
}

unsigned char s_get_byte(Stream *stream)
{
    unsigned char res = (stream->buffer)[stream->position];
    stream->position = stream->position + 1;
    return res;
}

unsigned short s_get_short(Stream *stream)
{
    unsigned short res = (unsigned short)(stream->buffer[stream->position] & 0xFF) | 
    ((unsigned short)(stream->buffer[stream->position + 1] & 0xFF) << 8);
    stream->position = stream->position + 2;
    return res;
}

void free_stream(Stream *stream)
{
    free(stream);
    return;
}