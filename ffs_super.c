#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef DISK_DRIVER_H
#include "disk_driver.h"
extern struct disk_operations disk_ops;
#endif

#ifndef FFS_SUPER_H
#include "ffs_super.h"
#endif

#ifndef FFS_INODE_H
#include "ffs_inode.h"
#endif

struct IMsuper ffs_IMsb; // one in-memory SB only

/***
  create: To be called by format(), requires NOT mounted as it
          overwrites the in-memory SB structure.
	  DOES NOT UPDATE the disk superblock, that must be an explicit
	  call to the disk driver write
    parameters:
     @in: disk size (blocks); number of blocks to allocate for inodes
     @out: pointer to superblock structure				***/

static void super_create(unsigned int nblocks, unsigned int ninodeblocks)
{
    ffs_IMsb.sb.fsmagic = FS_MAGIC;

    ffs_IMsb.sb.nblocks = nblocks;
    ffs_IMsb.sb.nbmapblocksinodes = 1;
    ffs_IMsb.sb.ninodeblocks = ninodeblocks;
    ffs_IMsb.sb.ninodes = (ninodeblocks / 2) * LRG_INOS_PER_BLK + (ninodeblocks / 2) * SML_INOS_PER_BLK;
    ffs_IMsb.sb.startInArea = INODE_OFFSET;
    ffs_IMsb.sb.startRotdir = ffs_IMsb.sb.startInArea + ninodeblocks;
    ffs_IMsb.sb.startDtBmap = ffs_IMsb.sb.startRotdir + 1;
    ffs_IMsb.sb.nbmapblocksdata = 1;
    ffs_IMsb.sb.startDtArea = ffs_IMsb.sb.startDtBmap + 1;
    ffs_IMsb.sb.ndatablocks = nblocks - ffs_IMsb.sb.startDtArea;

    ffs_IMsb.sb.mounted = 0;
}

/***
  read: reads in a SB from disk, overwrites the in-mem SB structure.
	Requires disk open at the driver level.
    parameters:
     @out: pointer to superblock structure				***/

static int super_read()
{
    int ercode;
    union sb_block sb_u;

    ercode = disk_ops.read(SB_OFFSET, sb_u.data);
    if (ercode < 0)
        return ercode;

    memcpy(&ffs_IMsb.sb, &sb_u.sb, sizeof(struct super));

    return 0;
}

/***
  write: writes the in-mem SB structure to disk.
        Requires disk open at the driver level.
    parameters:
     @in: pointer to superblock structure				***/

static int super_write()
{
    int ercode;
    union sb_block sb_u;

    memset(sb_u.data, 0, DISK_BLOCK_SIZE); // clean...
    memcpy(&sb_u.sb, &ffs_IMsb.sb, sizeof(struct super));

    ercode = disk_ops.write(SB_OFFSET, sb_u.data);
    if (ercode < 0)
        return ercode;

    return 0;
}

/***
  get*: gets the relevant BFS info from the in-mem SB
    parameters: none
    returns: unsigned int value or address of region			***/

unsigned int super_getStartRotdir()
{
    return ffs_IMsb.sb.startRotdir;
}

unsigned int super_getStartDtBmap()
{
    return ffs_IMsb.sb.startDtBmap;
}

unsigned int super_getStartDtArea()
{
    return ffs_IMsb.sb.startDtArea;
}

unsigned int super_getNinodeblocks()
{
    return ffs_IMsb.sb.ninodeblocks;
}

unsigned int super_getTotalInodes()
{
    return ffs_IMsb.sb.ninodes;
}

unsigned int super_getNdatablocks()
{
    return ffs_IMsb.sb.ndatablocks;
}

/* Helper functions */

void super_debug()
{
    printf("Superblock:\n");
    printf("  fsmagic          = 0x%x\n", ffs_IMsb.sb.fsmagic);
    printf("  nblocks          = %u\n", ffs_IMsb.sb.nblocks);
    printf("  nbmapblocksinodes= %u\n", ffs_IMsb.sb.nbmapblocksinodes);
    printf("  ninodeblocks     = %u\n", ffs_IMsb.sb.ninodeblocks);
    printf("  ninodes          = %u\n", ffs_IMsb.sb.ninodes);
    printf("  nbmapblocksdata  = %u\n", ffs_IMsb.sb.nbmapblocksdata);
    printf("  ndatablocks      = %u\n", ffs_IMsb.sb.ndatablocks);
    printf("  startInArea      = %u\n", ffs_IMsb.sb.startInArea);
    printf("  startRotdir      = %u\n", ffs_IMsb.sb.startRotdir);
    printf("  startDtBmap      = %u\n", ffs_IMsb.sb.startDtBmap);
    printf("  startDtArea      = %u\n", ffs_IMsb.sb.startDtArea);
    printf("  mounted          = %s\n", (ffs_IMsb.sb.mounted) ? "yes" : "no");
    fflush(stdout);
}

void IMsuper_debug()
{
    printf("In-Memory Superblock:\n");
    printf("  dirty       = %s\n", (ffs_IMsb.dirty) ? "yes" : "no");
    super_debug();
}

struct super_operations super_ops = {
    .create = &super_create,
    .read = &super_read,
    .write = &super_write,
    .getStartRotdir = &super_getStartRotdir,
    .getStartDtBmap = &super_getStartDtBmap,
    .getStartDtArea = &super_getStartDtArea,
    .getNinodeblocks = &super_getNinodeblocks,
    .getTotalInodes = &super_getTotalInodes,
    .getNdatablocks = &super_getNdatablocks
};
