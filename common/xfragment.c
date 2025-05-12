#include "xfragment.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

// __thread static bool logStatus;
// __thread static FILE* logFile;

// static char default_store_path[XP_PATH_LEN] = "./X-Files/";

ssize_t write_file_to_fragment(xfile_t *_file, xfragment_t *_fragment)
{
    /* Valiating Pointers */
    if (_file == NULL || _fragment == NULL)
    {
        // invalid Pointers
        errno = EINVAL;
        return -1;
    }

    // if (_fragment->buff_len > 0)
    // {
    //     // Buffer is not empty
    //     errno = EAGAIN;
    //     return -1;
    // }
    _fragment->f_flag = XFLAG_FILE_DATA;
    _fragment->buff_len = 0;
    _fragment->f_offset = _file->x_rel_size;
    _fragment->f_size = _file->x_size;
    strcpy(_fragment->f_target, _file->x_path);

    // calculate the amount of data has to be written in fragment
    ssize_t data_size, fragment_size;
    if ((data_size = _file->x_size - _file->x_rel_size) < XFBUFF_SIZE)
    {
        fragment_size = data_size;
    }
    else
    {
        fragment_size = XFBUFF_SIZE;
    }

    // while loop to  assure that the whole Data has been written to fragment
    while (fragment_size > 0)
    {
        /*
        ssize_t pread(int fd, void buf[.count], size_t count,
                         off_t offset);
                         */
        ssize_t byte_write_to_fragment = pread(_file->x_descriptor, _fragment->buffer + _fragment->buff_len,
                                               XFBUFF_SIZE - _fragment->buff_len, _file->x_rel_size);
        if (byte_write_to_fragment == -1)
        {
            return -1;
        }
        _file->x_rel_size += byte_write_to_fragment;
        _fragment->buff_len += byte_write_to_fragment;
        fragment_size -= byte_write_to_fragment;
    }
    return _fragment->buff_len;
}

