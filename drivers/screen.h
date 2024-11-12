#define VIDEO_ADRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
//Color schemes
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define PURPLE 5
#define BROWN 6
#define GRAY 7
#define DARK_GRAY 8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_PURPLE 13
#define YELLOW 14
#define WHITE 15
//Screen I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5
void set_fg(unsigned char fgVal);
void set_bg(unsigned char bgVal);
void set_attr_byte(unsigned char attrByte);
void print_char(char character, int col, int row);
void erase_char();
int get_screen_offset(int colums, int rows);
int get_cursor();
void set_cursor(int offset);
void print_at(char* message, int col, int row);
void print(char* message);
void println(char* message);
void printc(char charToPrint);
void clear_screen();
int handle_scrolling(int cursor_offset);
void disable_cursor();
int get_cursor_row();
int get_cursor_col();