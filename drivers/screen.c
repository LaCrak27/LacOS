#include "screen.h"
#include "../util/low_level.h"
#include "../util/util.h"
#include "timer.h"
#include "serial.h"

char graphicsMode = TEXT;

/* Source ASM from osdev.org
		;clear even/odd mode
		mov			dx, 03ceh
		mov			ax, 5
		out			dx, ax
		;map VGA memory to 0A0000h
		mov			ax, 0406h
		out			dx, ax
		;set bitplane 2
		mov			dx, 03c4h
		mov			ax, 0402h
		out			dx, ax
		;clear even/odd mode (the other way, don't ask why)
		mov			ax, 0604h
		out			dx, ax
		;copy charmap
		mov			esi, 0A0000h
		mov			ecx, 256
		;copy 16 bytes to bitmap
@@:		movsd
		movsd
		movsd
		movsd
		;skip another 16 bytes
		add			esi, 16
		loop			@b
		;restore VGA state to normal operation
		mov			ax, 0302h
		out			dx, ax
		mov			ax, 0204h
		out			dx, ax
		mov			dx, 03ceh
		mov			ax, 1005h
		out			dx, ax
		mov			ax, 0E06h
		out			dx, ax 
*/
void init_screen()
{

}

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
    if (graphicsMode == GRAPHICS)
    {
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
        write_serial('\r', COM1_PORT);
    }
    else
    {
        vidmem[offset] = character;
        vidmem[offset + 1] = attribute_byte;
    }
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

