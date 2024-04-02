#include "xpath.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

int xpcreate(const char *_full_path, xpath_t *_path)
{
    if (_full_path == NULL || _path == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if (XP_FULLPATH_LEN < strlen(_full_path) - 1)
    {
        printf("[!] xpcreate() failed(): XP_FULLPATH_LEN is less\n");
        errno = ERANGE;
        return -1;
    }
    strncpy(_path->p_full_path, _full_path, XP_FULLPATH_LEN);
    if (xtrct(_path->p_full_path, _path->p_path, _path->p_fname, _path->p_fext))
    {
        return -1;
    }
    return 0;
}

int printxpath(const xpath_t *_path)
{
    if (_path == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    printf("file name: %s\nfile extension: %s\npath: %s\nfull path: %s\n",
           _path->p_fname,
           _path->p_fext,
           _path->p_path,
           _path->p_full_path);

    return 0;
}

int import(const char *_file_extension, const char *_file_name, const char *_path, char *_full_path)
{
    if (_full_path == NULL || _path == NULL || _file_name == NULL || _file_extension == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if (strcpy(_full_path, _path) == NULL)
    {
        errno = ERANGE;
        return -1;
    }
    if (strcat(_full_path, _file_name) == NULL)
    {
        errno = ERANGE;
        return -1;
    }
    if(strcat(_full_path,_file_extension)==NULL){
        errno = ERANGE;
        return -1;
    }
    return 0;
}

int xpimport(xpath_t *_path, const char *_new_path, const char *_new_file_name,const char *_new_file_extension)
{
    if (_path == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    if (_new_path != NULL)
    {
        if (strcpy(_path->p_path, _new_path) == NULL)
        {
            errno = ERANGE;
            return -1;
        }
    }
    if (_new_file_name != NULL)
    {
        if (strcpy(_path->p_fname, _new_file_name) == NULL)
        {
            errno = ERANGE;
            return -1;
        }
    }
    if(_new_file_extension != NULL){
        if(strcpy(_path->p_fext,_new_file_extension) == NULL){
            errno = ERANGE;
        }
    }
    if (import(_path->p_fext,_path->p_fname, _path->p_path, _path->p_full_path) == -1)
    {
        return -1;
    }
    return 0;
}

int xtrct(const char *_full_path, char *_path, char *_file_name, char *_file_extension)
{
    if (_full_path == NULL || _path == NULL || _file_name == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    size_t full_path_len = strlen(_full_path);
    int32_t fpIterator = full_path_len - 1,
            pIterator = 0,
            fIterator = 0,
            feIterator = full_path_len;

    bool dotFound = false,
         slashFound = false;
    for (; fpIterator >= 0; fpIterator--)
    {
        if (_full_path[fpIterator] == '/' && !slashFound)
        {
            pIterator = fpIterator;
            slashFound = true;
        }
        if (_full_path[fpIterator] == '.' && !dotFound)
        {
            feIterator = fpIterator;
            dotFound = true;
        }
        if (slashFound && dotFound)
        {
            break;
        }
    }
    // if (feIterator)
    // {

    // copy file extension to corresponding xpath buffer
    if (strncpy(_file_extension, _full_path + feIterator, full_path_len - feIterator) == NULL)
    {
        errno = ERANGE;
        return -1;
    }
    _file_extension[full_path_len - feIterator] = '\0';
    // }

    // copy path to corresponding xpath buffer
    if (strncpy(_path, _full_path, pIterator + 1) == NULL)
    {
        errno = ERANGE;
        return -1;
    }
    _path[pIterator + 1] = '\0';
    // copy file name to corresponding xpath buffer
    if (strncpy(_file_name, _full_path + pIterator + 1, feIterator - pIterator - 1) == NULL)
    {
        errno = ERANGE;
        return -1;
    }

    _file_name[feIterator - pIterator - 1] = '\0';
    return 0;
}
