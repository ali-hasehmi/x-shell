#ifndef XPATH_H
#define XPATH_H

#define XP_FEXT_LEN 32                                             // File Extension Buffer length
#define XP_FNAME_LEN 128                                           // File Name Buffer length
#define XP_PATH_LEN 128                                            // Path buffer length
#define XP_FULLPATH_LEN (XP_FNAME_LEN + XP_PATH_LEN + XP_FEXT_LEN) // Full Path buffer length

typedef struct xpath
{
    char p_fext[XP_FEXT_LEN];          // File Extension
    char p_fname[XP_FNAME_LEN];        // File Name
    char p_path[XP_PATH_LEN];          // Path To File
    char p_full_path[XP_FULLPATH_LEN]; // Full Path(Path+FileName+FileExtension)
} xpath_t;

// create a xpath_t object from a string contains full path to a file
int xpcreate(const char *_full_path, xpath_t *_path);

/*
    extract file name and path from a full path
    and set the result in _path and _file_name argument
*/
static int xtrct(const char *_full_path, char *_path, char *_file_name, char *_file_extension);

/*
    print xpath object
*/
int printxpath(const xpath_t *_path);

/*
    import new _file_name and new _path to _full_path
*/
static int import(const char *_file_extension,const char *_file_name, const char *_path, char *_full_path);

/*
    import _new_file_name and _new_path to _path object
    passing any char * argument as null means use previous value
*/
int xpimport(xpath_t *_path, const char *_new_path, const char *_new_file_name, const char *_new_file_extension);

#endif // XPATH_H