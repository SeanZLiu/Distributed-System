//
// Starter code for CS 454/654
// You SHOULD change this file
//

#include "rpc.h"
#include "debug.h"
INIT_LOG

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fuse.h>
#include <fcntl.h>

// # define PRINT_ERR 1

// Global state server_persist_dir.
char *server_persist_dir = nullptr;

// Important: the server needs to handle multiple concurrent client requests.
// You have to be carefuly in handling global variables, esp. for updating them.
// Hint: use locks before you update any global variable.

// We need to operate on the path relative to the the server_persist_dir.
// This function returns a path that appends the given short path to the
// server_persist_dir. The character array is allocated on the heap, therefore
// it should be freed after use.
// Tip: update this function to return a unique_ptr for automatic memory management.
char *get_full_path(char *short_path) {
    int short_path_len = strlen(short_path);
    int dir_len = strlen(server_persist_dir);
    int full_len = dir_len + short_path_len + 1;

    char *full_path = (char *)malloc(full_len);

    // First fill in the directory.
    strcpy(full_path, server_persist_dir);
    // Then append the path.
    strcat(full_path, short_path);
    DLOG("Full path: %s\n", full_path);

    return full_path;
}

// The server implementation of getattr.
int watdfs_getattr(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the stat structure, which should be filled in
    // by this function.
    struct stat *statbuf = (struct stat *)args[1];
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // TODO: Make the stat system call, which is the corresponding system call needed
    // to support getattr. You should use the statbuf as an argument to the stat system call.
    (void)statbuf;

    // Let sys_ret be the return code from the stat system call.
    int sys_ret = 0;
    sys_ret = stat(full_path, statbuf);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    //DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}

// The server implementation of mknod.
int watdfs_mknod(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the mode
    mode_t *mode = (mode_t *)args[1];
    dev_t *dev = (dev_t *)args[2];
    // The fourth argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[3];

    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;

    // Let sys_ret be the return code from the stat system call.
    int sys_ret = 0;
    DLOG("full_path: %s", full_path);
    DLOG("mode: %d", *mode);
    DLOG("dev: %d", *dev);

    sys_ret = mknod(full_path, *mode, *dev);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}

// The server implementation of open.
int watdfs_open(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the fi
    struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];
    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int fd = 0;
    fd = open(full_path, fi->flags);

    if (fd < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }else{
        fi->fh = fd;
        DLOG("the file descriptor is : %d", fd);

    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}

// The server implementation of close.
int watdfs_release(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the fi
    struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];
    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int sys_ret = 0;
    sys_ret = close(fi->fh);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }
    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}

// The server implementation of read.
int watdfs_read(int *argTypes, void **args) {
    // Get the arguments.
    char *short_path = (char *)args[0];
    DLOG("full path: ");

    char *full_path = get_full_path(short_path);
    DLOG(full_path);

    char *buf = (char *)args[1];
    long *buf_size = (long *)args[2];
    long *offset = (long *)args[3];
    struct fuse_file_info *fi = (struct fuse_file_info *)args[4];
    int *ret = (int *)args[5];


    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int bytes_read = 0;
    
    DLOG("file des: %d", fi->fh);


    bytes_read = pread(fi->fh, buf, *buf_size, *offset);
    DLOG("file descrip: %d, path: %s, after call", fi->fh, full_path);

    DLOG("Bytes read: %d", bytes_read);

    if (bytes_read <= 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }else{
        *ret = bytes_read;
    }

    DLOG("Returning code: %d", *ret);
    free(full_path);
    return 0;
}

// The server implementation of write.
int watdfs_write(int *argTypes, void **args) {
    // Get the arguments.
    char *short_path = (char *)args[0];
    DLOG("full path: ");

    char *full_path = get_full_path(short_path);
    DLOG(full_path);

    char *buf = (char *)args[1];
    long *buf_size = (long *)args[2];
    long *offset = (long *)args[3];
    struct fuse_file_info *fi = (struct fuse_file_info *)args[4];
    int *ret = (int *)args[5];


    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int bytes_write = 0;
    
    DLOG("file des: %d", fi->fh);

    bytes_write = pwrite(fi->fh, buf, *buf_size, *offset);
    DLOG("file descrip: %d, path: %s, after call", fi->fh, full_path);

    DLOG("Bytes write: %d", bytes_write);

    if (bytes_write < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }else{
        *ret = bytes_write;
    }

    DLOG("Returning code: %d", *ret);
    free(full_path);
    return 0;
}

