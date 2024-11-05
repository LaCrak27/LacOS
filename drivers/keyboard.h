typedef struct InterruptRegisters InterruptRegisters;
char readKey();
void initKeyboard();
void keyboardHandler(InterruptRegisters *regs);