#ifndef XFILE_H
#define XFILE_H

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "xtcpsocket.h"

#define XF_RDONLY O_RDONLY
#define XF_WRONLY O_WRONLY
#define XF_RDWR O_RDWR
#define XF_APPEND O_APPEND
#define XF_CREAT O_CREAT
#define XF_EXCL O_EXCL
#define XF_CREAT O_CREAT

#define XF_PATHLEN 256

typedef struct xfile
{
    int x_descriptor;        /* file descriptor which returned from open() */
    char x_path[XF_PATHLEN]; /* Path to the file*/
    uint64_t x_size;         /* size of the file */
    uint64_t x_rel_size;     /* relative size shows how many bytes have been read from/written to fragments so far*/
} xfile_t;

/* file Size
Return size of a file showd by the path
Return 0 if _file_path is invalid*/
ssize_t fsize(const char *_file_path);

/*
    file access mode
    return the access file
*/
mode_t fmode(const char *_file_path);

/* Byte to Scaled Data
convert Number of Bytes to scaled Format e.g. MiB
and put the result in the memory pointed by buff argument
return 0 on success*/
int btosd(double _num_of_bytes, char *_buff, uint32_t _buff_size);

/*
    open a xfile with given flags
    return 0 on success
    return otherwise on failure
*/
int xfopen(const char *_file_path, xfile_t *_file, int _xflag);

/* close a xfile
return 0 on success*/
int xfclose(xfile_t *_file);

int printxfile(const xfile_t *_file);

/*
    send a file using socket
    return 0 on success
    return -1 on failure
*/
int xfile_send(xtcpsocket_t* _socket, xfile_t *_file);

/*
    receive a file from socket and store it in _file
    return 0 on success
    return -1 on failure
*/
int xfile_recv(xtcpsocket_t* _socket, xfile_t *_file);
// static int handleDuplication(xfile_t* _file,int _xflag);

#endif // XFILE_H