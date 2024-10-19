void initKeyboard();
void keyboardHandler(struct InterruptRegisters *regs);
void keyboardInstallHandler(int num, void (*handler)(char scanCode, char press));
void keyboardUninstallHandler(int num);