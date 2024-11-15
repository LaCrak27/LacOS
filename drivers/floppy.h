#define FLOPPY_144_SECTORS_PER_TRACK 18
struct CHSAddr {
    unsigned long cyl;
    unsigned long head;
    unsigned long sect;
};