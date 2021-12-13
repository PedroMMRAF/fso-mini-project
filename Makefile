CFLAGS = -Wall

tstFFS: tstFFS.o disk_driver.o ffs_bytemap.o ffs_super.o 
	gcc -o tstFFS tstFFS.o disk_driver.o ffs_bytemap.o ffs_super.o

tstFFS.o: tstFFS.c
	gcc $(CFLAGS) -c tstFFS.c

disk_driver.o: disk_driver.c disk_driver.h bfs_errno.h
	gcc $(CFLAGS) -c disk_driver.c

ffs_bytemap.o: ffs_bytemap.c ffs_bytemap.h bfs_errno.h
	gcc $(CFLAGS) -c ffs_bytemap.c

ffs_super.o: ffs_super.c ffs_super.h disk_driver.h ffs_inode.h bfs_errno.h
	gcc $(CFLAGS) -c ffs_super.c

clean:
	rm tstFFS tstFFS.o disk_driver.o ffs_bytemap.o ffs_super.o