// Switches to graphics mode
// (mode 13h)
void switch_graphics()
{
    // REFERENCES:
    // https://ia801905.us.archive.org/30/items/bitsavers_ibmpccardseferenceManualMay92_1756350/IBM_VGA_XGA_Technical_Reference_Manual_May92.pdf
    // https://wiki.osdev.org/VGA_Hardware
    cli();

    // MISC Output register (0x03C2):
    // _______________________________
    // | 7   6   5   4  3  2  1    0 |
    // |VSP|HSP| 1 |---| CS |ERAM|IOS|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // VSP = 0, HSP = 1 -> 400 vertical lines (duplicated)
    // CS = 00 -> 25.175MHz clock for 320 horizontal lines
    // ERAM = 1 -> Enable RAM
    // IOS = 1 -> CRT controller addresses: 0x3Dx, IS1 reg: 0x3DA
    outb(0x03C2, 0b01100011);

    //// CRT Control registers (0x03D4):
    /// Horizontal:
    // Select register 0x11 (Vert. Retrace End)
    // ________________________
    // | 7  6   5   4  3 2 1 0|
    // |PR|S5R|EVI|CVI|  VRE  |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // Set to 0b00001110:
    // PR = 0 -> Registers 0-7 unlocked
    // S5R = 0 -> Basically always zero, weird 15.75kHz display things
    // EVI = 0 -> If set, generate an IRQ2 every time you finish drawing a frame
    // CVI = 0 -> Clears said interrupt
    // VRE = 0b1110 -> Is used when calculating when vertical retrace starts, gets set again later
    outw(0x03D4, 0x0E11);

    // Select register 0x00 (Horizontal Total)
    // ___________________
    // | 7 6 5 4 3 2 1 0 |
    // |    Hor. Total   |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // !!! Value is the stated in the register plus 5
    // HT = 95 -> 100 Characters in the horizontal scan interval (as required for mode 0x13)
    outw(0x03D4, 0x5F00);
    // Select register 0x01 (Hor. Display-Enable End)
    // ___________________
    // | 7 6 5 4 3 2 1 0 |
    // | Hor.Disp.En.End |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // !!! Value is the stated in the register plus 1
    // HDEE = 79 -> 80 Character positions per horizontal line (as required for mode 0x13)
    outw(0x03D4, 0x4F01);
    // Select register 0x02 (Hor. Blanking Start)
    // ___________________
    // | 7 6 5 4 3 2 1 0 |
    // | Hor. Blank. St. |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // HB = 80 -> Character count where the horizontal blanking starts is 80 (as required for mode 0x13)
    outw(0x03D4, 0x5002);
    // Select register 0x03 (Hor. Blanking End)
    // _____________________
    // | 7  6 5  4 3 2 1 0 |
    // | 1 |DES|     EB    |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // 1 = 1 -> Always 1
    // DES = 00 -> No character clock skew (ignored on type 2)
    // EB = 0b00010 -> End blanking field, 5 lowest bytes of value (highest is in reg 0x05), says when blanking ends
    outw(0x03D4, 0x8203);
    // Select register 0x04 (Start Horizontal Retrace Pulse)
    // ___________________
    // | 7 6 5 4 3 2 1 0 |
    // | St.Hor.Ret.Pul. |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // STRP = 84 -> Used to center screen, specifies char pos when horizontal retrace goes active (as required for mode 0x13)
    outw(0x03D4, 0x5404);
    // Select register 0x05 (End Horizontal Retrace)
    // __________________________
    // |  7   6   5   4 3 2 1 0 |
    // | EB5|  HRD  |    EHR    |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // EB5 = 1 -> MSB of "End blanking field" (lowest in reg 0x04)
    // HRD = 0 -> No skew
    // EHR = 0 -> Set's horizontal retrace to inactive at position 0 (as required for mode 0x13)
    outw(0x03D4, 0x8005);
    // Select register 0x13 (Offset register / Logical width)
    // ___________________
    // | 7 6 5 4 3 2 1 0 |
    // |      Offset     |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // Offset = 40 -> Logical line width of screen (as required for mode 13h)
    outw(0x03D4, 0x2813);

    /// Vertical:
    // Select register 0x06 (Vertical Total)
    // ___________________
    // | 7 6 5 4 3 2 1 0 |
    // |   Vert. Total   |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // VT = 191 -> 8 low bits of number of horizontal lines on display. Upper 2 are in overflow register
    outw(0x03D4, 0xBF06);
    // Select register 0x07 (Overflow register)
    // _______________________________________
    // |  7    6    5   4   3    2    1    0 |
    // | VRS9|VDE9|VT9|LC8|VBS8|VRS8|VDE8|VT8|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // Each bit is the corresponding one from those values.
    outw(0x03D4, 0x1F07);
    // Select register 0x09 (Max. Scanline)
    // __________________________
    // |  7   6   5   4 3 2 1 0 |
    // | DSC|LC9|VBS9|    MSL   |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // !!! Value is the stated in the register plus 1
    // MSL = 1 -> 2 scanlines per character row (which is why vertical lines are duplicated in MISC)
    outw(0x03D4, 0x4109);
    // Vert. Retrace start (same as hor. roughly)
    outw(0x03D4, 0x9C10);
    // Vert. Retrace end (same as hor. roughly)
    outw(0x03D4, 0x8E11);
    // Vert. Display enable end (same as hor. roughly)
    outw(0x03D4, 0x8F12);
    // Start vert. blanking (same as hor. roughly)
    outw(0x03D4, 0x9615);
    // End vert. blanking (same as hor. roughly)
    outw(0x03D4, 0xB916);
    // Select register 0x08 (Preset row scan)
    // ________________________
    // |  7  6  5   4 3 2 1 0 |
    // | ---| BP |     SRS    |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // !!! Value is the stated in the register plus 1
    // BP = 00 -> Are set that way in all BIOS modes
    // SRS = 0000 -> Sets starting row after a vertical retrace
    outw(0x03D4, 0x0008);

    /// Misc CRT regs:
    // Select register 0x14 (Underline location)
    // _________________________
    // |  7  6   5   4 3 2 1 0 |
    // | ---|DW|CB4|    SUL    |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // !!! Value is the stated in the register plus 1
    // DW = 1 -> Use DWord for addressing
    // CB4 = 0 -> Enable to divide memory counter freq. by 4
    // SUL = 0 -> Horizontal scan line where underline occurs is 1
    outw(0x03D4, 0x4014);
    // Select register 0x17 (CRT mode control)
    // __________________________________
    // |  7  6   5   4   3   2   1   0  |
    // | RST|WB|ADW|---|CB2|HRS|SRC|CMS0|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // RST = 1 -> Pull to zero to disable display
    // WB = 0 -> Adressing shenanigans, look at page 92 of the "IBM VGA XGA Technical Reference Manual" from 1992
    // ADW = 1 -> Unlocks memory
    // CB2 = 0 -> Would divide memory counter freq. by 2 if set
    // HRS = 0 -> Would double resolution if set
    // SRC = 1 -> Sets source of bit 14 of output multiplexer (?)
    // CMS0 = 1 -> Sets source of bit 13 of output multiplexer (?)
    outw(0x03D4, 0xA317);
    //// Sequencer registers:
    // Select register 0x04 (Memory mode)
    // ________________________________
    // |  7   6   5   4   3  2  1   0 |
    // | ---|---|---|---|CH4|OE|EM|---|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // CH4 = 1 -> Causes the 2 low order bits of adress to select map, making mode appear linear
    // OE = 1 -> Don't use Odd/Even, access data in maps sequentially
    // EM = 1 -> Extended memory (use 256K)
    outw(0x03C4, 0x0E04);
    // Select register 0x01 (Clocking mode)
    // _____________________________
    // |  7   6  5   4  3  2  1  0 |
    // | ---|---|SO|SH4|DC|SL|1|D89|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // SO = 0 -> Would turn the screen off (hence the name)
    // SH4 = 0 -> Should always be cleared
    // DC = 0 -> Would divide dot clock by 2
    // SL = 0 -> Should always be cleared
    // D89 = 1 -> Should be set unless using modes 0,1,2,3 and 7
    outw(0x03C4, 0x0301);
    // Select register 0x02 (Map mask)
    // __________________________________
    // |  7   6   5   4   3   2   1   0 |
    // | ---|---|---|---|M3E|M2E|M1E|M0E|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // MXE = 1 -> Enable map X (enable all)
    outw(0x03C4, 0x0f02);
    //// Graphics controller registers:
    // Select register 0x05 (Graphics mode)
    // _______________________________
    // |  7   6   5  4  3   2   1  0 |
    // | ---|C256|SR|OE|RM|---|  WM  |
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // C256 = 1 -> Enable 256-color mode
    // SR = 0 -> Only used in modes 4 and 5
    // OE = 0 -> Same as the other OE
    // RM = 0 -> Do NOT use Color Compare registers
    // WM = 0 ->   "Each memory map is written with the system data rotated by the count
    //              in the Data Rotate register. If the set/reset function is enabled for a
    //              specific map, that map receives the 8-bit value contained in the
    //              Set/Reset register" (page 103 of tech manual)
    outw(0x03CE, 0x4005);
    // Select register 0x06 (MISC Register)
    // _______________________________
    // |  7   6   5   4   3  2  1  0 |
    // | ---|---|---|---|  MM  |OE|GM|
    // ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
    // MM = 01 -> A000 -> AFFF
    // OE = 0 -> Do i need to keep explaining it
    // GM = 1 -> We are in a graphics mode :)
    outw(0x03CE, 0x0506); // graph mode & A000-AFFF

    //// Attribute controller registers:
    inb(0x03DA);        // Clear flip flop from Adress Register (flip flop says which register you access)
    outb(0x03C0, 0x30); // IPAS = 1 -> Set normal operation, select register 0x10           # Address
    outb(0x03C0, 0x41); // PW = 1 -> 256-color mode, G = 1 -> Graphics mode selected        # 0x10
    outb(0x03C0, 0x33); // IPAS = 1, select register 0x13                                   # Address
    outb(0x03C0, 0x00); // Do not shift the image to the left (shift it by 0)               # 0x13

    //// Set palette (same as text mode, 16 colors):
    for (int i = 0; i < 15; i++)
    {
        unsigned char base = 16*i;
        g_set_color(base + BLACK, 0, 0, 0);
        g_set_color(base + BLUE, 0, 0, 168);
        g_set_color(base + GREEN, 0, 168, 0);
        g_set_color(base + CYAN, 0, 168, 168);
        g_set_color(base + RED, 168, 0, 0);
        g_set_color(base + PURPLE, 168, 0, 168);
        g_set_color(base + BROWN, 168, 84, 0);
        g_set_color(base + GRAY, 168, 168, 168);
        g_set_color(base + DARK_GRAY, 84, 84, 84);
        g_set_color(base + LIGHT_BLUE, 84, 84, 252);
        g_set_color(base + LIGHT_GREEN, 84, 252, 84);
        g_set_color(base + LIGHT_CYAN, 84, 252, 252);
        g_set_color(base + LIGHT_RED, 252, 84, 84);
        g_set_color(base + LIGHT_PURPLE, 252, 84, 252);
        g_set_color(base + YELLOW, 252, 168, 84);
        g_set_color(base + WHITE, 252, 252, 252);
    }

    outb(0x03C0, 0x20); // Set IPAS = 1 again, making sure output color is indeed enabled   # Address
    graphicsMode = GRAPHICS;
    sti();
    g_cls();
    return;
}

