#ifndef _testread2_h
#define _testread2_h

#include <stdbool.h>

#define NLS_SYSTEM_AREA 16
#define NBYTES_LOGICAL_BLOCK 2048

#define FNTYPE_9660 0
#define FNTYPE_ROCKRIDGE 1
#define FNTYPE_JOLIET 2

/* note on maximum file/directory name lengths:
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

/* all contents of this are defined in posix 5.6.1
* now where could i get a copy of that.. */
//~ typedef struct
//~ {
    //~ unsigned fileMode;
    //~ unsigned numLinks;
    //~ unsigned uid;
    //~ unsigned gid;
    //~ unsigned serialNum; // ?? was this removed from the final standard ??
    
//~ } PxInfo;

typedef struct
{
    char name[NCHARS_FILE_ID_MAX];
    unsigned posixFileMode;
    struct DirLL* directories;
    struct FileLL* files;
    
} Dir;

typedef struct DirLL
{
    Dir dir;
    struct DirLL* next;
    
} DirLL;

typedef struct
{
    char name[NCHARS_FILE_ID_MAX];
    unsigned posixFileMode;
    bool onImage;
    unsigned position; /* if on image, in bytes */
    unsigned size; /* if on image, in bytes */
    char pathAndName[NCHARS_FILE_ID_FS_MAX]; /* if on filesystem, full path + filename */
    
} File;

typedef struct FileLL
{
    File file;
    struct FileLL* next;
    
} FileLL;

bool haveNextRecordInSector(int image);
void oops(char* msg);
int readDir(int image, Dir* dir, int filenameType, bool readPosix);
int readDir9660(int image, Dir* dir, unsigned size, int filenameType, bool readPosix);
int readFileInfo(int image, File* file, int filenameType, bool readPosix);
unsigned char readNextRecordLen(int image);
int readPosixInfo(int image, unsigned* posixFileMode, int lenSU);
int readRockridgeFilename(int image, char* dest, int lenSU);
void removeCrapFromFilename(char* src, char* dest, int length);
int skipDR(int image);

#endif
