#include "screen.h"
#include "../util/low_level.h"
#include "../util/util.h"
#include "timer.h"
#include "serial.h"

char graphicsMode = TEXT;

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
    unsigned char *vidmem = (unsigned char *)TEXT_VIDEO_ADRESS;
    int offset;
    offset = get_cursor();
    offset -= 2;
    vidmem[offset] = ' ';
    vidmem[offset + 1] = attribute_byte;
    set_cursor(offset);
}

// Prints a character on the screen at a certain position or at the cursor's position.
// Also sends to COM1
void print_char(char character, int col, int row)
{
    if(graphicsMode == GRAPHICS)
    {
        if (serial_available())
            write_serial('\r', COM1_PORT);
        return;
    }
    // Pointer that starts at the beggining of video memory
    unsigned char *vidmem = (unsigned char *)TEXT_VIDEO_ADRESS;
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
        offset -= 2;
        vidmem[offset] = ' ';
        vidmem[offset + 1] = attribute_byte;
        set_cursor(offset);
        if (serial_available())
            write_serial('\b', COM1_PORT); 
            write_serial(' ', COM1_PORT); 
            write_serial('\b', COM1_PORT); 
        return;
    }
    if (character == '\n')
    {
        int rows = offset / (2 * MAX_COLS);
        offset = get_screen_offset(79, rows);
        // Serial only returns by itself when you press the enter key
        // so we need to make this so that it also does it when programs want newlines
        if (serial_available())
            write_serial('\r', COM1_PORT); 
    }
    else
    {
        vidmem[offset] = character;
        vidmem[offset + 1] = attribute_byte;
    }
    if (serial_available())
        write_serial(character, COM1_PORT);
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
        memcpy((char *)(get_screen_offset(0, i) + TEXT_VIDEO_ADRESS),
               (char *)(get_screen_offset(0, i - 1) + TEXT_VIDEO_ADRESS),
               MAX_COLS * 2);
    }
    // Blank the last line
    char *last_line = (char *)(get_screen_offset(0, MAX_ROWS - 1) + TEXT_VIDEO_ADRESS);
    for (i = 0; i < MAX_COLS * 2; i += 2)
    {
        last_line[i] = ' ';
        last_line[i + 1] = attribute_byte;
    }
    // Generate new offset
    cursor_offset -= 2 * MAX_COLS;
    return cursor_offset;
}

// Switches to text mode
// (mode 03h)
void switch_text()
{
}

// misc out (3c2h) value for various modes
#define R_COM 0x63 // "common" bits

#define R_W320 0x00

#define R_H200 0x00

// Switches to graphics mode
// (mode 13h)
void switch_graphics()
{
    cli();
    static const unsigned char hor_regs[] = {0x0, 0x1, 0x2, 0x3, 0x4,
                                             0x5, 0x13};

    static const unsigned char width_320[] = {0x5f, 0x4f, 0x50, 0x82, 0x54,
                                              0x80, 0x28};

    static const unsigned char ver_regs[] = {0x6, 0x7, 0x9, 0x10, 0x11,
                                             0x12, 0x15, 0x16};

    static const unsigned char height_200[] = {0xbf, 0x1f, 0x41, 0x9c, 0x8e,
                                               0x8f, 0x96, 0xb9};

    const unsigned char *w, *h;
    unsigned char val;
    int a;

    w = width_320;
    val = R_COM + R_W320;
    h = height_200;
    val |= R_H200;

    // here goes the actual modeswitch

    outb(0x3c2, val);
    outw(0x3d4, 0x0e11); // enable regs 0-7

    for (a = 0; a < size(hor_regs); ++a)
        outw(0x3d4, (unsigned short)((w[a] << 8) + hor_regs[a]));
    for (a = 0; a < size(ver_regs); ++a)
        outw(0x3d4, (unsigned short)((h[a] << 8) + ver_regs[a]));

    outw(0x3d4, 0x0008); // vert.panning = 0

    outw(0x3d4, 0x4014);
    outw(0x3d4, 0xa317);
    outw(0x3c4, 0x0e04);
    outw(0x3c4, 0x0101);

    outw(0x3c4, 0x0f02); // enable writing to all planes
    outw(0x3ce, 0x4005); // 256color mode
    outw(0x3ce, 0x0506); // graph mode & A000-AFFF

    inb(0x3da);
    outb(0x3c0, 0x30);
    outb(0x3c0, 0x41);
    outb(0x3c0, 0x33);
    outb(0x3c0, 0x00);
    for (a = 0; a < 16; a++) // Set EGA palette
    {
        outb(0x3c0, (unsigned char)a);
        outb(0x3c0, (unsigned char)a);
    }
    outb(0x3c0, 0x20); // enable video
    sti();
    graphicsMode = GRAPHICS;
    return;
}

// Sets a color in the palette for graphic modes.
// R, G and B must be between 0 and 63 (6-bit color)
void g_set_color(unsigned char color_number, unsigned char R, unsigned char G, unsigned char B)
{
    outb(0x03C6, 0xff);         // Mask all registers to allow updating
    outb(0x03C8, color_number); // Select color
    outb(0x03C9, R);            // Write values sequentially
    outb(0x03C9, G);
    outb(0x03C9, B);
}

void g_put_pixel_linear(int pixel_pos, unsigned char color)
{
    unsigned char *vga = (unsigned char *)GRAPHICS_VIDEO_ADRESS;
    vga[pixel_pos] = color;
}

void g_cls()
{
    for (int i = 0; i < 320 * 200; i++)
    {
        g_put_pixel_linear(i, 0);
    }
}