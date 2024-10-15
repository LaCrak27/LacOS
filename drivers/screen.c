#include "screen.h"
#include "../kernel/low_level.h"
#include "../kernel/util.h"
// Prints a character on the screen at a certain position or at the cursor's pos
void print_char(char character, int col, int row, char attribute_byte)
{
    // Pointer that starts at the beggining of video memory
    unsigned char *vidmem = (unsigned char *)VIDEO_ADRESS;
    if (!attribute_byte) // If it is 0x00
    {
        attribute_byte = WHITE_ON_BLACK;
    }
    int offset;
    if (col >= 0 && row >= 0)
    {
        offset = get_screen_offset(col, row);
    }
    else
    {
        offset = get_cursor();
    }
    if (character == '\n')
    {
        int rows = offset / (2 * MAX_COLS);
        offset = get_screen_offset(79, rows);
    }
    else
    {
        vidmem[offset] = character;
        vidmem[offset + 1] = attribute_byte;
    }
    // Update offset
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
    // reg 14: high byte of offset
    // reg 15: low byte of offset
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2; // Multiply it by 2 to get the memory adress.
}

void set_cursor(int offset)
{
    offset /= 2; // From memory offset to cursor offset
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, offset);
}

void print_at(char *message, int col, int row)
{
    if (col >= 0 && row >= 0)
    {
        set_cursor(get_screen_offset(col, row));
    }
    int i = 0;
    while (message[i] != 0)
    {
        print_char(message[i++], col, row, WHITE_ON_BLACK);
    }
}

void print(char *message)
{
    print_at(message, -1, -1);
}

void clear_screen()
{
    int row = 0;
    int col = 0;
    for (row = 0; row < MAX_ROWS; row++)
    {
        for (col = 0; col < MAX_COLS; col++)
        {
            print_char(' ', col, row, WHITE_ON_BLACK);
        }
    }
    set_cursor(get_screen_offset(0, 0));
}

int handle_scrolling(int cursor_offset)
{
    if (cursor_offset < MAX_ROWS * MAX_COLS * 2)
    {
        return cursor_offset;
    }
    // Juggles all rows so that they go one before
    int i;
    for (i = 1; i < MAX_ROWS; i++)
    {
        memory_copy((char *)(get_screen_offset(0, i) + VIDEO_ADRESS),
                    (char * )(get_screen_offset(0, i - 1) + VIDEO_ADRESS),
                    MAX_COLS * 2);
    }
    // Blank the last line
    char *last_line = (char *)(get_screen_offset(0, MAX_ROWS - 1) + VIDEO_ADRESS);
    for (i = 0; i < MAX_COLS * 2; i++)
    {
        last_line[i] = 0;
    }
    // Generate new offset
    cursor_offset -= 2 * MAX_COLS;
    return cursor_offset;
}