#define FLOPPY_144_SECTORS_PER_TRACK 18
// Floppy registers! (IO ports)
#define STATUS_REGISTER_A 0x3F0 // read-only
#define STATUS_REGISTER_B 0x3F1 // read-only
#define DIGITAL_OUTPUT_REGISTER 0x3F2
#define TAPE_DRIVE_REGISTER 0x3F3
#define MAIN_STATUS_REGISTER 0x3F4     // read-only
#define DATARATE_SELECT_REGISTER 0x3F4 // write-only
#define DATA_FIFO 0x3F5
#define DIGITAL_INPUT_REGISTER 0x3F7         // read-only
#define CONFIGURATION_CONTROL_REGISTER 0x3F7 // write-only
// Floppy commands! (sent through DATA_FIFO)
#define READ_TRACK 2 // generates IRQ6
#define SPECIFY 3    // * set drive parameters
#define SENSE_DRIVE_STATUS 4
#define WRITE_DATA 5      // * write to the disk
#define READ_DATA 6       // * read from the disk
#define RECALIBRATE 7     // * seek to cylinder 0
#define SENSE_INTERRUPT 8 // * generates IRQ6, get status of last command
#define WRITE_DELETED_DATA 9
#define READ_ID 10 // generates IRQ6
#define READ_DELETED_DATA 12
#define FORMAT_TRACK 13 // *
#define DUMPREG 14
#define SEEK 15    // * seek both heads to cylinder X
#define VERSION 16 // * used during initialization, once
#define SCAN_EQUAL 17
#define PERPENDICULAR_MODE 18 // * used during initialization, once, maybe
#define CONFIGURE 19          // * set controller parameters
#define LOCK 0x94               // * protect controller params from a reset
#define VERIFY 22
#define SCAN_LOW_OR_EQUAL 25
#define SCAN_HIGH_OR_EQUAL 29
// Misc
enum
{
    floppy_motor_off = 0,
    floppy_motor_on
};
typedef enum
{
    floppy_dir_read = 1,
    floppy_dir_write = 2
} floppy_dir;

#define floppy_dmalen 0x4800

int initFloppy();
char isFloppyAvailable();