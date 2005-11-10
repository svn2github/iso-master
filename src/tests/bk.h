/*******************************************************************************
* bk.h
* global #defines and definitions for structures
* */

#ifndef bk_h
#define bk_h

#include <stdbool.h>
#include <stdlib.h>

/* number of logical sectors in system area (in practice always 16) */
#define NLS_SYSTEM_AREA 16
/* number of bytes in a logical block (in practice always 2048) */
#define NBYTES_LOGICAL_BLOCK 2048

/* can be |ed */
#define FNTYPE_9660 1
#define FNTYPE_ROCKRIDGE 2
#define FNTYPE_JOLIET 4

/* long note on maximum file/directory name lengths:
* max 128 (joliet)
*     + 2 separator1 (9660, just in case)
*     + 2 separator2 (9660, just in case)
*     + 10 version (9660, just in case)
*     = 142 bytes (71 characters)
* rockridge allows unlimited file name lengths but i would need to have the
* 'continue' su entry implemented. doubt it will ever happen
*
* the 71 maximum is for reading the record,
* i will want to store the 64 characters + 1 for '\0' (the rest is nonsense)
*
* max filename length on the filesystem is 255 bytes on almost every kind of
* file system. reiserfs is an exception in that it supports a max of 255 
* characters (4032 bytes)
* i will want to add a '\0' at the end */
#define NCHARS_FILE_ID_MAX_READ 71
#define NCHARS_FILE_ID_MAX 65
#define NCHARS_FILE_ID_FS_MAX 256

/*******************************************************************************
* VolInfo
* information about a volume (one image)
* strings are '\0' terminated */
typedef struct
{
    /* bk use */
    unsigned filenameTypes;
    off_t pRootDrOffset; /* primary (9660 and maybe rockridge) */
    off_t sRootDrOffset; /* secondary (joliet), 0 if does not exist */
    //!! boot record
    /* maybe one day record LEN_SKP from SP */
    
    /* public use */
    char publisher[129];
    char dataPreparer[129];
    time_t creationTime;
    
} VolInfo;

/*******************************************************************************
* Dir
* information about a directory and it's contents */
typedef struct
{
    char name[NCHARS_FILE_ID_MAX]; /* '\0' terminated */
    unsigned posixFileMode;
    struct DirLL* directories;
    struct FileLL* files;
    
} Dir;

/*******************************************************************************
* DirLL
* list of directories */
typedef struct DirLL
{
    Dir dir;
    struct DirLL* next;
    
} DirLL;

/*******************************************************************************
* File
* information about a file, whether on the image or on the filesystem */
typedef struct
{
    char name[NCHARS_FILE_ID_MAX]; /* '\0' terminated */
    unsigned posixFileMode;
    unsigned size; /* in bytes */
    bool onImage;
    unsigned position; /* if on image, in bytes */
    char* pathAndName; /* if on filesystem, full path + filename
                       * is to be freed by whenever the File is freed */
    
} File;

/*******************************************************************************
* FileLL
* list of files */
typedef struct FileLL
{
    File file;
    struct FileLL* next;
    
} FileLL;

/*******************************************************************************
* Path
* full path of a directory on the image */
typedef struct
{
    unsigned numDirs;
    char** dirs;
    
} Path;

/*******************************************************************************
* FilePath
* full path of a file on the image */
typedef struct
{
    Path path;
    char filename[NCHARS_FILE_ID_FS_MAX]; /* '\0' terminated */
    
} FilePath;

#endif