// Sets a color in the palette for graphic modes.
// R, G and B will be compressed to 0-63 from 0-255
void g_set_color(unsigned char color_number, unsigned char R, unsigned char G, unsigned char B)
{
    outb(0x03C6, 0xff);         // Mask all registers to allow updating
    outb(0x03C8, color_number); // Select color
    outb(0x03C9, R / 4);        // Write values sequentially
    outb(0x03C9, G / 4);
    outb(0x03C9, B / 4);
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

// Switches to text mode
// (mode 03h)
void switch_text()
{
    cli();
    // All registers are already explained in switch_graphics(), so I will not go into detail here
    outb(0x03C2, 0x67);
    outw(0x03D4, 0x8E11);

    outw(0x03D4, 0x5F00);
    outw(0x03D4, 0x4F01);
    outw(0x03D4, 0x5002);
    outw(0x03D4, 0x8203);
    outw(0x03D4, 0x5504);
    outw(0x03D4, 0x8105);
    outw(0x03D4, 0x2813);

    outw(0x03D4, 0xBF06);
    outw(0x03D4, 0x1F07);
    outw(0x03D4, 0x4F09);
    outw(0x03D4, 0x9C10);
    outw(0x03D4, 0x8E11);
    outw(0x03D4, 0x8F12);
    outw(0x03D4, 0x9615);
    outw(0x03D4, 0xB916);
    outw(0x03D4, 0x0008);

    outw(0x03D4, 0x1F14);
    outw(0x03D4, 0xA317);
    outw(0x03C4, 0x0704);
    outw(0x03C4, 0x0001);
    outw(0x03C4, 0x0f02);
    outw(0x03C4, 0x0003);

    outw(0x03CE, 0x1005);
    outw(0x03CE, 0x0E06);

    inb(0x03DA);        
    outb(0x03C0, 0x30); 
    outb(0x03C0, 0x0C); 
    outb(0x03C0, 0x33); 
    outb(0x03C0, 0x08); 

    outb(0x03C0, 0x20); 
    graphicsMode = TEXT;
    sti();
    clear_screen();
    return;
}