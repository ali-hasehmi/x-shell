#include "xfile.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "xfragment.h"

#define DEFAULT_MODE 0644

ssize_t fsize(const char *_file_path)
{
    if (_file_path == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    struct stat st;
    if (stat(_file_path, &st))
    {
        return -1;
    }

    return st.st_size;
}

mode_t fmode(const char *_file_path)
{
    if (_file_path == NULL)
    {
        errno = EINVAL;
        return 0;
    }

    struct stat st;
    if (stat(_file_path, &st))
    {
        return 0;
    }

    return st.st_mode;
}

int btosd(double _num_of_bytes, char *_buff, uint32_t _buff_size)
{
    if (_num_of_bytes < 0)
    {
        // invalid number of bytes
        return -1;
    }
    if (_buff == NULL)
    {
        // invalid pointer to buffer
        return -2;
    }
    if (_buff_size == 0)
    {
        // invalid buffer size
        return -3;
    }

    char sign[][4] = {
        "B",
        "KiB",
        "MiB",
        "GiB"};

    int i = 0;
    while (i < 3 && _num_of_bytes > 1024)
    {
        _num_of_bytes /= 1024;
        i++;
    }
    snprintf(_buff, _buff_size, "%.1lf%s", _num_of_bytes, sign[i]);
    return 0;
}

int xfopen(const char *_file_path, xfile_t *_file, int _xflag)
{
    if (_file == NULL)
    {
        errno = EINVAL;
        return 1;
    }
    strcpy(_file->x_path, _file_path);
    if ((_file->x_descriptor = open(_file_path, _xflag, DEFAULT_MODE)) == -1)
    {
        return -1;
    }

    // if (strncpy(_file->x_path, _file_path, XF_PATHLEN) == NULL)
    // {
    //     errno = ERANGE;
    //     return -2;
    // }

    _file->x_rel_size = 0;

    if ((_file->x_size = fsize(_file->x_path)) == -1)
    {
        printf("[!] xfopen() stat failed\n");
        return -3;
    }

    return 0;
}

int xfclose(xfile_t *_file)
{
    if (close(_file->x_descriptor) == -1)
    {
        return -1;
    }
    return 0;
}

int printxfile(const xfile_t *_file)
{
    if (_file == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    printf("#path: %s\n#fd: %d\n#size: %lu\n#offset: %lu\n",
           _file->x_path,
           _file->x_descriptor,
           _file->x_size,
           _file->x_rel_size);

    return 0;
}

int xfile_send(xtcpsocket_t *_socket, xfile_t *_file)
{
    xfragment_t tmp_frag;
    while (_file->x_rel_size < _file->x_size)
    {
        if (write_file_to_fragment(_file, &tmp_frag) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_send():write_file_to_fragment() failed\r\n");
            return -1;
        }
        if (send_xfragment(_socket, &tmp_frag) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_send():send_xfragment() failed\r\n");
            return -1;
        }
    }

    return 0;
}

int xfile_recv(xtcpsocket_t *_socket, xfile_t *_file)
{
    xfragment_t tmp_frag;
    while (_file->x_rel_size < _file->x_size)
    {
        if (recv_xfragment(_socket, &tmp_frag) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_recv():recv_xfragment() failed\r\n");
            return -1;
        }
        if (write_fragment_to_file(&tmp_frag, _file) == -1)
        {
            fprintf(stderr,
                    "[!] xfile_recv():write_fragment_to_file() failed\r\n");
            return -1;
        }
    }
    return 0;
}

// int handleDuplication(xfile_t *_file, int _xflags)
// {
//     errno = 0;
//     bool isFileCreated = false;
//     int i = 2;
//     char newFileNameBuff[141];
//     xpath_t tmp = _file->x_path;
//     while (!isFileCreated)
//     {
//         snprintf(newFileNameBuff, 141, "%s(%d)", _file->x_path.p_fname, i);
//         xpimport(&tmp, NULL, newFileNameBuff, NULL);
//         if ((_file->x_descriptor = open(tmp.p_full_path, _xflags, DEFAULT_MODE)) != -1)
//         {
//             isFileCreated = true;
//         }
//         else if (errno != EEXIST)
//         {
//             return -1;
//         }
//         i++;
//     }
//     _file->x_path = tmp;
//     return 0;
// }
