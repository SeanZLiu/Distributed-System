import os
import sys



fd = os.open( "/tmp/j254liu/mount/touchfile.txt", os.O_RDWR )


l = os.write( fd, "happy" )


os.close( fd )