ssize_t write_fragment_to_file(const xfragment_t *_fragment, xfile_t *_file)
{
    if (_fragment == NULL || _file == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    /*
    ssize_t pwrite(int fd, const void *buf, size_t  count,  off_t  off‐
       set);
       */
    ssize_t byte_written = 0;
    while (byte_written < _fragment->buff_len)
    {
        ssize_t res;
        if ((res = pwrite(_file->x_descriptor, _fragment->buffer + byte_written, _fragment->buff_len - byte_written, _fragment->f_offset + byte_written)) == -1)
        {
            return -1;
        }
        byte_written += res;
        _file->x_rel_size += res;
    }
    return byte_written;
}

int printxfragment(const xfragment_t *_fragment)
{
    if (_fragment == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    printf("#fname: %s\n#fsize: %lu\n#foffset: %lu\n#bufflen: %hu",
           _fragment->f_target,
           _fragment->f_size,
           _fragment->f_offset,
           _fragment->buff_len);
    for (uint16_t i = 0; i < _fragment->buff_len; ++i)
    {
        if (i % 16 == 0)
        {
            putchar('\n');
        }
        printf("%02x ", _fragment->buffer[i]);
    }
    putchar('\n');
}

int send_xfragment(const xtcpsocket_t *_socket, const xfragment_t *_fragment)
{
    xmessage_t msg;
    msg.xm_f = XMESSAGE_FILE;
    msg.xm_len = sizeof(_fragment->buff_len) + sizeof(_fragment->f_flag) + sizeof(_fragment->f_target) +
                 sizeof(_fragment->f_size) + sizeof(_fragment->f_offset) + _fragment->buff_len;
    msg.xm_d = malloc(sizeof(xfragment_t) - XFBUFF_SIZE + _fragment->buff_len);
    memcpy(msg.xm_d, _fragment, msg.xm_len);

    xmessage_queue_enqueue(_socket->outgoing_message_queue, &msg);
    // send everything except buffer data
    // if (xtcpsocket_sendall(_socket,
    //                        _fragment,
    //                        sizeof(xfragment_t) - XFBUFF_SIZE) != sizeof(xfragment_t) - XFBUFF_SIZE)
    // {
    //     return -1;
    // }

    // // send buffer data
    // if (xtcpsocket_sendall(_socket,
    //                        _fragment->buffer,
    //                        _fragment->buff_len) != _fragment->buff_len)
    // {
    //     return -1;
    // }

    fprintf(stderr, "[+] send_xfragment(): %luB sent!\n",
            sizeof(xfragment_t) - XFBUFF_SIZE + _fragment->buff_len);

    return 0;
}

int recv_xfragment(const xtcpsocket_t *_socket, xfragment_t *_fragment)
{
    xmessage_t msg;
    xmessage_queue_dequeue(_socket->incoming_message_queue, &msg);
    if (msg.xm_f != XMESSAGE_FILE)
    {
        return -1;
    }
    memcpy(_fragment, msg.xm_d, msg.xm_len);
    free(msg.xm_d);
    // // send everything except buffer data
    // if (xtcpsocket_recvall(_socket,
    //                        _fragment,
    //                        sizeof(xfragment_t) - XFBUFF_SIZE) != sizeof(xfragment_t) - XFBUFF_SIZE)
    // {
    //     return -1;
    // }

    // // recv buffer data
    // if (xtcpsocket_recvall(_socket,
    //                        &_fragment->buffer,
    //                        _fragment->buff_len) != _fragment->buff_len)
    // {
    //     return -1;
    // }

    fprintf(stderr, "[+] recv_xfragment(): %luB recieved!\n",
            sizeof(xfragment_t) - XFBUFF_SIZE + _fragment->buff_len);

    return 0;
}

int create_ack_init_xfragment(xfragment_t *_fragment)
{
    _fragment->buff_len = 0;
    _fragment->f_flag = XFLAG_ACK_FINIT_SUCCESS;
    _fragment->f_offset = 0;
    return 0;
}

int make_fhandshake_d(const xtcpsocket_t *_socket, xfile_t *_file, int *_mode)
{
    xfragment_t init_frag;
    if (recv_xfragment(_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_xhandshake_d() failed: could not recv initital fragment\r\n");
        return -1;
    }

    int xfopen_flag;
    switch (init_frag.f_flag)
    {
    case XFLAG_FINIT_DOWNLOAD:
        *_mode = XFMODE_DOWNLOAD;
        xfopen_flag = XF_RDONLY;
        break;

    case XFLAG_FINIT_UPLOAD:
        *_mode = XFMODE_UPLOAD;
        xfopen_flag = XF_CREAT | XF_WRONLY | XF_EXCL;
        break;

    default:
        fprintf(stderr,
                "[!] make_fhandshake_d() failed: invalid init flag\r\n");
        return -1;
        break;
    }
    if (xfopen(init_frag.f_target, _file, xfopen_flag))
    {
        fprintf(stderr,
                "[!] make_xhandshake_d():xfopen() failed: %s\r\n", strerror(errno));
        sprintf(init_frag.buffer, "xfopen() failed: %s\r\n", strerror(errno));
        init_frag.f_flag = XFLAG_ACK_FINIT_FAILURE;
        init_frag.buff_len = strlen(init_frag.buffer) + 1;
        if (send_xfragment(_socket, &init_frag) == -1)
        {
            fprintf(stderr,
                    "[!] make_xhandshake() failed: could not send initital ack fragment\r\n");
            return -1;
        }
        return -1;
    }
    if (init_frag.f_flag == XFMODE_UPLOAD)
    {
        _file->x_size = init_frag.f_size;
    }
    else if(init_frag.f_flag == XFMODE_DOWNLOAD){
        init_frag.f_size = _file->x_size;
    }
    //_file->x_size = init_frag.f_size;
    init_frag.f_flag = XFLAG_ACK_FINIT_SUCCESS;
    if (send_xfragment(_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_xhandshake() failed: could not send initital ack fragment\r\n");
        return -1;
    }
    return 0;
}

int make_fhandshake(const xtcpsocket_t *_socket, const char *_target, xfile_t *_file, int _mode)
{
    xfragment_t init_frag;
    create_init_xfragment(&init_frag, _target, _file, _mode);
    if (send_xfragment(_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_xhandshake() failed: could not send initital fragment\r\n");
        return -1;
    }
    if (recv_xfragment(_socket, &init_frag) == -1)
    {
        fprintf(stderr,
                "[!] make_xhandshake() failed: could not receive initital Ack fragment\r\n");
        return -1;
    }
    if (init_frag.f_flag == XFLAG_ACK_FINIT_FAILURE)
    {
        printf(
            "[!] make_xhandshake() failed: error from peer: %s\r\n",
            init_frag.buffer);
        return -1;
    }
    if (init_frag.f_flag != XFLAG_ACK_FINIT_SUCCESS)
    {
        fprintf(stderr,
                "[!] make_xhandshake() failed: Invalid flag\r\n");
        return -1;
    }
    if (_mode == XFMODE_DOWNLOAD)
    {
        _file->x_size = init_frag.f_size;
    }
    return 0;
}

int create_init_xfragment(xfragment_t *_fragment, const char *_target, const xfile_t *_file, int _mode)
{
    _fragment->buff_len = 0;
    switch (_mode)
    {
    case XFMODE_DOWNLOAD:
        _fragment->f_flag = XFLAG_FINIT_DOWNLOAD;
        break;

    case XFMODE_UPLOAD:
        _fragment->f_flag = XFLAG_FINIT_UPLOAD;
        break;

    default:
        fprintf(stderr,
                "[!] create_init_xfragment failed: Invalid mode\r\n");
        break;
    }

    if (strcpy(_fragment->f_target, _target) == NULL)
    {
        fprintf(stderr,
                "[!] create_init_xfragment() failed: could not copy file name\r\n");
        return -1;
    }
    _fragment->f_offset = 0;
    _fragment->f_size = _file->x_size;
    return 0;
}
