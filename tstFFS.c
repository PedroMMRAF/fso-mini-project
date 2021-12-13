#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef DISK_DRIVER_H
#include "disk_driver.h"
extern struct disk_operations disk_ops;
#endif

#ifndef FFS_SUPER_H
#include "ffs_super.h"
extern struct super_operations super_ops;
#endif

#ifndef FFS_INODE_H
#include "ffs_inode.h"
extern struct inode_operations inode_ops;
#endif

#ifndef FFS_BYTEMAP_H
#include "ffs_bytemap.h"
extern struct bytemap_operations bmap_ops;
#endif

#define LINESIZE 64
#define ARGVMAX 32

int makeargv(char *s, char *argv[ARGVMAX + 1])
{
  // in: s points a text string with words
  // out: argv[] points to all words in the string s (*s is modified!)
  // pre: argv is predefined as char *argv[ARGVMAX+1]
  // return: number of words pointed to by the elements in argv (or -1 in case of error)

  int ntokens;

  if (s == NULL || argv == NULL || ARGVMAX == 0)
    return -1;

  ntokens = 0;
  argv[ntokens] = strtok(s, " \t\n");
  while ((argv[ntokens] != NULL) && (ntokens < ARGVMAX))
  {
    ntokens++;
    argv[ntokens] = strtok(NULL, " \t\n");
  }
  argv[ntokens] = NULL; // it must terminate with NULL
  return ntokens;
}

// deals with allocate first free entry, specified as -1
// must return the entry number, to be used in inode allocation

// Create a disk
void run_D(int argc, char *args[])
{
  int ercode;
  int nblocks = 0;

  if (argc == 3)
    nblocks = atoi(args[2]);

  ercode = disk_ops.open(args[1], nblocks);
  
  if (ercode < 0)
  {
    if (argc == 3)
      printf("%s %s %s\tERROR\n", args[0], args[1], args[2]);
    else
      printf("%s %s\t\tERROR\n", args[0], args[1]);
  }
  else
  {
    if (argc == 3)
      printf("%s %s %s\tOK\n", args[0], args[1], args[2]);
    else
      printf("%s %s\t\tOK\n", args[0], args[1]);
  }
}

// Close disk
void run_L(char *args[])
{
  int ercode;

  ercode = disk_ops.close();
  if (ercode < 0)
    printf("%s\t\tERROR\n", args[0]);
  else
    printf("%s\t\tOK\n", args[0]);
}

// this will be the format function
void format(char *diskname, unsigned int ninodeblocks)
{
  int ercode;

  if (ninodeblocks % 2)
  {
    printf("format(): ninodeblocks is odd!\n");
    return;
  }

  ercode = disk_ops.open(diskname, 0);
  if (ercode < 0)
  {
    printf("format():disk_ops.open %s\n", strerror(-ercode));
    return;
  }

  ercode = disk_ops.stat(diskname);
  if (ercode < 0)
  {
    printf("format():disk_ops.stat %s\n", strerror(-ercode));
    return;
  }

  unsigned int nblocks = ercode;
  super_ops.create(nblocks, ninodeblocks);

  ercode = super_ops.write();
  if (ercode < 0)
  {
    printf("format():super_ops.write %s\n", strerror(-ercode));
    return;
  }
}

// Format a disk
// args[0]= F, args[1]= disk name, args[2]= even ninodeblocks
void run_F(int argc, char *args[])
{
  unsigned int ninodeblocks;

  if (argc == 3)
    ninodeblocks = atoi(args[2]);
  else
  {
    printf("%s %s\t\tERROR: missing nidodeblocks\n", args[0], args[1]);
    return;
  }

  printf("%s %s\t%s\tFormatting...\n", args[0], args[1], args[2]);
  format(args[1], ninodeblocks);
}

// Mount: Read in the superblock and optionally debug. Should NOT be mounted
// args[0]= M, args[1]= disk name, args[2]= 1 debug, 0 no
void run_M(int argc, char *args[])
{
  int ercode;

  ercode = disk_ops.open(args[1], 0);
  if (ercode < 0)
  {
    printf("open: %s %s %s\tERROR\n", args[0], args[1], args[2]);
    return;
  }
  ercode = super_ops.read();
  if (ercode < 0)
  {
    printf("open: %s %s %s\tERROR\n", args[0], args[1], args[2]);
    return;
  }

  if (atoi(args[2]))
    super_debug();
  printf("%s %s %s\tOK\n", args[0], args[1], args[2]);
}

// Initialize BITMAP TABLE indexes
void run_i(int argc, char *args[])
{
  bmap_ops.init();
}

// Bytemap get free test
// args[0]= B, args[1]= 0 SML_INODE_BMAP 1 LRG_INODE_BMAP, 2 DATA_BMAP
void run_B(int argc, char *args[])
{

  //bmap_ops.init();
  printf("Bmap get free: %d\n", bmap_ops.getfree(atoi(args[1]), atoi(args[2])));
}

// Bytemap set test
// args[0]= S, args[1]= 0 SML_INODE_BMAP 1 LRG_INODE_BMAP, 2 DATA_BMAP
// args[2]= entry args[3]=value2set
void run_S(int argc, char *args[])
{

  //bmap_ops.init();
  printf("Bmap set: bmap %d entry %d value %d ercode %d\n",
         atoi(args[1]), atoi(args[2]), atoi(args[3]),
         bmap_ops.set(atoi(args[1]), atoi(args[2]), atoi(args[3])));
}

// Bytemap print table
// args[0]= T, args[1]= 0 SML_INODE_BMAP 1 LRG_INODE_BMAP, 2 DATA_BMAP
void run_T(int argc, char *args[])
{
  int ercode;

  //bmap_ops.init();
  ercode = bytemap_print_table(atoi(args[1]));
  if (ercode < 0)
    printf("ERROR bytemap_print_table\n");
}

void runcommand(int argc, char *args[])
{

  switch (args[0][0])
  {
  case 'D': // Disk open
    run_D(argc, args);
    break;
  case 'L': // L is disk cLose
    run_L(args);
    break;
  case 'F': // Format disk
    run_F(argc, args);
    break;
  case 'M': // Mount and optionally debug, must be unmounted
    run_M(argc, args);
    break;
  case 'i': // Initialize bytemap index tables
    run_i(argc, args);
    break;
  case 'B': // Bytemap getfree
    run_B(argc, args);
    break;
  case 'S': // Bytemap set
    run_S(argc, args);
    break;
  case 'T': // Print Bytemap tables
    run_T(argc, args);
    break;
    /***
    case 'U': // Umount
      ffs_ops.umount(&ffs_IMsb);
      break;
***/

  case '#': // Comment line
    break;
  default:
    printf("WRONG SPEC FILE?\n");
    break;
  }
}

void interp()
{
  char line[LINESIZE];
  char *av[ARGVMAX];
  int argc;

  while (fgets(line, LINESIZE, stdin) != NULL)
  {
    if ((argc = makeargv(line, av)) > 0)
      runcommand(argc, av);
  }
}

int main(int argc, char *argv[])
{

  interp();

  return 0;
}
