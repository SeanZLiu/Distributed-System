//
// Starter code for CS 454/654
// You SHOULD change this file
//

#include "watdfs_client.h"
#include "debug.h"
INIT_LOG

#include "rpc.h"
#include <iostream>

#define PRINT_ERR 1

// SETUP AND TEARDOWN
void *watdfs_cli_init(struct fuse_conn_info *conn, const char *path_to_cache,
                      time_t cache_interval, int *ret_code) {
    // TODO: set up the RPC library by calling `rpcClientInit`.
    int cliInitRes = rpcClientInit();
    // TODO: check the return code of the `rpcClientInit` call
    // `rpcClientInit` may fail, for example, if an incorrect port was exported.
    if(cliInitRes < 0){
        #ifdef PRINT_ERR
        std::cerr << "Failed to initialize RPC Client:" << -errno << std::endl;
        #endif
    }
    // It may be useful to print to stderr or stdout during debugging.
    // Important: Make sure you turn off logging prior to submission!
    // One useful technique is to use pre-processor flags like:
    // Tip: Try using a macro for the above to minimize the debugging code.

    // TODO Initialize any global state that you require for the assignment and return it.
    // The value that you return here will be passed as userdata in other functions.
    // In A1, you might not need it, so you can return `nullptr`.
    void *userdata = nullptr;

    // TODO: save `path_to_cache` and `cache_interval` (for A3).

    // TODO: set `ret_code` to 0 if everything above succeeded else some appropriate
    // non-zero value.
    if(cliInitRes >= 0){
        *ret_code = 0;
    }else{
        *ret_code = -errno;
    }
    // Return pointer to global state data.
    return userdata;
}

void watdfs_cli_destroy(void *userdata) {
    // TODO: clean up your userdata state.
    if(userdata != NULL)
        free(userdata);
    // TODO: tear down the RPC library by calling `rpcClientDestroy`.
    int destroyRes = rpcClientDestroy();
    if(destroyRes < 0){
        #ifdef PRINT_ERR
        std::cerr << "Failed to destroy RPC Client" << -errno << std::endl;
        #endif
    }
    return;
}

// GET FILE ATTRIBUTES
int watdfs_cli_getattr(void *userdata, const char *path, struct stat *statbuf) {
    // SET UP THE RPC CALL
    DLOG("watdfs_cli_getattr called for '%s'", path);
    
    // getattr has 3 arguments.
    int ARG_COUNT = 3;

    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];

    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the stat structure. This argument is an output
    // only argument, and we treat it as a char array. The length of the array
    // is the size of the stat structure, which we can determine with sizeof.
    arg_types[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) |
                   (uint) sizeof(struct stat); // statbuf
    args[1] = (void *)statbuf;

    // The third argument is the return code, an output only argument, which is
    // an integer.
    // TODO: fill in this argument type.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    // The return code is not an array, so we need to hand args[2] an int*.
    // The int* could be the address of an integer located on the stack, or use
    // a heap allocated integer, in which case it should be freed.
    // TODO: Fill in the argument
    int retCode;
    args[2] = (int *)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"getattr", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_getattr will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("getattr rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        fxn_ret = retCode;
        DLOG("getattr rpc call sucess with retcode '%d'", retCode);
        // TODO: set the function return value to the return code from the server.
    }

    if (fxn_ret < 0) {
        // If the return code of watdfs_cli_getattr is negative (an error), then 
        // we need to make sure that the stat structure is filled with 0s. Otherwise,
        // FUSE will be confused by the contradicting return values.
        memset(statbuf, 0, sizeof(struct stat));
    }

    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

// CREATE, OPEN AND CLOSE
int watdfs_cli_mknod(void *userdata, const char *path, mode_t mode, dev_t dev) {
    // Called to create a file.

    // SET UP THE RPC CALL
    DLOG("watdfs_cli_mknod called for '%s'", path);
    // getattr has 4 arguments.
    int ARG_COUNT = 4;
    // Allocate space for the output arguments.
    void **args = new void*[ARG_COUNT];
    // Allocate the space for arg types, and one extra space for the null
    // array element.
    int arg_types[ARG_COUNT + 1];
    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the mode. This argument is an input
    // only argument, an integer
    arg_types[1] = (1u << ARG_INPUT) | (ARG_INT << 16u);
    args[1] = (void *)(&mode);

    // The third argument is dev, an input only argument, which is
    // an long number.
    arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
    args[2] = (void*)(&dev);

    // The fourth argument is retcode, an output only argument, which is
    // an integer.
    arg_types[3] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[3] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[4] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"mknod", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("mknod rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("mknod rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

int watdfs_cli_open(void *userdata, const char *path,
                    struct fuse_file_info *fi) {
    // Called during open.
    // You should fill in fi->fh.

    DLOG("watdfs_cli_open called for '%s'", path);
    // getattr has 3 arguments.
    int ARG_COUNT = 3;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the fi. This argument is an input, output and array
    // argument, a struct, use char array to store it.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); // statbuf
    args[1] = (void *)fi;

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"open", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("open rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("open rpc call sucess with retcode '%d'", retCode);
        DLOG("fi->fh '%d'", fi->fh);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

int watdfs_cli_release(void *userdata, const char *path,
                       struct fuse_file_info *fi) {
    // Called during close, but possibly asynchronously.

    DLOG("watdfs_cli_release called for '%s'", path);
    // getattr has 3 arguments.
    int ARG_COUNT = 3;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the fi. This argument is an input, output and array
    // argument, a struct, use char array to store it.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); // statbuf
    args[1] = (void *)fi;

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"close", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("close rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("close rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

// READ AND WRITE DATA
int watdfs_cli_read(void *userdata, const char *path, char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    // Read size amount of data at offset of file into buf.

    // Remember that size may be greater then the maximum array size of the RPC
    // library.
    return -ENOSYS;
}
int watdfs_cli_write(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi) {
    // Write size amount of data at offset of file from buf.

    // Remember that size may be greater then the maximum array size of the RPC
    // library.
    return -ENOSYS;
}
int watdfs_cli_truncate(void *userdata, const char *path, off_t newsize) {
    // Change the file size to newsize.
    return -ENOSYS;
}

int watdfs_cli_fsync(void *userdata, const char *path,
                     struct fuse_file_info *fi) {
    // Force a flush of file data.
    return -ENOSYS;
}

// CHANGE METADATA
int watdfs_cli_utimensat(void *userdata, const char *path,
                       const struct timespec ts[2]) {
    // Change file access and modification times.
    return -ENOSYS;
}
