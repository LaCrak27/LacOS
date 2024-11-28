#include "floppy.h"
#include "../kernel/low_level.h"
#include "../kernel/util.h"
#include "screen.h"
#include "../interrupts/idt.h"
#include "timer.h"
#include "../kernel/debug.h"

void floppy_irq_handler();
void wait_irq();
void floppy_write_cmd(int cmd);
unsigned char floppy_read_data();
void floppy_check_interrupt(int *st0, int *cyl);
void floppy_configure();
int floppy_calibrate();
int floppy_reset();
void floppy_motor(int onoff);
static int floppy_dma_init(floppy_dir dir);
int floppy_do_track(unsigned cyl, floppy_dir dir);
int floppy_read_track(unsigned cyl);
int floppy_write_track(unsigned cyl);
void floppy_motor_kill();

static char floppy_available = 0;
char isFloppyAvailable()
{
    return floppy_available;
}

static char *drive_types[8] = {
    "none",
    "360kB 5.25\"",
    "1.2MB 5.25\"",
    "720kB 3.5\"",

    "1.44MB 3.5\"",
    "2.88MB 3.5\"",
    "unknown type",
    "unknown type"};

char irqReceived = FALSE;
void floppy_irq_handler()
{
    irqReceived = TRUE;
}

void wait_irq()
{
    while (!irqReceived)
    {
    }
    return;
}

static unsigned char floppy_dmabuf[floppy_dmalen]__attribute__((aligned(0x8000)));

int initFloppy()
{
    println("Installing irq handler...");
    irqInstallHandler(6, &floppy_irq_handler);
    print("DMA Adress (for debugging): ");
    println(uitoh((unsigned)&floppy_dmabuf));
    outb(0x70, 0x10);
    unsigned drives = inb(0x71);
    print(" - Drive 0: ");
    set_fg(YELLOW);
    println(drive_types[drives >> 4]);
    set_fg(GRAY);
    print(" - Drive 1: ");
    set_fg(YELLOW);
    println(drive_types[drives & 0x0F]);
    set_fg(GRAY);
    if(drives >> 4 != 4)
    {
        println("Unsupported drive, aborting...");
        return 1;
    }
    println("Checking controller version...");
    floppy_write_cmd(VERSION);
    if(floppy_read_data() != 0x90)
    {
        println("Unsupported controller (how on earth did you even get a computer this old), stuff may fail, continue at your own risk");
    }
    println("Configuring...");
    floppy_write_cmd(SPECIFY); // Enable IRQs and stuff
    floppy_write_cmd(0xdf); /* steprate = 3ms, unload time = 240ms */
    floppy_write_cmd(0x02); /* load time = 16ms, no-DMA = 0 */
    floppy_configure(); // Implied seek off, fifo on, polling off, threshold 15ms, no precomp
    println("Locking config...");
    floppy_write_cmd(LOCK);
    println("Reseting and calibrating controller...");
    if(floppy_reset())
    {
        println("Error when calibrating controller, aborting...");
        return 2;
    }
    println("Floppy 0 configured correctly!");
    println("Validating floppy access...");
    floppy_read_track(0);
    if(floppy_dmabuf[0x1FF] != 0xAA)
    {
        println("Floppy validation failed. Dumping read boot sector in 1s:");
        sleep(1000);
        for(int i = 0; i < 0x20; i++)
        {
            print(uitoh(i*16));
            print("  ");
            for(int j = 0; j < 16; j++)
            {
                print(uctoh(floppy_dmabuf[i*16 + j]));
                printc(' ');
            }
            printc('\n');
            sleep(10);
        }
        return 3;
    }
    floppy_available = 1;
    return 0;
}

void floppy_write_cmd(int cmd)
{
    irqReceived = FALSE;
    int i; // do timeout, 10 seconds
    for (i = 0; i < 100; i++)
    {
        sleep(10);
        if (0x80 & inb(MAIN_STATUS_REGISTER))
        {
            outb(DATA_FIFO, cmd);
            return;
        }
    }
    // Handle the command failing
    except_intern("Floppy command failed (wut da hellll)");
}

unsigned char floppy_read_data()
{
    irqReceived = FALSE;
    int i; // do timeout, 10 seconds
    for (i = 0; i < 100; i++)
    {
        sleep(10);
        if (0x80 & inb(MAIN_STATUS_REGISTER))
        {
            return inb(DATA_FIFO);
        }
    }
    // Handle read failed
    except_intern("Floppy read failed!!!");
    return 0; // not reached
}

void floppy_check_interrupt(int *st0, int *cyl)
{
    floppy_write_cmd(SENSE_INTERRUPT);
    *st0 = floppy_read_data(STATUS_REGISTER_A);
    *cyl = floppy_read_data(STATUS_REGISTER_A);
}