// The server implementation of truncate.
int watdfs_truncate(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is newsize
    off_t *newsize = (off_t *)args[1];
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];
    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int sys_ret = 0;
    sys_ret = truncate(full_path, *newsize);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }
    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}


// The server implementation of fsync.
int watdfs_fsync(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the fi
    struct fuse_file_info *fi = (struct fuse_file_info *)args[1];
    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];
    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int sys_ret = 0;
    sys_ret = fsync(fi->fh);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}


// The server implementation of fsync.
int watdfs_utimensat(int *argTypes, void **args) {
    // Get the arguments.
    // The first argument is the path relative to the mountpoint.
    char *short_path = (char *)args[0];
    // The second argument is the fi
    struct timespec ts[2] = {((struct timespec *)args[1])[0], ((struct timespec *)args[1])[1]};

    // TODO: fi gai ts

    // The third argument is the return code, which should be set be 0 or -errno.
    int *ret = (int *)args[2];
    // Get the local file name, so we call our helper function which appends
    // the server_persist_dir to the given path.
    char *full_path = get_full_path(short_path);

    // Initially we set set the return code to be 0.
    *ret = 0;
    // Let sys_ret be the return code from the stat system call.
    int sys_ret = 0;
    sys_ret = utimensat(0, full_path, ts, 0);

    if (sys_ret < 0) {
        // If there is an error on the system call, then the return code should
        // be -errno.
        *ret = -errno;
    }

    // Clean up the full path, it was allocated on the heap.
    free(full_path);

    DLOG("Returning code: %d", *ret);
    // The RPC call succeeded, so return 0.
    return 0;
}

