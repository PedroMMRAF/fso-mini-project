#ifndef BFS_FILE_H
#include "bfs_file.h"
#endif

/***
  create_C: creates a contiguous file
    parameters:
      @in: filename, max blocks, type (C or D)
     @out: return OK
    errors:
     from bytemap, inode, dir operations
     errors from disk driver						***/

static int file_create_C( char *name, unsigned int nblocks, char type) {

  return 0;
 }


/***
  create_I: creates an indexed file
    parameters:
      @in: filename
     @out: return OK
    errors:
     from bytemap, inode, dir operations
     errors from disk driver						***/

static int file_create_I(char *name) {

  return 0;
}


struct file_operations file_ops= {
        .create_C= file_create_C,
        .create_I= file_create_I
};
