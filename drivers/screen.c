#include "screen.h"
#include "../kernel/low_level.h"
#include "../kernel/util.h"

unsigned char attribute_byte = 0x07;
// Sets the foreground text color
void set_fg(unsigned char fgVal)
{
    attribute_byte &= 0xF0;
    attribute_byte |= fgVal;
}
// Sets the background text color
void set_bg(unsigned char bgVal)
{
    attribute_byte &= 0x0F;
    attribute_byte |= bgVal << 4;
}
// Sets the whole attribute byte
void set_attr_byte(unsigned char attrByte)
{
    attribute_byte = attrByte;
}

int lastOffset = 0;
void disable_cursor()
{
    // Disable Cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void erase_char()
{
    unsigned char *vidmem = (unsigned char *)VIDEO_ADRESS;
    int offset;
    offset = get_cursor();
    offset = offset -= 2;
    vidmem[offset] = ' ';
    vidmem[offset + 1] = attribute_byte;
    set_cursor(offset);
}

// Prints a character on the screen at a certain position or at the cursor's pos
void print_char(char character, int col, int row)
{
    // Pointer that starts at the beggining of video memory
    unsigned char *vidmem = (unsigned char *)VIDEO_ADRESS;
    int offset;
    if (col >= 0 && row >= 0)
    {
        offset = get_screen_offset(col, row);
    }
    else
    {
        offset = get_cursor();
    }
    if (character == '\b')
    {
        erase_char();
        return;
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
    outb(REG_SCREEN_CTRL, 14);
    int offset = inb(REG_SCREEN_DATA) << 8;
    outb(REG_SCREEN_CTRL, 15);
    offset += inb(REG_SCREEN_DATA);
    return offset * 2; // Multiply it by 2 to get the memory adress.
}

int get_cursor_row()
{
    int cursor = get_cursor() / 2;
    return cursor / MAX_COLS;
}

int get_cursor_col()
{
    int cursor = get_cursor() / 2;
    return cursor % MAX_COLS;
}

void set_cursor(int offset)
{
    offset /= 2; // From memory offset to cursor offset
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    outb(REG_SCREEN_CTRL, 15);
    outb(REG_SCREEN_DATA, offset);
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
        print_char(message[i++], col, row);
    }
}

void print(char *message)
{
    print_at(message, -1, -1);
}

void println(char *message)
{
    print(message);
    print("\n");
}

void printc(char charToPrint)
{
    print_char(charToPrint, -1, -1);
}

void clear_screen()
{
    int row = 0;
    int col = 0;
    for (row = 0; row < MAX_ROWS; row++)
    {
        for (col = 0; col < MAX_COLS; col++)
        {
            print_char(' ', col, row);
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
        memcpy((char *)(get_screen_offset(0, i) + VIDEO_ADRESS),
                (char *)(get_screen_offset(0, i - 1) + VIDEO_ADRESS),
                MAX_COLS * 2);
    }
    // Blank the last line
    char *last_line = (char *)(get_screen_offset(0, MAX_ROWS - 1) + VIDEO_ADRESS);
    for (i = 0; i < MAX_COLS * 2; i+=2)
    {
        last_line[i] = ' ';
        last_line[i+1] = attribute_byte;
    }
    // Generate new offset
    cursor_offset -= 2 * MAX_COLS;
    return cursor_offset;
}