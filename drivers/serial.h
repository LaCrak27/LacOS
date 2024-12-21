#define COM1_PORT 0x3F8
int init_serial(unsigned short baud, unsigned short port);
void write_serial(char c, unsigned short port);
int serial_available();