void floppy_configure()
{
    floppy_write_cmd(CONFIGURE);
    floppy_write_cmd(0);
    floppy_write_cmd(0 << 6 | 0 << 5 | 1 << 4 | 8);
    floppy_write_cmd(0);
    return;
}

// Move to cylinder 0, which calibrates the drive..
int floppy_calibrate()
{
    int i, st0, cyl = -1; // set to bogus cylinder
    floppy_motor(floppy_motor_on);

    for (i = 0; i < 10; i++)
    {
        // Attempt to positions head to cylinder 0
        floppy_write_cmd(RECALIBRATE);
        floppy_write_cmd(0); // argument is drive, we only support 0

        floppy_check_interrupt(&st0, &cyl); // sense interruptttt

        if (st0 & 0xC0)
        {
            continue;
        }

        if (!cyl) // found cylinder 0 ?
        {
            floppy_motor(floppy_motor_off);
            return 0;
        }
    }
    floppy_motor(floppy_motor_off);
    except_intern("Floppy unable to calibrate");
    return -1;
}

int floppy_reset()
{
    irqReceived = FALSE;
    outb(DIGITAL_OUTPUT_REGISTER, 0x00); // disable controller
    outb(DIGITAL_OUTPUT_REGISTER, 0x0C); // enable controller
    wait_irq();                       
    int st0, cyl;                       // ignore these here..
    floppy_check_interrupt(&st0, &cyl); // Send sense interrupt
    // set transfer speed 500kb/s for 1.44 mb floppy
    outb(CONFIGURATION_CONTROL_REGISTER, 0x00);
    //  - 1st byte is: bits[7:4] = steprate, bits[3:0] = head unload time
    //  - 2nd byte is: bits[7:1] = head load time, bit[0] = no-DMA
    floppy_write_cmd(SPECIFY);
    floppy_write_cmd(0xdf); /* steprate = 3ms, unload time = 240ms */
    floppy_write_cmd(0x02); /* load time = 16ms, no-DMA = 0 */
    // it could fail...
    if (floppy_calibrate())
        return -1;
    return 0;
}

static volatile int floppy_motor_ticks = 0;
static volatile int floppy_motor_state = 0;

void floppy_motor(int onoff)
{
    if (onoff)
    {
        if (!floppy_motor_state)
        {
            // need to turn on
            outb(DIGITAL_OUTPUT_REGISTER, 0x1c);
            sleep(500); // Wait for motor to spin up
        }
        floppy_motor_state = floppy_motor_on;
    }
    else
    {
        floppy_motor_kill();
    }
}

void floppy_motor_kill()
{
    sleep(300);
    outb(DIGITAL_OUTPUT_REGISTER, 0x0c);
    floppy_motor_state = floppy_motor_off;
}

// Seek for a given cylinder, with a given head
int floppy_seek(unsigned cyli, int head)
{

    unsigned i, st0, cyl = -1; // set to bogus cylinder

    floppy_motor(floppy_motor_on);

    for (i = 0; i < 10; i++)
    {
        irqReceived = FALSE; // Get ready
        // Attempt to position to given cylinder
        // 1st byte bit[1:0] = drive, bit[2] = head
        // 2nd byte is cylinder number
        floppy_write_cmd(SEEK);
        floppy_write_cmd(head << 2);
        floppy_write_cmd(cyli);

        wait_irq();
        floppy_check_interrupt(&st0, &cyl); // Sense interrupt

        if (cyl == cyli)
        { // We reach our cylinder (otherwise try again 10 times)
            floppy_motor(floppy_motor_off);
            return 0;
        }
    }
    floppy_motor(floppy_motor_off);
    except_intern("Floppy seek error");
    return -1;
}

static int floppy_dma_init(floppy_dir dir)
{
    union
    {
        unsigned char b[4]; // 4 bytes
        unsigned long l;    // 1 long = 32-bit
    } a, c;                 // address and count

    a.l = (unsigned)&floppy_dmabuf;
    c.l = (unsigned)floppy_dmalen - 1; // -1 because of DMA counting

    // check that address is at most 24-bits (under 16MB)
    // check that count is at most 16-bits (DMA limit)
    // check that if we add count and address we don't get a carry
    if ((a.l >> 24) || (c.l >> 16) || (((a.l & 0xffff) + c.l) >> 16))
    {
        except_intern("Buffer not suitable");
    }

    unsigned char mode;
    switch (dir)
    {
    // 01:0:0:01:10 = single/inc/no-auto/to-mem/chan2
    case floppy_dir_read:
        mode = 0x46;
        break;
    // 01:0:0:10:10 = single/inc/no-auto/from-mem/chan2
    case floppy_dir_write:
        mode = 0x4a;
        break;
    default:
        return -1; // not reached, please "mode user uninitialized"
    }
    outb(0x0a, 0x06); // mask chan 2
    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x04, a.b[0]); //  - address low byte
    outb(0x04, a.b[1]); //  - address high byte
    outb(0x81, a.b[2]); // external page register
    outb(0x0c, 0xff);   // reset flip-flop
    outb(0x05, c.b[0]); //  - count low byte
    outb(0x05, c.b[1]); //  - count high byte
    outb(0x0b, mode); // set mode (see above)
    outb(0x0a, 0x02); // unmask chan 2
    return 0;
}

