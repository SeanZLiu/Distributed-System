import os
import sys



# fd = os.open( "/tmp/j254liu/mount/newfile1.txt", os.O_RDWR)

fd = os.open( "/tmp/j254liu/mount/newfile12.txt", os.O_WRONLY| os.O_CREAT)

l = os.write( fd, "12345".encode())
l = os.write( fd, "3".encode())
l = os.write( fd, "4".encode())
# l = os.write( fd, "5".encode())
# l = os.write( fd, "2".encode())
# l = os.write( fd, "3".encode())
# l = os.write( fd, "2".encode())
# l = os.write( fd, "1".encode())

os.fsync(fd)



os.close( fd )
fd = os.open( "/tmp/j254liu/mount/newfile12.txt", os.O_WRONLY)
