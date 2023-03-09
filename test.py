import os
import sys



# fd = os.open( "/tmp/j254liu/mount/newfile1.txt", os.O_RDWR)

fd = os.open( "/tmp/j254liu/mount/newfile7.txt", os.O_RDWR| os.O_CREAT)

l = os.write( fd, "happy".encode())
os.fsync(fd)


# os.close( fd )