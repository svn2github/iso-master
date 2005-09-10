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
    char* pathAndName; /* if on filesystem, full path + filename
                       * is to be freed by whenever the File is freed */
    
} File;

typedef struct FileLL
{
    File file;
    struct FileLL* next;
    
} FileLL;

typedef struct
{
    unsigned numDirs;
    char** dirs;
    
} Path;

typedef struct
{
    Path path;
    char filename[256];
    
} FilePath;

int addFile(Dir* tree, char* srcPathAndName, Path* destDir);
int deleteDir(Dir* tree, Path* srcDir);
int deleteFile(Dir* tree, FilePath* pathAndName);
bool dirDrFollows(int image);
int extractDir(int image, Dir* tree, Path* srcDir, char* destDir,
                                                        bool keepPermissions);
int extractFile(int image, Dir* tree, FilePath* pathAndName, char* destDir,
                                                        bool keepPermissions);
void freePath(Path* path);
bool haveNextRecordInSector(int image);
int makeLongerPath(Path* origPath, char* newDir, Path** newPath);
void oops(char* msg);
int readDir(int image, Dir* dir, int filenameType, bool readPosix);
int readDir9660(int image, Dir* dir, unsigned size, int filenameType, bool readPosix);
int readFileInfo(int image, File* file, int filenameType, bool readPosix);
unsigned char readNextRecordLen(int image);
int readPosixInfo(int image, unsigned* posixFileMode, int lenSU);
int readRockridgeFilename(int image, char* dest, int lenSU);
void removeCrapFromFilename(char* src, char* dest, int length);
int skipDR(int image);
void showDir(Dir* dir, int level);

#endif
