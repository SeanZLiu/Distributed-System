//
// Starter code for CS 454/654
// You SHOULD change this file
//

#include "watdfs_client.h"
#include "debug.h"
INIT_LOG

#include "rpc.h"
#include <iostream>
#include "rw_lock.h"
#include <map>
#include <unistd.h>
#include <ctime>
#define PRINT_ERR 1


typedef struct client_open_meta{
    // bool opening; // necessary? 
    time_t tc; // last validate time
    struct fuse_file_info client_file_inf; // contain client fd and flags(O_CREATE, O_RDONLY, O_RDWR, etc.)
    struct fuse_file_info server_file_inf; // contain server fd and flags(O_CREATE, O_RDONLY, O_RDWR, etc.)
}meta_d;

typedef struct client_data{
    time_t cache_intvl;
    char *cache_path;
    std::map <std::string, meta_d> *open_map;  // notice that it is a pointer to mapping boject
    // maps the filenme(without directory path) to its metadata defined below
}cli_data;


int utils_getattr(void *userdata, const char *path, struct stat *statbuf) {
    // SET UP THE RPC CALL
    
    int ARG_COUNT = 3;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];

    // The path has string length (strlen) + 1 (for the null character).
    int pathlen = strlen(path) + 1;

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

// check the modification time of a file on server, compare it with client modification time
// result get from retcode, 1 means equal, 0 means nequal, negative means errno
int utils_check_server_time(void *userdata, const char *path,
                       const struct timespec *t_client){

    DLOG("utils_check_time called for '%s'", path);
    // getattr has 3 arguments.
    int ARG_COUNT = 3;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    DLOG("client last access time sec'%ld'", t_client->tv_sec);
    DLOG("client last access time nsec'%ld'", t_client->tv_nsec);

    // The first argument is the path
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is t_client
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) (sizeof(struct timespec)); // statbuf
    args[1] = (void *)t_client;

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"checktime", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("fsync rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("fsync rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;


    // Finally return the value we got from the server.
    return fxn_ret;
}


