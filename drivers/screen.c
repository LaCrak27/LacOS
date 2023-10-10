#include "screen.h"
#include <low_level.h>
//Prints a character on the screen at a certain position or at the cursor's pos
void print_char(char character, int col, int row, char attribute_byte) 
{
    //Pointer that starts at the beggining of video memory
    unsigned char *vidmem = (unsigned char *) VIDEO_ADRESS;
    if(!attribute_byte) //If it is 0x00
    {
        attribute_byte = WHITE_ON_BLACK;
    }
    int offset;
    if(col >= 0 && row >= 0)
    {
        offset = get_screen_offset(col, row);
    }
    else
    {
        offset = get_cursor();
    }
    if(character == '\n')
    {
        int rows = offset / (2*MAX_COLS);
        offset = get_screen_offset(79, rows);
    }
    else
    {
        vidmem[offset] = character;
        vidmem[offset+1] = attribute_byte;
    }
    //Update offset
    offset += 2;
    offset = handle_scrolling(offset);
    set_cursor(offset);
}

int get_screen_offset(int colums, int rows)
{
    return (((rows * MAX_COLS) + colums) * 2);
}

int get_cursor()
{
    //reg 14: high byte of offset
    //reg 15: low byte of offset
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2; //Multiply it by 2 to get the memory adress.
}