// This monster does full cylinder (both tracks) transfer to
// the specified direction (since the difference is small).
//
// It retries (a lot of times) on all errors except write-protection
// which is normally caused by mechanical switch on the disk.
//
int floppy_do_track(unsigned cyl, floppy_dir dir)
{
    irqReceived = FALSE;
    
    // transfer command, set below
    unsigned char cmd;

    // Read is MT:MF:SK:0:0:1:1:0, write MT:MF:0:0:1:0:1
    // where MT = multitrack, MF = MFM mode, SK = skip deleted
    static const int flags = 0xC0;
    switch(dir)
    {
        case floppy_dir_read:
            cmd = READ_DATA | flags;
            break;
        case floppy_dir_write:
            cmd = WRITE_DATA | flags;
            break;
        default: 
            return -1; // not reached, but pleases "cmd used uninitialized"
    }

    // seek both heads
    if(floppy_seek(cyl, 0)) return -1;
    if(floppy_seek(cyl, 1)) return -1;

    int i;
    int error = 0;
    for(i = 0; i < 20; i++) 
    {
        floppy_motor(floppy_motor_on);

        // init dma..
        floppy_dma_init(dir);

        sleep(100); // give some time (100ms) to settle after the seeks
        irqReceived = FALSE; // Get ready to get interrupt
        floppy_write_cmd(cmd);  // set above for current direction
        floppy_write_cmd(0);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        floppy_write_cmd(cyl);  // cylinder
        floppy_write_cmd(0);    // first head (should match with above)
        floppy_write_cmd(1);    // first sector, strangely counts from 1
        floppy_write_cmd(2);    // bytes/sector, 128*2^x (x=2 -> 512)
        floppy_write_cmd(18);   // number of tracks to operate on
        floppy_write_cmd(0x1b); // GAP3 length, 27 is default for 3.5"
        floppy_write_cmd(0xff); // data length (0xff if B/S != 0)
        
        wait_irq();

        // first read status information
        unsigned char st0, st1, st2, rcy, rhe, rse, bps;
        st0 = floppy_read_data();
        st1 = floppy_read_data();
        st2 = floppy_read_data();
        rcy = floppy_read_data(); // Read cyclinder
        rhe = floppy_read_data(); // Read head
        rse = floppy_read_data(); // Read sector
        // bytes per sector, should be what we programmed in
        bps = floppy_read_data();

        if(st0 & 0xC0) {
            error = 1;
        }
        if(st1 & 0x80) { // End of cylinder
            error = 2;
        }
        if(st0 & 0x08) { // Drive not ready
            error = 3;
        }
        if(st1 & 0x20) { // CRC error
            error = 4;
        }
        if(st1 & 0x10) { // Controller timeout
            error = 5;
        }
        if(st1 & 0x04) { // No data found
            error = 6;
        }
        if((st1|st2) & 0x01) { // No address mark found
            error = 7;
        }
        if(st2 & 0x40) { // Deleted adress mark
            error = 8;
        }
        if(st2 & 0x20) { // CRC error (data)
            error = 9;
        }
        if(st2 & 0x10) { // Wrong cyl
            error = 10;
        }
        if(st2 & 0x04) { // Unknown error
            error = 11;
        }
        if(st2 & 0x02) { // Bad cylinder
            error = 12;
        }
        if(bps != 0x2) { // Wrong sector lenght
            error = 13;
        }
        if(st1 & 0x02) { // Write protection
            error = 14;
            break;
        }

        if(!error) {
            floppy_motor(floppy_motor_off);
            return 0;
        }
    }
    floppy_motor(floppy_motor_off);
    return error;
}

int floppy_read_track(unsigned cyl) 
{
    return floppy_do_track(cyl, floppy_dir_read);
}

int floppy_write_track(unsigned cyl) 
{
    return floppy_do_track(cyl, floppy_dir_write);
}