int utils_mknod(void *userdata, const char *path, mode_t mode, dev_t dev) {
    // Called to create a file.

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


int utils_open(void *userdata, const char *path,
                    struct fuse_file_info *fi) {
    // Called during open.
    // You should fill in fi->fh.

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
    
    DLOG("open rpc call path '%s'", path);

    // The second argument is the fi. This argument is an input, output and array
    // argument, a struct, use char array to store it.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); // statbuf
    args[1] = (void *)fi;
    
    DLOG("open rpc call flag '%d'", fi->flags);
    DLOG("open rpc call fd '%d'", fi->fh);

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

int utils_release(void *userdata, const char *path,
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
    int rpc_ret = rpcCall((char *)"release", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("release rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("release rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

int utils_read(void *userdata, const char *path, char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    // Read size amount of data at offset of file into buf.
    // Remember that size may be greater then the maximum array size of the RPC
    // library.

    int ARG_COUNT = 6;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    long read_size = 0;
    long buf_size = MAX_ARRAY_LEN;

    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;

    arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    // TODO : offset need to change each time you read
    arg_types[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    arg_types[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); 
    
    arg_types[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[5] = (void*)(&retCode);

    args[0] = (void *)path;
    args[4] = (void *)fi;
    arg_types[6] = 0;

    // long current_offset = offset;
    // char *tempBufCache = buf;

    while (read_size < size){
        DLOG("read_size this loop '%d'", read_size);

        // char *tep_buf = (char *)malloc(buf_size);
		// 			// tempBufCache is used to write bufCache from the server to buf, a pointer for loop
		// memset(tep_buf, 0, sizeof(buf_size));

        // DLOG("client tep_buf add '%u'", tep_buf);
        if(size < MAX_ARRAY_LEN){
            buf_size = size;
            DLOG("size: %d,  is smaller than buf_size",  size);
        }
        if(size - read_size < MAX_ARRAY_LEN){
            buf_size = size - read_size;
        }

        // Fill in the arguments
        arg_types[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) buf_size;
        args[1] = (void *)buf;
        args[2] = (void *)&buf_size;
        args[3] = (void *)&offset;
        // DLOG("file descrip: %d, path: %s, before call", ((void *)args[4])->fh, path);


        //rpc_ret expected to return the num of bytes that the server read if succeed, else negative
        int rpc_ret = rpcCall((char *)"read", arg_types, args);
        DLOG("file descrip: %d, path: %s, after call", fi->fh, path);

        // int fxn_ret = 0;
        if (rpc_ret < 0) {
            DLOG("read rpc failed with error '%d'", rpc_ret);
            return -EINVAL;
        } else {
            // Our RPC call succeeded. However, it's possible that the return code
            // from the server is not 0, that is it may be -errno. Therefore, we
            // should set our function return value to the retcode from the server.
            DLOG("read rpc call sucess with retcode '%d'", retCode);
            if (retCode < 0){
                return retCode;
            }
            if(retCode == 0){
                break;
            }
            DLOG("read bytes this time: '%d'", retCode);
            // update read_size with num of bytes we just read
            read_size += retCode;
            // copy the new content into buf
			// buf = (char *)mempcpy(buf, tep_buf, retCode);
            offset += retCode;
            buf += retCode;
            // DLOG("tep_buf: %s, bufsize: %ld, offset_each: %ld, after call", tep_buf, buf_size, current_offset);
            DLOG("file descrip: %d, path: %s, after call", fi->fh, path);
            DLOG("buf: %s, after call", buf);

            // release the memory
            // free(tep_buf);
            // if EOF then return
            DLOG("read bytes this time: '%d'", retCode);
            DLOG("buf size this time: '%d'", buf_size);

            if(retCode < buf_size){
                break;
            }
            //update offset and buf pointer
        }
    }
    delete []args;

    // if no error return the size of bytes have been read
    DLOG("read returned! read bytes: '%d'", retCode);
    return read_size;
}

int utils_write(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi) {
    // Write size amount of data at offset of file from buf.

    // Remember that size may be greater then the maximum array size of the RPC
    // library.
    int ARG_COUNT = 6;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    long write_size = 0;
    long buf_size = MAX_ARRAY_LEN;

    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;

    arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    // TODO : offset need to change each time you read
    arg_types[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    arg_types[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); 
    
    arg_types[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[5] = (void*)(&retCode);
    args[0] = (void *)path;
    args[4] = (void *)fi;


    arg_types[6] = 0;

    long current_offset = offset;
    // char *tempBufCache = buf;
    // char *tempBuf = (char *)malloc(size);		// tempBufCache is a copy of buf, a pointer for loop
    // memcpy(tempBuf, buf, size);	

    while (write_size < size){
        DLOG("write_size this loop '%d'", write_size);
        if(size < buf_size){
            buf_size = size;
        }
        if((size - write_size) < buf_size){
            buf_size = size - write_size;
            DLOG("buf_size new:'%d'", buf_size);

        }

        // char *tep_buf = (char *)malloc(buf_size);
		// 			// tempBufCache is used to write bufCache from the server to buf, a pointer for loop
		// memset(tep_buf, 0, sizeof(buf_size));

        // memcpy(tep_buf, tempBuf, buf_size);
        // DLOG("client tep_buf add '%u'", tep_buf);
        
        // Fill in the arguments
        arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) buf_size;
        args[1] = (void *)buf;
        args[2] = (void *)&buf_size;
        args[3] = (void *)&offset;
        // DLOG("file descrip: %d, path: %s, before call", ((void *)args[4])->fh, path);


        //rpc_ret expected to return the num of bytes that the server read if succeed, else negative
        int rpc_ret = rpcCall((char *)"write", arg_types, args);
        DLOG("file descrip: %d, path: %s, after call", fi->fh, path);

        // int fxn_ret = 0;
        if (rpc_ret < 0) {
            DLOG("read rpc failed with error '%d'", rpc_ret);
            return -EINVAL;
        } else {
            // Our RPC call succeeded. However, it's possible that the return code
            // from the server is not 0, that is it may be -errno. Therefore, we
            // should set our function return value to the retcode from the server.
            DLOG("read rpc call sucess with retcode '%d'", retCode);
            if(retCode < 0){
                return retCode;
            }
            if(retCode == 0){
                break;
            }
            DLOG("write bytes this time: '%d'", retCode);
            // update write_size with num of bytes we just read
            write_size += retCode;
            buf += retCode;
            // copy the new content into buf
			// tempBufCache = (char *)mempcpy(tempBufCache, tep_buf, retCode);
            offset += retCode;
            // DLOG("tep_buf: %s, bufsize: %ld, current_offset: %ld, after call", tep_buf, buf_size, current_offset);
            DLOG("file descrip: %d, path: %s, after call", fi->fh, path);
            DLOG("buf: %s, after call", buf);

            // release the memory
            // free(tep_buf);
            // if EOF then return
            if(retCode < buf_size){
                break;
            }
            //update offset and buf pointer
        }
    }
    delete []args;

    // if no error return the size of bytes have been read
    return write_size;
}

int utils_truncate(void *userdata, const char *path, off_t newsize) {
    // Change the file size to newsize.
 // SET UP THE RPC CALL
    DLOG("watdfs_cli_truncate called for '%s'", path);

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

     // The second argument is newsize, an output only argument, which is
    // an integer.
    arg_types[1] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
    args[1] = (void *)(&newsize);

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"truncate", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("truncate rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("truncate rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}


int utils_fsync(void *userdata, const char *path,
                     struct fuse_file_info *fi) {
    // Force a flush of file data.
     // Called during open.
    // You should fill in fi->fh.

    DLOG("watdfs_cli_fsync called for '%s'", path);
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
    int rpc_ret = rpcCall((char *)"fsync", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("fsync rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("fsync rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

int utils_utimensat(void *userdata, const char *path,
                       const struct timespec ts[2]) {
    // Change file access and modification times.
    DLOG("watdfs_cli_utimensat called for '%s'", path);
    // getattr has 3 arguments.
    int ARG_COUNT = 3;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;
    DLOG("want to set last access time '%ld'", ts[1].tv_sec);
    DLOG("want to set last access time '%ld'", ts[1].tv_nsec);
    DLOG("want to set last modification time '%ld'", ts[2].tv_sec);
    DLOG("want to set last modification time '%ld'", ts[2].tv_nsec);

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the ts.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) (2 * sizeof(struct timespec)); // statbuf
    args[1] = (void *)ts;

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"utimensat", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("utimensat rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("utimensat rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;


    // Finally return the value we got from the server.
    return fxn_ret;
}

// todo impl
int utils_lock(void *userdata, const char *path, rw_lock_mode_t mode){
    // int rpc_ret = rpcCall((char *)"lock", arg_types, args);
    return -1;
}

// todo impl
int utils_unlock(void *userdata, const char *path, rw_lock_mode_t mode){
    // int rpc_ret = rpcCall((char *)"unlock", arg_types, args);
    return -1;
}

// find out whether the file has opened, return 1 if open, else 0
int utils_isopen(void *userdata, const char *path){
    std::string path_str = path;
    DLOG("try to get open state for path %s\n", path);

    if(((cli_data*)userdata)->open_map->find(path_str) != 
     ((cli_data*)userdata)->open_map->end())
        return 1;
    else 
        return 0;
}

char *utils_get_full_path(void * userdata, const char *short_path) {
    DLOG("try to get full length\n");

    int short_path_len = strlen(short_path);
    int dir_len = strlen(((cli_data*)userdata)->cache_path);
    int full_len = dir_len + short_path_len + 1;
    DLOG("Full length: %d\n", full_len);

    char *full_path = (char *)malloc(full_len);

    // First fill in the directory.
    strcpy(full_path, ((cli_data*)userdata)->cache_path);
    // Then append the path.
    strcat(full_path, short_path);
    DLOG("Full path: %s\n", full_path);

    return full_path;
}

// check freshness,whether the file is time out, only for files already open
bool utils_fresh_timeout(void * userdata, const char *short_path){
    std::string path_name = short_path;
    meta_d file_meta = (((cli_data*)userdata)->open_map->find(path_name))->second;
    time_t tc = file_meta.tc;
    time_t current_time = time(0);
    return (current_time - tc) >= ((cli_data*)userdata)->cache_intvl;
}

// get the mode/falg of an opened file, O_RDONLY, etc.
int utils_get_open_mode(void * userdata, const char *short_path){
    std::string path_name = short_path;
    meta_d file_meta = (((cli_data*)userdata)->open_map->find(path_name))->second;
    int mode = file_meta.client_file_inf.flags;
    return mode & O_ACCMODE;
}

// transfer file from server to client, return 0 if succeed, else errno
int utils_download(void * userdata, const char* path, struct fuse_file_info *client_file_info, struct fuse_file_info *server_file_info){
    char * full_path = utils_get_full_path(userdata, path);
    // utils_lock(userdata, path, ) // read mode

    DLOG("Try to local trucnate.");
    int trunc_ret = truncate(full_path, 0); // local truncate 
    if(trunc_ret < 0){
        free(full_path);
        DLOG("local truncate failed.");
        return -errno;
    }
            
    DLOG("try to get server file stat.");
    struct stat *tmp_stat = new struct stat; // remote get_attr
    int server_stat_ret = utils_getattr(userdata, path, tmp_stat);
    if(server_stat_ret < 0){
        delete tmp_stat;
        free(full_path);
        DLOG("get server file stat failed.");

        return server_stat_ret;
    }

    DLOG("try to allocate buf.");
    size_t size = 65536; // read from server
    char * buf = (char *)calloc(size, 1);
    DLOG("allocate buf succeed, try to read from server.");
    int server_read_ret = utils_read(userdata, path, buf, size, 0, server_file_info);
    if(server_read_ret < 0){
        delete tmp_stat;
        free(full_path);
        free(buf);
        DLOG("read from server file failed.");
        return server_read_ret;
    }
    
    DLOG("try write to local copy.");
    // TODO if read 0 bytes?
    int bytes_write = pwrite(client_file_info->fh, buf, server_read_ret, 0); 
    if(bytes_write < 0){
        delete tmp_stat;
        free(full_path);
        free(buf);
        DLOG("local write failed.");
        return -errno;
    }
    free(buf);

    DLOG("try to local utimensat the copy.");
    struct timespec ts[2] = {tmp_stat->st_atim, tmp_stat->st_mtim};
    int utime_ret = utimensat(0, full_path, ts, 0); // update T_client for the file
    if(utime_ret < 0){
        delete tmp_stat;
        free(full_path);
        free(buf);
        DLOG("local utimensat failed.");

        return -errno;
    }

    // need to update metadata like Tc when has been open
    if(utils_isopen(userdata, path)){
        // meta_d file_meta = (((cli_data*)userdata)->open_map->find(path_name))->second;
        std::string str_path = path;
        // std::map <std::string, meta_d> *tmp_map = ((cli_data*)userdata)->open_map;  // notice that it is a pointer to mapping boject
        (*(((cli_data*)userdata)->open_map))[str_path].tc = time(0);
        // (*tmp_map)[str_path].tc = time(0);
    }

    // utils_unlock(userdata, path, ) // read mode

    delete tmp_stat;
    free(full_path);
    return 0;
}

// todo impl
// upload file to remote server
int utils_upload(void * userdata, const char* path, struct fuse_file_info *client_file_info, struct fuse_file_info *server_file_info){


    // utils_lock(userdata, path, ) // write mode

    int server_trunc = utils_truncate(userdata ,path, 0);
    if(server_trunc < 0){
        return server_trunc;
    }

    char * full_path = utils_get_full_path(userdata, path);

    struct stat *tmp_stat = new struct stat; // local stat, to get 
    int stat_ret = stat(full_path, tmp_stat);
    if(stat_ret < 0){
        delete tmp_stat;
        free(full_path);
        return -errno;
    }

    // todo 

    size_t size = 65536; // read from server
    char * buf = (char *)calloc(size, 1);
    int read_ret = pread(client_file_info->fh, buf, size, 0);
    if(read_ret < 0){
        delete tmp_stat;
        free(full_path);
        free(buf);
        return -errno;    
     }
    
    // TODO if read 0 bytes?
    int server_write = utils_write(userdata, path, buf, read_ret, 0, server_file_info); 
    if(server_write < 0){
        delete tmp_stat;
        free(full_path);
        free(buf);
        return server_write;
    }
    free(buf);

    // TODO not necessary to allocate dump memory, since ts is input
    struct timespec ts[2] = {tmp_stat->st_atim, tmp_stat->st_mtim};  
    int server_utime_ret = utils_utimensat(userdata, path, ts); // update T_client for the file
    if(server_utime_ret < 0){
        delete tmp_stat;
        free(full_path);
        return server_utime_ret;
    }

    delete tmp_stat;
    free(full_path);

    // utils_unlock(userdata, path, ) // write mode

    return 0;
}

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
    // cli_data *userdata = (cli_data *)malloc(sizeof(cli_data));// todo maybe can try new
    cli_data *userdata = new cli_data;
    userdata->cache_intvl = cache_interval;

    userdata->cache_path = (char*)malloc(strlen(path_to_cache) + 1);
    strcpy(userdata->cache_path, path_to_cache);

    userdata->open_map = new std::map <std::string, meta_d>; // pointer

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
    if(userdata != NULL){
        ((cli_data*)userdata)->open_map->clear(); // todo need to free the space each struct used before this
        delete ((cli_data*)userdata)->open_map;
        // TODO: and mapping need to be delete?
        free(((cli_data*)userdata)->cache_path);
        delete userdata;
    }
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
    char *full_path = utils_get_full_path(userdata, path);
    int fxn_ret = 0;
    // TODO check client open state
    DLOG("try to judge open state '%s'", path);

    if(utils_isopen(userdata, path)){ // client has opened the file
        DLOG("file has opened.");
        // todo time check 
        if(utils_get_open_mode(userdata, path) == O_RDONLY){ // if has opended at read only mode
            DLOG("file has opened in read only mode.");
            // need to do freshness check
            if(utils_fresh_timeout(userdata, path)){
                DLOG("file time out.");

                struct stat tmp_stat;
                if(stat(full_path, &tmp_stat) < 0){
                    free(full_path);
                    return -errno;
                }
                struct timespec t_client;
                t_client = tmp_stat.st_mtim;
                DLOG("get client modify time : %ld.", t_client.tv_sec);

                int ret = utils_check_server_time(userdata, path, &t_client);
                if(ret < 0){
                    free(full_path);
                    return ret;
                }else if(ret == 0){ // time out , t_client != t_server ,need download
                    DLOG("also, t_client != t_server, need download.");

                    struct fuse_file_info *tmp_info_server = new struct fuse_file_info;
                    tmp_info_server->flags = O_RDONLY;  // for download, server should open in read mode
                    // try to open on the server
                    int open_res = utils_open(userdata, path, tmp_info_server); // temporarily open the file on the server for download
                    DLOG("open file on the server for temp download with result '%d'", open_res);
                    if(open_res < 0){
                        free(full_path);
                        DLOG("server open failed.");
                        return open_res; // maybe be not exist on server
                    }

                    struct fuse_file_info *tmp_info_cli = new struct fuse_file_info;
                    tmp_info_cli->flags = O_RDWR;
                    int open_local = open(full_path, tmp_info_cli->flags);
                    if(open_local < 0){
                        free(full_path);
                        DLOG("local open failed.");
                        return -errno; // maybe be not exist on server
                    }
                    // 本地也得重新打开文件？ 不然你怎么写呢
                    int download_ret = utils_download(userdata, path, tmp_info_cli, tmp_info_server); // TODO need to set arguments
                    if(download_ret < 0){
                        return download_ret;
                    }

                    int close_local = close(tmp_info_cli->fh);
                    DLOG("close file on the client for temp download with result '%d'", close_local);

                    int close_res = utils_release(userdata, path, tmp_info_server);
                    DLOG("close file on the server for temp download with result '%d'", close_res);
                    delete tmp_info_server;
                    delete tmp_info_cli;
                }
            }
        }
        int res = stat(full_path, statbuf);
        if(res < 0){
            memset(statbuf, 0, sizeof(struct stat));
            fxn_ret = -errno;
        }
    }else{ // not open
        // first transfer from the server
        DLOG("file has not opened.");

        struct fuse_file_info *tmp_info_server = new struct fuse_file_info;
        tmp_info_server->flags = O_RDONLY;  // for download, server should open in read mode
        // try to open on the server
        int open_res = utils_open(userdata, path, tmp_info_server); // temporarily open the file on the server for download
        DLOG("open file on the server for temp download with result '%d'", open_res);
        if(open_res < 0){
            free(full_path);
            delete tmp_info_server;
            DLOG("server open failed.");
            return open_res; // other client may has opened the file in write mode, or file maybe not exist on server
        }

        struct fuse_file_info *tmp_info_cli = new struct fuse_file_info;
        tmp_info_cli->flags = O_RDWR | O_CREAT;
        int open_local = open(full_path, tmp_info_cli->flags, 0777);
        DLOG("open file at local with fd for temp: '%d'", open_local);
        if(open_local < 0){
            free(full_path);
            delete tmp_info_server;
            delete tmp_info_cli;   
            DLOG("local open failed.");
            return -errno; // maybe be not exist on server
        }
        tmp_info_cli->fh = open_local;

        int download_ret = utils_download(userdata, path, tmp_info_cli, tmp_info_server); // TODO need to set arguments
        if(download_ret < 0){
            free(full_path);
            delete tmp_info_server;
            delete tmp_info_cli;      
            DLOG("download failed.");
            return download_ret;
        }

        int close_local = close(tmp_info_cli->fh);
        DLOG("close file on the client for temp download with result '%d'", close_local);

        int close_res = utils_release(userdata, path, tmp_info_server);
        DLOG("close file on the server for temp download with result '%d'", close_res);
        
        int res = stat(full_path, statbuf);
        if(res < 0){
            memset(statbuf, 0, sizeof(struct stat));
            fxn_ret = -errno;
        }
        delete tmp_info_server;
        delete tmp_info_cli;        
    } 

    free(full_path);  
    return fxn_ret;
}


// CREATE, OPEN AND CLOSE
int watdfs_cli_mknod(void *userdata, const char *path, mode_t mode, dev_t dev) {
    // Called to create a file.
    DLOG("watdfs_cli_mknod called for '%s'", path);
    int fxn_ret = 0;

    int server_ret = utils_mknod(userdata, path, mode, dev);
    if(server_ret < 0){
        return server_ret; // contain errno
    }

    char * full_path = utils_get_full_path(userdata, path);
    int local_mknod = mknod(full_path, mode, dev);  // todo 如果文件存在， 做成更改 mode 和 dev？？
    if(local_mknod < 0 and errno != EEXIST){
        return -errno;
    }

    // TODO 是否需要download

    // 需要open
    struct fuse_file_info *tmp_info_server = new struct fuse_file_info;
    tmp_info_server->flags = O_RDWR;  // for download, server should open in read mode
    // try to open on the server
    int open_res = utils_open(userdata, path, tmp_info_server); // temporarily open the file on the server for download
    DLOG("open file on the server for upload with result '%d'", open_res);
    if(open_res < 0){
        delete tmp_info_server;
        free(full_path);
        DLOG("server open failed.");
        return open_res; // maybe be opended by other client in write mode, or not exist on server
    }

    struct fuse_file_info *tmp_info_cli = new struct fuse_file_info;
    tmp_info_cli->flags = O_RDONLY;
    int open_local = open(full_path, tmp_info_cli->flags);
    if(open_local < 0){
        delete tmp_info_server;
        delete tmp_info_cli;
        free(full_path);
        DLOG("local open failed.");
        return -errno; // maybe be not exist on server
    }
    tmp_info_cli->fh = open_local;

    int upload_ret =utils_upload(userdata, path, tmp_info_cli ,tmp_info_server);
    if(upload_ret < 0){
        delete tmp_info_server;
        delete tmp_info_cli;
        free(full_path);
        DLOG("upload failed.");
        return upload_ret;
    }

    int close_local = close(tmp_info_cli->fh);
    DLOG("close file on the client for temp upload with result '%d'", close_local);

    int close_res = utils_release(userdata, path, tmp_info_server);
    DLOG("close file on the server for temp upload with result '%d'", close_res); 
    delete tmp_info_server;
    delete tmp_info_cli;    

    // Finally return the value we got from the server.
    return fxn_ret;
}

int watdfs_cli_open(void *userdata, const char *path,
                    struct fuse_file_info *fi) {
    // Called during open.
    DLOG("watdfs_cli_open called for '%s'", path);
    
    if(utils_isopen(userdata, path)){ // for mutual exclusion
        return -EMFILE;
    }

    // You should fill in fi->fh.

    // TODO is it necessary to check T_client and T_server before creating local copy?
    DLOG("downloading '%s' from the server.", path);
    char *full_path = utils_get_full_path(userdata, path);
    
    struct fuse_file_info *tmp_info_server = new struct fuse_file_info;
    tmp_info_server->flags = O_RDONLY;  // for download, server should open in read mode
    // try to open on the server
    int open_res = utils_open(userdata, path, tmp_info_server); // temporarily open the file on the server for download
    DLOG("open file on the server for temp download with result '%d'", open_res);
    if(open_res < 0){
        free(full_path);
        delete tmp_info_server;
        DLOG("server open failed.");
        return open_res; //  file maybe not exist on server
    }

    // TODO is it ok to have O_CREAT flag for local ?
    struct fuse_file_info *tmp_info_cli = new struct fuse_file_info;
    tmp_info_cli->flags = O_RDWR | O_CREAT;
    int open_local = open(full_path, tmp_info_cli->flags, 0777);  // need chmod 
    if(open_local < 0){
        free(full_path);
        delete tmp_info_server;
        delete tmp_info_cli;   
        DLOG("local open failed.");
        return -errno; // maybe be not exist on server
    }
    tmp_info_cli->fh = open_local;


    int download_ret = utils_download(userdata, path, tmp_info_cli, tmp_info_server); // TODO need to set arguments
    if(download_ret < 0){
        free(full_path);
        delete tmp_info_server;
        delete tmp_info_cli;      
        DLOG("download failed.");
        return download_ret;
    }

    int close_local = close(tmp_info_cli->fh);
    DLOG("close file on the client for temp download with result '%d'", close_local);

    int close_res = utils_release(userdata, path, tmp_info_server);
    DLOG("close file on the server for temp download with result '%d'", close_res);
    delete tmp_info_server;
    delete tmp_info_cli;

    int fxn_ret = 0;
    // true open local
    int client_open = open(full_path, fi->flags);
    if(client_open < 0){
        free(full_path);
        return -errno;
    }
    fi->fh = client_open;  // set local descriptor

    struct fuse_file_info server_file_inf;
    server_file_inf.flags = fi->flags;  // open server with input flags
    // try to open on the server
    int server_open = utils_open(userdata, path, &server_file_inf);
    DLOG("open file on the server with result '%d'", open_res);
    if(server_open < 0){
        free(full_path);
        DLOG("server open failed.");
        return server_open; // maybe be not exist on server
    }
    
    std::string path_str = path;
    time_t cur_time = time(0); // need to update metadata at userdata, Tc, file info on cli and server
    (*(((cli_data*)userdata)->open_map))[path_str] = {cur_time, *fi, server_file_inf };
        // insert(std::pair<std::string, meta_d>(path_str, {cur_time, *fi, server_file_inf }));

    free(full_path);
    // Finally return the value we got from the server.
    return fxn_ret;
}

int watdfs_cli_release(void *userdata, const char *path,
                       struct fuse_file_info *fi) {
    // Called during close, but possibly asynchronously.

    DLOG("watdfs_cli_release called for '%s'", path);
    
    struct fuse_file_info server_info;
    std::string path_str = path;
    server_info = (*(((cli_data*)userdata)->open_map))[path_str].server_file_inf;

    //  write back to server for write mode opened file
    if(utils_get_open_mode(userdata, path) != O_RDONLY){
        int upload_ret =utils_upload(userdata, path, fi ,&server_info);
        if(upload_ret < 0){
            return upload_ret;
        }
    }

    int sys_ret = 0;    // local release
    sys_ret = close(fi->fh);
    if(sys_ret < 0){
        return -errno;
    }

    int server_ret = 0; // remote release
    server_ret = utils_release(userdata, path, &server_info);
    if(server_ret < 0){
        return server_ret;
    }

    ((cli_data*)userdata)->open_map->erase(path_str);  // metadata update

    return 0;
}

// READ AND WRITE DATA
int watdfs_cli_read(void *userdata, const char *path, char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    // Read size amount of data at offset of file into buf.
    // Remember that size may be greater then the maximum array size of the RPC
    // library.

    int ARG_COUNT = 6;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    long read_size = 0;
    long buf_size = MAX_ARRAY_LEN;

    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;

    arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    // TODO : offset need to change each time you read
    arg_types[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    arg_types[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); 
    
    arg_types[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[5] = (void*)(&retCode);

    args[0] = (void *)path;
    args[4] = (void *)fi;
    arg_types[6] = 0;

    // long current_offset = offset;
    // char *tempBufCache = buf;

    while (read_size < size){
        DLOG("read_size this loop '%d'", read_size);

        // char *tep_buf = (char *)malloc(buf_size);
		// 			// tempBufCache is used to write bufCache from the server to buf, a pointer for loop
		// memset(tep_buf, 0, sizeof(buf_size));

        // DLOG("client tep_buf add '%u'", tep_buf);
        if(size < MAX_ARRAY_LEN){
            buf_size = size;
            DLOG("size: %d,  is smaller than buf_size",  size);
        }
        if(size - read_size < MAX_ARRAY_LEN){
            buf_size = size - read_size;
        }

        // Fill in the arguments
        arg_types[1] = (1u << ARG_OUTPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) buf_size;
        args[1] = (void *)buf;
        args[2] = (void *)&buf_size;
        args[3] = (void *)&offset;
        // DLOG("file descrip: %d, path: %s, before call", ((void *)args[4])->fh, path);


        //rpc_ret expected to return the num of bytes that the server read if succeed, else negative
        int rpc_ret = rpcCall((char *)"read", arg_types, args);
        DLOG("file descrip: %d, path: %s, after call", fi->fh, path);

        // int fxn_ret = 0;
        if (rpc_ret < 0) {
            DLOG("read rpc failed with error '%d'", rpc_ret);
            return -EINVAL;
        } else {
            // Our RPC call succeeded. However, it's possible that the return code
            // from the server is not 0, that is it may be -errno. Therefore, we
            // should set our function return value to the retcode from the server.
            DLOG("read rpc call sucess with retcode '%d'", retCode);
            if (retCode < 0){
                return retCode;
            }
            if(retCode == 0){
                break;
            }
            DLOG("read bytes this time: '%d'", retCode);
            // update read_size with num of bytes we just read
            read_size += retCode;
            // copy the new content into buf
			// buf = (char *)mempcpy(buf, tep_buf, retCode);
            offset += retCode;
            buf += retCode;
            // DLOG("tep_buf: %s, bufsize: %ld, offset_each: %ld, after call", tep_buf, buf_size, current_offset);
            DLOG("file descrip: %d, path: %s, after call", fi->fh, path);
            DLOG("buf: %s, after call", buf);

            // release the memory
            // free(tep_buf);
            // if EOF then return
            DLOG("read bytes this time: '%d'", retCode);
            DLOG("buf size this time: '%d'", buf_size);

            if(retCode < buf_size){
                break;
            }
            //update offset and buf pointer
        }
    }
    delete []args;

    // if no error return the size of bytes have been read
    DLOG("read returned! read bytes: '%d'", retCode);
    return read_size;
}


// write data
int watdfs_cli_write(void *userdata, const char *path, const char *buf,
                     size_t size, off_t offset, struct fuse_file_info *fi) {
    // Write size amount of data at offset of file from buf.

    // Remember that size may be greater then the maximum array size of the RPC
    // library.
    int ARG_COUNT = 6;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;

    long write_size = 0;
    long buf_size = MAX_ARRAY_LEN;

    arg_types[0] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;

    arg_types[2] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    // TODO : offset need to change each time you read
    arg_types[3] = (1u << ARG_INPUT) | (ARG_LONG << 16u);

    arg_types[4] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) sizeof(struct fuse_file_info); 
    
    arg_types[5] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[5] = (void*)(&retCode);
    args[0] = (void *)path;
    args[4] = (void *)fi;


    arg_types[6] = 0;

    long current_offset = offset;
    // char *tempBufCache = buf;
    // char *tempBuf = (char *)malloc(size);		// tempBufCache is a copy of buf, a pointer for loop
    // memcpy(tempBuf, buf, size);	

    while (write_size < size){
        DLOG("write_size this loop '%d'", write_size);
        if(size < buf_size){
            buf_size = size;
        }
        if((size - write_size) < buf_size){
            buf_size = size - write_size;
            DLOG("buf_size new:'%d'", buf_size);

        }

        // char *tep_buf = (char *)malloc(buf_size);
		// 			// tempBufCache is used to write bufCache from the server to buf, a pointer for loop
		// memset(tep_buf, 0, sizeof(buf_size));

        // memcpy(tep_buf, tempBuf, buf_size);
        // DLOG("client tep_buf add '%u'", tep_buf);
        
        // Fill in the arguments
        arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) buf_size;
        args[1] = (void *)buf;
        args[2] = (void *)&buf_size;
        args[3] = (void *)&offset;
        // DLOG("file descrip: %d, path: %s, before call", ((void *)args[4])->fh, path);


        //rpc_ret expected to return the num of bytes that the server read if succeed, else negative
        int rpc_ret = rpcCall((char *)"write", arg_types, args);
        DLOG("file descrip: %d, path: %s, after call", fi->fh, path);

        // int fxn_ret = 0;
        if (rpc_ret < 0) {
            DLOG("read rpc failed with error '%d'", rpc_ret);
            return -EINVAL;
        } else {
            // Our RPC call succeeded. However, it's possible that the return code
            // from the server is not 0, that is it may be -errno. Therefore, we
            // should set our function return value to the retcode from the server.
            DLOG("read rpc call sucess with retcode '%d'", retCode);
            if(retCode < 0){
                return retCode;
            }
            if(retCode == 0){
                break;
            }
            DLOG("write bytes this time: '%d'", retCode);
            // update write_size with num of bytes we just read
            write_size += retCode;
            buf += retCode;
            // copy the new content into buf
			// tempBufCache = (char *)mempcpy(tempBufCache, tep_buf, retCode);
            offset += retCode;
            // DLOG("tep_buf: %s, bufsize: %ld, current_offset: %ld, after call", tep_buf, buf_size, current_offset);
            DLOG("file descrip: %d, path: %s, after call", fi->fh, path);
            DLOG("buf: %s, after call", buf);

            // release the memory
            // free(tep_buf);
            // if EOF then return
            if(retCode < buf_size){
                break;
            }
            //update offset and buf pointer
        }
    }
    delete []args;

    // if no error return the size of bytes have been read
    return write_size;
}

int watdfs_cli_truncate(void *userdata, const char *path, off_t newsize) {
    // Change the file size to newsize.
 // SET UP THE RPC CALL
    DLOG("watdfs_cli_truncate called for '%s'", path);

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

     // The second argument is newsize, an output only argument, which is
    // an integer.
    arg_types[1] = (1u << ARG_INPUT) | (ARG_LONG << 16u);
    args[1] = (void *)(&newsize);

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"truncate", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("truncate rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("truncate rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}



int watdfs_cli_fsync(void *userdata, const char *path,
                     struct fuse_file_info *fi) {
    // Force a flush of file data.
     // Called during open.
    // You should fill in fi->fh.

    DLOG("watdfs_cli_fsync called for '%s'", path);
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
    int rpc_ret = rpcCall((char *)"fsync", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("fsync rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("fsync rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;

    // Finally return the value we got from the server.
    return fxn_ret;
}

// CHANGE METADATA
int watdfs_cli_utimensat(void *userdata, const char *path,
                       const struct timespec ts[2]) {
    // Change file access and modification times.
    DLOG("watdfs_cli_utimensat called for '%s'", path);
    // getattr has 3 arguments.
    int ARG_COUNT = 3;
    void **args = new void*[ARG_COUNT];
    int arg_types[ARG_COUNT + 1];
    int pathlen = strlen(path) + 1;
    DLOG("want to set last access time '%ld'", ts[1].tv_sec);
    DLOG("want to set last access time '%ld'", ts[1].tv_nsec);
    DLOG("want to set last modification time '%ld'", ts[2].tv_sec);
    DLOG("want to set last modification time '%ld'", ts[2].tv_nsec);

    // Fill in the arguments
    // The first argument is the path, it is an input only argument, and a char
    // array. The length of the array is the length of the path.
    arg_types[0] =
        (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) pathlen;
    // For arrays the argument is the array pointer, not a pointer to a pointer.
    args[0] = (void *)path;

    // The second argument is the ts.
    arg_types[1] = (1u << ARG_INPUT) | (1u << ARG_ARRAY) | (ARG_CHAR << 16u) | (uint) (2 * sizeof(struct timespec)); // statbuf
    args[1] = (void *)ts;

    // The third argument is retcode, an output only argument, which is
    // an integer.
    arg_types[2] = (1u << ARG_OUTPUT) | (ARG_INT << 16u);
    int retCode;
    args[2] = (void*)(&retCode);

    // Finally, the last position of the arg types is 0. There is no
    // corresponding arg.
    arg_types[3] = 0;

    // MAKE THE RPC CALL
    int rpc_ret = rpcCall((char *)"utimensat", arg_types, args);

    // HANDLE THE RETURN
    // The integer value watdfs_cli_mknod will return.
    int fxn_ret = 0;
    if (rpc_ret < 0) {
        DLOG("utimensat rpc failed with error '%d'", rpc_ret);
        // Something went wrong with the rpcCall, return a sensible return
        // value. In this case lets return, -EINVAL
        fxn_ret = -EINVAL;
    } else {
        // Our RPC call succeeded. However, it's possible that the return code
        // from the server is not 0, that is it may be -errno. Therefore, we
        // should set our function return value to the retcode from the server.
        DLOG("utimensat rpc call sucess with retcode '%d'", retCode);
        fxn_ret = retCode;
        // TODO: set the function return value to the return code from the server.
    }
    // Clean up the memory we have allocated.
    delete []args;


    // Finally return the value we got from the server.
    return fxn_ret;
}


    // char *full_path = utils_get_full_path(userdata, path);
    //     if(access(full_path, F_OK) != -1){ // local exist the copy of the file
    //             struct stat tmp_stat;
    //             if(stat(full_path, &tmp_stat) == 0){
    //                 struct timespec t_client = tmp_stat.st_mtim;
    //                 if(T -tc < cache_interval ){ // 时间有效
    //                     *statbuf = tmp_stat;
    //                     free(full_path);
    //                     return 0;
    //                 }else{ // out of time, need tocheck
    //                     int freshness = utils_check_server_time(userdata, path, &t_client);
    //                     if(freshness == 1){
    //                         *statbuf = tmp_stat;
    //                         free(full_path);
    //                         return 0;
    //                     }elseif(freshness == 0)
    //                         // todo open on the server
    //                         utils_download();
    //                         // todo close on the server
    //                         stat();
    //                         free(full_path);
    //                         return 0;
    //                     }else{ // time check error

    //                     }
    //             }else{
    //                 // stat error
    //             }
    //          if(utils_check_server_time(userdata, path,))
    //     }else{ // local does not exist copy 
    //         TODO open on the server
    //         utils_download()
    //         close on the server
    //         res = stat()
    //         free(full_path);
    //         return 0;

    //     }