#define VIDEO_ADRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
//Color scheme, can be changed ig
#define WHITE_ON_BLACK 0x0f
//Screen I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5
void print_char(char character, int col, int row, char attribute_byte);
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