// The main function of the server.
int main(int argc, char *argv[]) {
    // argv[1] should contain the directory where you should store data on the
    // server. If it is not present it is an error, that we cannot recover from.
    if (argc != 2) {
        // In general you shouldn't print to stderr or stdout, but it may be
        // helpful here for debugging. Important: Make sure you turn off logging
        // prior to submission!
        // See watdfs_client.c for more details
        #ifdef PRINT_ERR
        std::cerr << "Usaage:" << argv[0] << " server_persist_dir" << std::endl;
        #endif
        return -1;
    }
    // Store the directory in a global variable.
    server_persist_dir = argv[1];

    // TODO: Initialize the rpc library by calling `rpcServerInit`.
    // Important: `rpcServerInit` prints the 'export SERVER_ADDRESS' and
    // 'export SERVER_PORT' lines. Make sure you *do not* print anything
    // to *stdout* before calling `rpcServerInit`.

    DLOG("Initializing server...");
    int ret = rpcServerInit();
    if (ret < 0){
        #ifdef PRINT_ERR
        std::cerr << "Server init failed:" << -errno << std::endl;
        #endif
        return -1;
    }
    // TODO: If there is an error with `rpcServerInit`, it maybe useful to have
    // debug-printing here, and then you should return.

    // TODO: Register your functions with the RPC library.
    // Note: The braces are used to limit the scope of `argTypes`, so that you can
    // reuse the variable for multiple registrations. Another way could be to
    // remove the braces and use `argTypes0`, `argTypes1`, etc.
    {
        // There are 3 args for the function (see watdfs_client.c for more
        // detail).
        int argTypes[4];
        // First is the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // The second argument is the statbuf.
        argTypes[1] =
            (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // The third argument is the retcode.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
        // Finally we fill in the null terminator.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"getattr", argTypes, watdfs_getattr);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "getattr register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    // mknod
    {
        int argTypes[5];
        argTypes[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // For arrays the argument is the array pointer, not a pointer to a pointer.
        // The second argument is the mode. This argument is an input
        // only argument, an integer
        argTypes[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);
        // The third argument is dev, an input only argument, which is
        // an long number.
        argTypes[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
        // The fourth argument is retcode, an output only argument, which is
        // an integer.
        argTypes[3] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally, the last position of the arg types is 0. There is no
        // corresponding arg.
        argTypes[4] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"mknod", argTypes, watdfs_mknod);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "mknod register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    // open
    {      
        int argTypes[4];
        // The first argument is the path, it is an input only argument, and a char
        // array. The length of the array is the length of the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // For arrays the argument is the array pointer, not a pointer to a pointer.

        // The second argument is the fi. This argument is an input, output and array
        // argument, a struct, use char array to store it.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u; 

        // The third argument is retcode, an output only argument, which is
        // an integer.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally, the last position of the arg types is 0. There is no
        // corresponding arg.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"open", argTypes, watdfs_open);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "open register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    //release
    {
        int argTypes[4];
        // The first argument is the path, it is an input only argument, and a char
        // array. The length of the array is the length of the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // For arrays the argument is the array pointer, not a pointer to a pointer.

        // The second argument is the fi. This argument is an input and array
        // argument, a struct, use char array to store it.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u; 

        // The third argument is retcode, an output only argument, which is
        // an integer.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally, the last position of the arg types is 0. There is no
        // corresponding arg.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"release", argTypes, watdfs_release);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "close register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    //read
    {
        // Fill in the arguments
        int argTypes[7];

        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        argTypes[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        argTypes[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        argTypes[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        argTypes[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u; 

        argTypes[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        argTypes[6] = 0;
        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"read", argTypes, watdfs_read);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "read register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    // write
    {
        // Fill in the arguments
        int argTypes[7];

        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;

        argTypes[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        argTypes[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

        argTypes[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u; 

        argTypes[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        argTypes[6] = 0;
        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"write", argTypes, watdfs_write);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "write register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }
        // truncate
    {      
        int argTypes[4];
        // The first argument is the path, it is an input only argument, and a char
        // array. The length of the array is the length of the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // For arrays the argument is the array pointer, not a pointer to a pointer.

        // The second argument is the fi. This argument is an input, output and array

        argTypes[1] = (1u << ARG_INPUT) | (ARG_LONG << 16u); 

        // The third argument is retcode, an output only argument, which is
        // an integer.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally, the last position of the arg types is 0. There is no
        // corresponding arg.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"truncate", argTypes, watdfs_truncate);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "truncate register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    // fsync
    {
        int argTypes[4];
        // The first argument is the path, it is an input only argument, and a char
        // array. The length of the array is the length of the path.
        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // For arrays the argument is the array pointer, not a pointer to a pointer.

        // The second argument is the fi. This argument is an input, and array
        // argument, a struct, use char array to store it.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u; 

        // The third argument is retcode, an output only argument, which is
        // an integer.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally, the last position of the arg types is 0. There is no
        // corresponding arg.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"fsync", argTypes, watdfs_fsync);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "open register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    // utimensat
    {
        int argTypes[4];

        argTypes[0] =
            (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u;
        // For arrays the argument is the array pointer, not a pointer to a pointer.

        // The second argument is the ts. This argument is an input, output and array
        // argument, a struct, use char array to store it.
        argTypes[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | 1u; // statbuf

        // The third argument is retcode, an output only argument, which is
        // an integer.
        argTypes[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);

        // Finally, the last position of the arg types is 0. There is no
        // corresponding arg.
        argTypes[3] = 0;

        // We need to register the function with the types and the name.
        ret = rpcRegister((char *)"utimensat", argTypes, watdfs_utimensat);
        if (ret < 0) {
            // It may be useful to have debug-printing here.
            #ifdef PRINT_ERR
            std::cerr << "utimensat register error: " << -errno << std::endl;
            #endif
            return ret;
        }
    }

    // TODO: Hand over control to the RPC library by calling `rpcExecute`.
    ret = rpcExecute();
    if(ret < 0){
        #ifdef PRINT_ERR
        std::cerr << "RPC Execute error: " << -errno << std::endl;
        #endif
        return -1;
    }
    // rpcExecute could fail so you may want to have debug-printing here, and
    // then you should return.

    return ret;
}
