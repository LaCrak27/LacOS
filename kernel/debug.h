#include "low_level.h"
// outputs a character to the debug console
#define BochsConsolePrintChar(c) outb(0xe9, c)
// stops simulation and breaks into the debug console
#define BochsBreak() outw(0x8A00, 0x8A00); outw(0x8A00, 0x08AE0);