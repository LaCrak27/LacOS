#include "low_level.h"
// outputs a character to the debug console
#define BochsConsolePrintChar(c) pByteOut(0xe9, c)
// stops simulation and breaks into the debug console
#define BochsBreak() pWordOut(0x8A00, 0x8A00); pWordOut(0x8A00, 0x08AE0);