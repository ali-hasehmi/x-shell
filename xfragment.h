#ifndef XFRAGMENT_H
#define XFRAGMENT_H

#include <stdint.h>
#include <sys/types.h> // to access ssize_t
#include "xfile.h"
#include "xtcpsocket.h"

#define BYTE uint8_t
#define KiB ((uint32_t)1024U)
#define MiB ((uint32_t)KiB * 1024)
#define GiB ((uint32_t)MiB * 1024)

#define XFBUFF_SIZE 8 * KiB

#define XFMODE_DOWNLOAD 0
#define XFMODE_UPLOAD 1



#define XFLAG_FINIT_DOWNLOAD 0
#define XFLAG_FINIT_UPLOAD 1
#define XFLAG_ACK_FINIT_SUCCESS 2
#define XFLAG_ACK_FINIT_FAILURE 3
#define XFLAG_FILE_DATA 4
#define XFLAG_FFINISH 5

typedef struct xfragment
{
    uint16_t buff_len;        /* actual amount of Data in Buffer */
    BYTE f_flag;              /* fragment flags */
    char f_target[141];       /* File Name */
    uint64_t f_size;          /* absolute File Size */
    uint64_t f_offset;        /* File Offset a.k.a how many byte of the file has been read */
    BYTE buffer[XFBUFF_SIZE]; /* Real-Data */
} xfragment_t;

ssize_t write_file_to_fragment(xfile_t *_file, xfragment_t *_fragment);

ssize_t write_fragment_to_file(const xfragment_t *_fragment, xfile_t *_file);

int printxfragment(const xfragment_t *_fragment);

int send_xfragment(const xtcpsocket_t *_socket, const xfragment_t *_fragment);

int recv_xfragment(const xtcpsocket_t *_socket, xfragment_t *_fragment);

int create_init_xfragment(xfragment_t *_fragment,const char *_target,const xfile_t *_file_name,int _mode);

int create_ack_init_xfragment(xfragment_t *_fragment);

/*
    making handshake from responder in File mode
    sending initial xfragment
    letting reciever know the file name and size
*/
int make_fhandshake_d(const xtcpsocket_t *_socket, xfile_t *_file,int *_mode);

/*
    making handshake from Requester in File mode
    sending initial xfragment
    letting reciever know the file name and size
*/
int make_fhandshake(const xtcpsocket_t *_socket,const char* _target, xfile_t *_file,int _mode);

#endif // XFRAGMENT_H