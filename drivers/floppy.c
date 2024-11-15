#include "floppy.h"
// ASSUMPTIONS FOR NOW:
// - There's a 1.44mb floppy on the master drive, dunno about the slave
// - All floppies are 1.44mb

typedef struct CHSAddr CHSAddr;

CHSAddr lbatochs(unsigned long lba)
{
    CHSAddr res;
    res.cyl = lba / (2 * FLOPPY_144_SECTORS_PER_TRACK);
    res.head = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) / FLOPPY_144_SECTORS_PER_TRACK);
    res.sect = ((lba % (2 * FLOPPY_144_SECTORS_PER_TRACK)) % FLOPPY_144_SECTORS_PER_TRACK + 1);
    return res;
}