#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "testread2.h"
#include "read7x.h"
#include "vd.h"

const unsigned posixFileDefaults = 33188; /* octal 100644 */
const unsigned posixDirDefaults = 16877; /* octal 40711 */

int main(int argc, char** argv)
{
    int image;
    VdSet vdset;
    int rc;
    
    Dir tree;
    FilePath filePath;
    Path srcDir;
    Path dirPath;
    char* dest; /* destination directory */
    char* fileToAdd;
    char* dirToAdd;
    
    /* open image file for reading */
    image = open(argv[1], O_RDONLY);
    if(image == -1)
        oops("unable to open image\n");
    
    /* skip system area */
    lseek(image, NLS_SYSTEM_AREA * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    
    /* volume descriptor set */
    rc = readVDSet(image, &vdset);
    if(rc <= 0)
        oops("problem reading vd set");
    
    printf("lb size: %d\n", vdset.pvd.lbSize);
    
    printf("volume space size: %u\n", vdset.pvd.volSpaceSize);
    
    printf("human-readable volume size: %dB, %dMB, %dMiB\n", vdset.pvd.lbSize * vdset.pvd.volSpaceSize,
                                                             vdset.pvd.lbSize * vdset.pvd.volSpaceSize / 1024000,
                                                             vdset.pvd.lbSize * vdset.pvd.volSpaceSize / 1048576);
    printf("pathtable size: %d\n", vdset.pvd.pathTableSize);
    
    printf("publ: \'%s\'\n", vdset.pvd.publId);
    
    printf("dprp: \'%s\'\n", vdset.pvd.dataPrepId);
    
    printf("L path table: %d\n", vdset.pvd.locTypeLPathTable);
    
    printf("M path table: %d\n", vdset.pvd.locTypeMPathTable);
    
    printf("root extent at: %d\n", vdset.pvd.rootDR.locExtent);
    
    printf("data length: %d\n", vdset.pvd.rootDR.dataLength);
    
    //printf("joliet type: %d\n", svdGetJolietType(&(vdset.svd)));
    printf("joliet root extent at: %d\n", vdset.svd.rootDR.locExtent);
    
    lseek(image, vdset.pvd.rootDROffset, SEEK_SET);
    tree.directories = NULL;
    tree.files = NULL;
    rc = readDir(image, &tree, FNTYPE_ROCKRIDGE, true);
    printf("readDir ended with %d\n", rc);
    
    //showDir(&tree, 0);
    
    filePath.path.numDirs = 2;
    filePath.path.dirs = malloc(sizeof(char*) * filePath.path.numDirs);
    filePath.path.dirs[0] = malloc(strlen("isolinux") + 1);
    strcpy(filePath.path.dirs[0], "isolinux");
    filePath.path.dirs[1] = malloc(strlen("sbootmgr") + 1);
    strcpy(filePath.path.dirs[1], "sbootmgr");
    strcpy(filePath.filename, "README.TXT");
    
    dirPath.numDirs = 1;
    dirPath.dirs = malloc(sizeof(char*) * dirPath.numDirs);
    dirPath.dirs[0] = malloc(strlen("isolinux") + 1);
    strcpy(dirPath.dirs[0], "isolinux");
    
    srcDir.numDirs = 1;
    srcDir.dirs = malloc(sizeof(char*) * srcDir.numDirs);
    srcDir.dirs[0] = malloc(strlen("kernels" + 1));
    strcpy(srcDir.dirs[0], "kernels");
    
    dest = malloc(strlen("/home/andrei/prog/isomaster/src/tests/") + 1);
    strcpy(dest, "/home/andrei/prog/isomaster/src/tests/");
    
    fileToAdd = malloc(strlen("/home/andrei/prog/isomaster/src/tests/read7x.o") + 1);
    strcpy(fileToAdd, "/home/andrei/prog/isomaster/src/tests/read7x.o");
    
    dirToAdd = malloc(strlen("/home/andrei/prog/isomaster/src/tests/") + 1);
    strcpy(dirToAdd, "/home/andrei/prog/isomaster/src/tests/");
    
    //deleteFile(&tree, &filePath);
    //printf("\n--------------------\n\n");
    
    //deleteDir(&tree, &dirPath);
    //printf("\n--------------------\n\n");
    
    //rc = extractFile(image, &tree, &filePath, dest, true);
    //if(rc <= 0)
    //    oops("problem extracting file");
    
    //rc = extractDir(image, &tree, &srcDir, dest, true);
    //if(rc <= 0)
    //    oops("problem extracting dir");
    
    //rc = addFile(&tree, fileToAdd, &dirPath);
    //if(rc <= 0)
    //    oops("problem adding file");
    
    rc = addDir(&tree, dirToAdd, &dirPath);
    if(rc <= 0)
        oops("problem adding file");
    
    //showDir(&tree, 0);
    
    close(image);
    if(image == -1)
        oops("faled to close image");
    
    return 0;
}

/*
* maybe should make sure tree is not modified if cannot opendir()
*/
int addDir(Dir* tree, char* srcPath, Path* destDir)
{
    /* to add dir to tree */
    char srcDirName[256];
    
    /* to read contents of a dir on fs */
    DIR* srcDir;
    struct dirent* dirEnt;
    struct stat anEntry;
    
    /* for children */
    Path* newDestDir;
    int newSrcPathLen; /* length of new path (including trailing '/' but not filename) */
    char* newSrcPathAndName; /* both for child dirs and child files */
    
    // find destdir and get name of last directory from srcPath
    
    // add dir to tree (with null lists)
    
    // allocate newSrcPathAndName (+255 +possible trailing '/') and copy srcPath into it
    
    // makeLongerPath newDestDir
    
    /* ADD contents of directory */
    srcDir = opendir(srcPath);
    if(srcDir == NULL)
        return -1;
    
    while( (dirEnt = readdir(srcDir)) != NULL )
    {
        if( strcmp(dirEnt->d_name, ".") == 0 || strcmp(dirEnt->d_name, "..") == 0 )
            printf("skipped '%s'\n", dirEnt->d_name);
        else
        {
            // append name to newSrcPathAndName
            
            // if dir, append '/' and addDir()
            // if regular file, addFile()
        }
    }
    /* END ADD contents of directory */
    
    // free local memory
    
    return 1;
}

/*
* file gets appended to the end of the list (screw the 9660 sorting, it's stupid)
* will only add a regular file (symblic links are followed, see stat(2))
*/
int addFile(Dir* tree, char* srcPathAndName, Path* destDir)
{
    int count;
    int rc;
    FileLL** lastFile;
    char filename[256];
    struct stat statStruct;
    
    /* vars to find the dir in the tree */
    Dir* destDirInTree;
    DirLL* searchDir;
    bool dirFound;
    
    rc = getFilenameFromPath(srcPathAndName, filename);
    if(rc <= 0)
        return rc;
    
    if(strlen(filename) > NCHARS_FILE_ID_MAX - 1)
        return -3;
    
    /* FIND dir to add to */
    destDirInTree = tree;
    for(count = 0; count < destDir->numDirs; count++)
    /* each directory in the path */
    {
        searchDir = destDirInTree->directories;
        dirFound = false;
        while(searchDir != NULL && !dirFound)
        /* find the directory */
        {
            if(strcmp(searchDir->dir.name, destDir->dirs[count]) == 0)
            {
                dirFound = true;
                destDirInTree = &(searchDir->dir);
            }
            else
                searchDir = searchDir->next;
        }
        if(!dirFound)
            return -1;
    }
    /* END FIND dir to add to */
    
    /* FIND last pointer in file list */
    //!! if not sorting, might as well append to beginnig of list
    lastFile = &(destDirInTree->files);
    while(*lastFile != NULL)
    {
        lastFile = &((*lastFile)->next);
    }
    /* END FIND last pointer in file list */
    
    /* ADD file */
    *lastFile = malloc(sizeof(FileLL));
    if(*lastFile == NULL)
        return -2;
    
    (*lastFile)->next = NULL;
    
    strcpy((*lastFile)->file.name, filename);
    
    rc = stat(srcPathAndName, &statStruct);
    if(rc == -1)
        return -4;
    
    if( !(statStruct.st_mode & S_IFREG) )
    /* not a regular file */
        return -5;
    
    (*lastFile)->file.posixFileMode = statStruct.st_mode;
    
    (*lastFile)->file.size = statStruct.st_size;
    
    (*lastFile)->file.onImage = false;
    
    (*lastFile)->file.position = 0;
    
    (*lastFile)->file.pathAndName = malloc(strlen(srcPathAndName) + 1);
    strcpy((*lastFile)->file.pathAndName, srcPathAndName);
    /* END ADD file */
    
    return 1;
}

int deleteDir(Dir* tree, Path* srcDir)
{
    int count;
    int rc;
    
    /* vars to find the dir in the tree */
    Dir* srcDirInTree;
    DirLL* searchDir;
    bool dirFound;
    
    /* vars to delete files */
    FileLL* currentFile;
    FileLL* nextFile;
    
    /* vars to delete subdirectories */
    DirLL* currentDir;
    Path* newSrcDir;
    
    /* vars to delete the directory */
    Dir* parentDirInTree;
    bool parentDirFound;
    DirLL** parentDirLL;
    DirLL* parentDirNextLL;
    
    /* FIND dir to know what the contents are */
    srcDirInTree = tree;
    for(count = 0; count < srcDir->numDirs; count++)
    /* each directory in the path */
    {
        searchDir = srcDirInTree->directories;
        dirFound = false;
        while(searchDir != NULL && !dirFound)
        /* find the directory */
        {
            if(strcmp(searchDir->dir.name, srcDir->dirs[count]) == 0)
            {
                dirFound = true;
                srcDirInTree = &(searchDir->dir);
            }
            else
                searchDir = searchDir->next;
        }
        if(!dirFound)
            oops("deleteDir(): directory not found in tree");
    }
    /* END FIND dir to know what the contents are */
    
    /* DELETE all files */
    currentFile = srcDirInTree->files;
    while(currentFile != NULL)
    {
        nextFile = currentFile->next;
        
        if(!currentFile->file.onImage)
            free(currentFile->file.pathAndName);
        
        free(currentFile);
        
        currentFile = nextFile;
    }
    /* END DELETE all files */
    
    /* DELETE all directories */
    currentDir = srcDirInTree->directories;
    while(currentDir != NULL)
    {
        rc = makeLongerPath(srcDir, currentDir->dir.name, &newSrcDir);
        if(rc <= 0)
            return rc;
        
        rc = deleteDir(tree, newSrcDir);
        if(rc <= 0)
            return rc;
        
        freePath(newSrcDir);
        
        currentDir = currentDir->next;
    }
    /* END DELETE all directories */
    
    /* GET A pointer to the parent dir */
    parentDirInTree = tree;
    for(count = 0; count < srcDir->numDirs - 1; count++)
    /* each directory in the path except the last one */
    {
        searchDir = parentDirInTree->directories;
        parentDirFound = false;
        while(searchDir != NULL && !parentDirFound)
        /* find the directory, last one found will be the parent */
        {
            if(strcmp(searchDir->dir.name, srcDir->dirs[count]) == 0)
            {
                parentDirFound = true;
                parentDirInTree = &(searchDir->dir);
            }
            else
                searchDir = searchDir->next;
        }
        if(!dirFound)
            oops("deleteDir(): directory not found in tree");
    }
    /* END GET A pointer to the parent dir */
    
    /* DELETE self */
    parentDirLL = &(parentDirInTree->directories);
    dirFound = false;
    while(*parentDirLL != NULL && !dirFound)
    {
        if(strcmp( (*parentDirLL)->dir.name, srcDir->dirs[srcDir->numDirs - 1] ) == 0 )
        {
            parentDirNextLL = (*parentDirLL)->next;
            
            free(*parentDirLL);
            
            *parentDirLL = parentDirNextLL;
            
            dirFound = true;
        }
        else
            parentDirLL = &((*parentDirLL)->next);
    }
    if(!dirFound)
    /* should not happen since i already found this dir above */
        oops("deleteDir(): directory not found in tree");
    /* END DELETE self */
    
    return 1;
}

int deleteFile(Dir* tree, FilePath* pathAndName)
{
    Dir* parentDir;
    DirLL* searchDir;
    bool dirFound;
    FileLL** pointerToIt; /* pointer to pointer to the file to delete */
    FileLL* pointerToNext; /* to assign to the pointer pointed to by the pointer above
                           * no i'm not kidding */
    bool fileFound;
    int count;
    
    parentDir = tree;
    for(count = 0; count < pathAndName->path.numDirs; count++)
    /* each directory in the path */
    {
        searchDir = parentDir->directories;
        dirFound = false;
        while(searchDir != NULL && !dirFound)
        /* find the directory */
        {
            if(strcmp(searchDir->dir.name, pathAndName->path.dirs[count]) == 0)
            {
                dirFound = true;
                parentDir = &(searchDir->dir);
            }
            else
                searchDir = searchDir->next;
        }
        if(!dirFound)
            oops("deleteFile(): directory not found in tree");
    }
    
    /* now i have parentDir pointing to the parent directory */
    
    pointerToIt = &(parentDir->files);
    fileFound = false;
    while(*pointerToIt != NULL && !fileFound)
    {
        if(strcmp((*pointerToIt)->file.name, pathAndName->filename) == 0)
        /* delete the node */
        {
            pointerToNext = (*pointerToIt)->next;
            
            if( (*pointerToIt)->file.onImage )
                free( (*pointerToIt)->file.pathAndName );
            
            free(*pointerToIt);
            
            *pointerToIt = pointerToNext;
            
            fileFound = true;
        }
        else
        {
            pointerToIt = &((*pointerToIt)->next);
        }
    }
    if(!fileFound)
        oops("deleteFile(): file not found in tree");
    
    return true;
}

bool dirDrFollows(int image)
{
    unsigned char fileFlags;
    off_t origPos;
    int rc;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    lseek(image, 25, SEEK_CUR);
    
    rc = read711(image, &fileFlags);
    if(rc != 1)
        return rc;
    
    lseek(image, origPos, SEEK_SET);
    
    if((fileFlags >> 1 & 1) == 1)
        return true;
    else
        return false;
}

/*
* don't try to extract root, don't know what will happen
*/
int extractDir(int image, Dir* tree, Path* srcDir, char* destDir,
                                                        bool keepPermissions)
{
    int rc;
    int count;
    
    /* vars to find file location on image */
    Dir* srcDirInTree;
    DirLL* searchDir; /* to find a dir in the tree */
    bool dirFound;
    
    /* vars to create destination dir */
    char* newDestDir;
    unsigned destDirPerms;
    
    /* vars to extract files */
    FilePath filePath;
    FileLL* currentFile;
    
    /* vars to extract subdirectories */
    DirLL* currentDir;
    Path* newSrcDir;
    
    /* FIND parent dir to know what the contents are */
    srcDirInTree = tree;
    for(count = 0; count < srcDir->numDirs; count++)
    /* each directory in the path */
    {
        searchDir = srcDirInTree->directories;
        dirFound = false;
        while(searchDir != NULL && !dirFound)
        /* find the directory */
        {
            if(strcmp(searchDir->dir.name, srcDir->dirs[count]) == 0)
            {
                dirFound = true;
                srcDirInTree = &(searchDir->dir);
            }
            else
                searchDir = searchDir->next;
        }
        if(!dirFound)
            oops("extractDir(): directory not found in tree");
    }
    /* END FIND parent dir to know what the contents are */
    
    /* CREATE destination dir */
    /* 1 for '/', 1 for '\0' */
    newDestDir = malloc(strlen(destDir) + strlen( (srcDir->dirs)[srcDir->numDirs - 1] ) + 2);
    if(newDestDir == NULL)
        return -2;
    strcpy(newDestDir, destDir);
    strcat(newDestDir, (srcDir->dirs)[srcDir->numDirs - 1]);
    strcat(newDestDir, "/");
    
    if(keepPermissions)
        destDirPerms = srcDirInTree->posixFileMode;
    else
        destDirPerms = posixDirDefaults;
    rc = mkdir(newDestDir, destDirPerms);
    if(rc == -1)
        return -1;
    /* END CREATE destination dir */
    
    /* BEGIN extract each file in directory */
    filePath.path = *srcDir; /* filePath is readonly so pointer sharing is ok here */
    currentFile = srcDirInTree->files;
    while(currentFile != NULL)
    {
        strcpy(filePath.filename, currentFile->file.name);
        
        rc = extractFile(image, tree, &filePath, newDestDir, keepPermissions);
        if(rc < 0) /* returns size of file extracted */
            return rc;
        
        currentFile = currentFile->next;
    }
    /* END extract each file in directory */
    
    /* BEGIN extract each subdirectory */
    currentDir = srcDirInTree->directories;
    while(currentDir != NULL)
    {
        rc = makeLongerPath(srcDir, currentDir->dir.name, &newSrcDir);
        if(rc <= 0)
            return rc;
        
        rc = extractDir(image, tree, newSrcDir, newDestDir, keepPermissions);
        if(rc <= 0)
            return rc;
        
        freePath(newSrcDir);
        
        currentDir = currentDir->next;
    }
    /* END extract each subdirectory */
    
    free(newDestDir);
    
    return 1;
}

/*
* destDir must have trailing slash
* read/write loop is waaay to slow when doing 1 byte at a time so changed it to
*  do 100K at a time instead
*/
int extractFile(int image, Dir* tree, FilePath* pathAndName, char* destDir,
                                                        bool keepPermissions)
{
    /* vars to find file location on image */
    Dir* parentDir;
    DirLL* searchDir;
    bool dirFound;
    FileLL* pointerToIt; /* pointer to the node with file to read */
    bool fileFound;
    
    /* vars to create destination file */
    char* destPathAndName;
    unsigned destFilePerms;
    int destFile; /* returned by open() */
    
    char block[102400]; /* 100K */
    int numBlocks;
    int sizeLastBlock;
    
    int count;
    int rc;
    
    parentDir = tree;
    for(count = 0; count < pathAndName->path.numDirs; count++)
    /* each directory in the path */
    {
        searchDir = parentDir->directories;
        dirFound = false;
        while(searchDir != NULL && !dirFound)
        /* find the directory */
        {
            if(strcmp(searchDir->dir.name, pathAndName->path.dirs[count]) == 0)
            {
                dirFound = true;
                parentDir = &(searchDir->dir);
            }
            else
                searchDir = searchDir->next;
        }
        if(!dirFound)
            oops("extractFile(): directory not found in tree");
    }
    
    /* now i have parentDir pointing to the parent directory */
    
    pointerToIt = parentDir->files;
    fileFound = false;
    while(pointerToIt != NULL && !fileFound)
    /* find the file in parentDir */
    {
        if(strcmp(pointerToIt->file.name, pathAndName->filename) == 0)
        /* this is the file */
        {
            if(!pointerToIt->file.onImage)
            //!! maybe just make a copy of the file here
                return -1;
            
            fileFound = true;
            
            destPathAndName = malloc(strlen(destDir) + strlen(pathAndName->filename) + 1);
            if(destPathAndName == NULL)
                return -2;
            strcpy(destPathAndName, destDir);
            strcat(destPathAndName, pathAndName->filename);
            
            /* WRITE file */
            if(keepPermissions)
                destFilePerms = pointerToIt->file.posixFileMode;
            else
                destFilePerms = posixFileDefaults;
            
            destFile = open(destPathAndName, O_WRONLY | O_CREAT | O_TRUNC, destFilePerms);
            if(destFile == -1)
              return -3;
            free(destPathAndName);
            
            lseek(image, pointerToIt->file.position, SEEK_SET);
            
            numBlocks = pointerToIt->file.size / 102400;
            sizeLastBlock = pointerToIt->file.size % 102400;
            
            for(count = 0; count < numBlocks; count++)
            {
                rc = read(image, block, 102400);
                if(rc != 102400)
                    return -3;
                rc = write(destFile, block, 102400);
                if(rc != 102400)
                    return -4;
            }
            
            rc = read(image, block, sizeLastBlock);
            if(rc != sizeLastBlock)
                    return -3;
            rc = write(destFile, block, sizeLastBlock);
            if(rc != sizeLastBlock)
                    return -4;
            
            close(destFile);
            if(destFile == -1)
              return -5;
            /* END WRITE file */
        }
        else
        {
            pointerToIt = pointerToIt->next;
        }
    }
    if(!fileFound)
        oops("extractFile(): file not found in tree");
    
    return pointerToIt->file.size;
}

void freePath(Path* path)
{
    int count;
    
    for(count = 0; count < path->numDirs; count++)
        free(path->dirs[count]);
    free(path->dirs);
    free(path);
}

int getFilenameFromPath(char* srcPathAndName, char* filename)
{
    int count;
    int srcLen;
    int indexLastSlash;
    bool found = false;
    int count2;
    
    srcLen = strlen(srcPathAndName);
    
    for(count = 0; count < srcLen; count++)
    {
        if(srcPathAndName[count] == '/')
        {
            indexLastSlash = count;
            found = true;
        }
    }
    if(!found)
        return -1;
    
    if(indexLastSlash == srcLen - 1)
    /* string ended with '/' */
        return -2;
    
    /* loop copies null byte also */
    for(count = indexLastSlash + 1, count2 = 0; count <= srcLen; count++, count2++)
    {
        filename[count2] = srcPathAndName[count];
    }
    
    return 1;
}

/* if the next byte is zero returns false otherwise true
* file position remains unchanged
* returns false on read error */
bool haveNextRecordInSector(int image)
{
    off_t origPos;
    char testByte;
    int rc;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, &testByte, 1);
    if(rc != 1)
        return false;
    
    lseek(image, origPos, SEEK_SET);
    
    return (testByte == 0) ? false : true;
}

int makeLongerPath(Path* origPath, char* newDir, Path** newPath)
{
    int count;
    
    *newPath  = malloc(sizeof(Path));
    if(*newPath == NULL)
        return -1;
    
    (*newPath)->numDirs = origPath->numDirs + 1;
    
    (*newPath)->dirs = malloc(sizeof(char*) * (*newPath)->numDirs);
    if((*newPath)->dirs == NULL)
        return -1;
    
    /* copy original */
    for(count = 0; count < origPath->numDirs; count++)
    {
        (*newPath)->dirs[count] = malloc(strlen((origPath->dirs)[count]) + 1);
        if((*newPath)->dirs[count] == NULL)
            return -1;
        strcpy((*newPath)->dirs[count], (origPath->dirs)[count]);
    }
    
    /* new dir */
    (*newPath)->dirs[count] = malloc(strlen(newDir));
    if((*newPath)->dirs[count] == NULL)
        return -1;
    strcpy((*newPath)->dirs[count], newDir);
    
    return 1;
}

void oops(char* msg)
{
    fflush(NULL);
    fprintf(stderr, "OOPS, %s\n", msg);
    exit(0);
}

/*
* if the root dr (inside vd) is read, it's filename will be ""
* note: directory identifiers do not end with ";1"
*/
int readDir(int image, Dir* dir, int filenameType, bool readPosix)
{
    int rc;
    unsigned char recordLength;
    unsigned locExtent; /* to know where to go before readDir9660() */
    unsigned lenExtent; /* parameter to readDir9660() */
    unsigned char lenFileId9660; /* also len joliet fileid (bytes) */
    int lenSU; /* calculated as recordLength - 33 - lenFileId9660 */
    off_t origPos;
    char rootTestByte;
    bool isRoot;
    
    rc = read(image, &recordLength, 1);
    if(rc != 1)
        return -1;
    
    lseek(image, 1, SEEK_CUR);
    
    rc = read733(image, &locExtent);
    if(rc != 8)
        return -1;
    
    rc = read733(image, &lenExtent);
    if(rc != 8)
        return -1;
    
    lseek(image, 14, SEEK_CUR);
    
    rc = read(image, &lenFileId9660, 1);
    if(rc != 1)
        return -1;
    
    lenSU = recordLength - 33 - lenFileId9660;
    if(lenFileId9660 % 2 == 0)
        lenSU -= 1;
    
    /* FIND out if root */
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, &rootTestByte, 1);
    if(rc != 1)
        return -1;
    
    lseek(image, origPos, SEEK_SET);
    
    if(lenFileId9660 == 1 && rootTestByte == 0x00)
    {
        isRoot = true;
        dir->name[0] = '\0';
    }
    else
        isRoot = false;
    /* END FIND out if root */
    
    if(filenameType == FNTYPE_9660)
    {
        if(!isRoot)
        {
            char nameAsOnDisk[256];
            
            rc = read(image, nameAsOnDisk, lenFileId9660);
            if(rc != lenFileId9660)
                return -1;
            
            strncpy(dir->name, nameAsOnDisk, lenFileId9660);
            
            dir->name[lenFileId9660] = '\0';
            
            if( strlen(dir->name) > NCHARS_FILE_ID_MAX - 1 )
                return -2;
        
            /* padding field */
            if(lenFileId9660 % 2 == 0)
                lseek(image, 1, SEEK_CUR);
        }
    }
    else if(filenameType == FNTYPE_JOLIET)
    {
        if(!isRoot)
        {
            char nameAsOnDisk[256];
            char nameInAscii[256];
            int ucsCount, byteCount;
            
            if(lenFileId9660 % 2 != 0)
                return -3;
            
            rc = read(image, nameAsOnDisk, lenFileId9660);
            if(rc != lenFileId9660)
                return -1;
            
            for(ucsCount = 1, byteCount = 0; ucsCount < lenFileId9660;
                ucsCount += 2, byteCount += 1)
            {
                nameInAscii[byteCount] = nameAsOnDisk[ucsCount];
            }
            nameInAscii[byteCount] = '\0';
            
            if( strlen(nameInAscii) > NCHARS_FILE_ID_MAX - 1 )
            //!! maybe just truncate the name instead
                return -2;
            
            strcpy(dir->name, nameInAscii);
            
            /* padding field */
            if(lenFileId9660 % 2 == 0)
                lseek(image, 1, SEEK_CUR);
        }
    }
    else if(filenameType == FNTYPE_ROCKRIDGE)
    {
        /* skip 9660 filename */
        lseek(image, lenFileId9660, SEEK_CUR);
        /* skip padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);
        
        if(!isRoot)
        {
            rc = readRockridgeFilename(image, dir->name, lenSU);
            if(rc < 0)
                return rc;
        }
    }
    else
        oops("readDir(): dude, what filename type did you ask for?");
    
    if(readPosix)
    {
        if(isRoot)
        {
            unsigned char realRootRecordLen;
            
            origPos = lseek(image, 0, SEEK_CUR);
            
            /* go to real root record */
            lseek(image, locExtent * NBYTES_LOGICAL_BLOCK, SEEK_SET);
            
            /* read record length */
            read(image, &realRootRecordLen, 1);
            if(rc != 1)
                return -1;
            
            /*go to sys use fields */
            lseek(image, 33, SEEK_CUR);
            
            readPosixInfo(image, &(dir->posixFileMode), realRootRecordLen - 34);
            
            /* return */
            lseek(image, origPos, SEEK_SET);
        }
        else
        {
            readPosixInfo(image, &(dir->posixFileMode), lenSU);
        }
    }
    else
    {
        /* this is good for root also */
        dir->posixFileMode = posixDirDefaults;
    }
    
    lseek(image, lenSU, SEEK_CUR);

    origPos = lseek(image, 0, SEEK_CUR);
    
    lseek(image, locExtent * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    
    rc = readDir9660(image, dir, lenExtent, filenameType, readPosix);
    if(rc < 0)
        return rc;
    
    lseek(image, origPos, SEEK_SET);
    
    return recordLength;
}

/*
* size is number of bytes
* hope you love pointers
*/
int readDir9660(int image, Dir* dir, unsigned size, int filenameType, bool readPosix)
{
    int rc;
    int bytesRead = 0;
    int childrenBytesRead;
    DirLL** nextDir; /* pointer to pointer to modify pointer :) */
    FileLL** nextFile; /* ditto */
    
    /* skip self and parent */
    bytesRead += skipDR(image);
    bytesRead += skipDR(image);
    
    nextDir = &(dir->directories);
    nextFile = &(dir->files);
    childrenBytesRead = 0;
    while(childrenBytesRead + bytesRead < size)
    {
        if(haveNextRecordInSector(image))
        /* read it */
        {
            if( dirDrFollows(image) )
            /* directory descriptor record */
            {
                int recordLength;
                
                *nextDir = malloc(sizeof(DirLL));
                if(*nextDir == NULL)
                    return -2;
                
                recordLength = readDir(image, &((*nextDir)->dir), filenameType, readPosix);
                if(recordLength < 0)
                    return recordLength;
                
                childrenBytesRead += recordLength;
                
                nextDir = &((*nextDir)->next);
                *nextDir = NULL;
            }
            else
            /* file descriptor record */
            {
                int recordLength;
                
                *nextFile = malloc(sizeof(FileLL));
                if(*nextFile == NULL)
                    return -2;
                
                recordLength = readFileInfo(image, &((*nextFile)->file), filenameType, readPosix);
                if(recordLength < 0)
                    return recordLength;
                
                childrenBytesRead += recordLength;
                
                nextFile = &((*nextFile)->next);
                *nextFile = NULL;
            }
        }
        else
        /* read zeroes until get to next record (that would be in the next
        *  sector btw) or get to the end of data (dir->self.dataLength) */
        {
            char testByte;
            off_t origPos;
            
            do
            {
                origPos = lseek(image, 0, SEEK_CUR);
                
                rc = read(image, &testByte, 1);
                if(rc != 1)
                    return -1;
                
                if(testByte != 0)
                {
                    lseek(image, origPos, SEEK_SET);
                    break;
                }
                
                childrenBytesRead += 1;
                
            } while (childrenBytesRead + bytesRead < size);
        }
    }
    
    return bytesRead;
}

int readFileInfo(int image, File* file, int filenameType, bool readPosix)
{
    int rc;
    unsigned char recordLength;
    unsigned locExtent; /* block num where the file is */
    unsigned lenExtent; /* in bytes */
    unsigned char lenFileId9660; /* also len joliet fileid (bytes) */
    int lenSU; /* calculated as recordLength - 33 - lenFileId9660 */
    
    rc = read(image, &recordLength, 1);
    if(rc != 1)
        return -1;
    
    lseek(image, 1, SEEK_CUR);
    
    rc = read733(image, &locExtent);
    if(rc != 8)
        return -1;
    
    rc = read733(image, &lenExtent);
    if(rc != 8)
        return -1;
    
    lseek(image, 14, SEEK_CUR);
    
    rc = read(image, &lenFileId9660, 1);
    if(rc != 1)
        return -1;
    
    lenSU = recordLength - 33 - lenFileId9660;
    if(lenFileId9660 % 2 == 0)
        lenSU -= 1;
    
    if(filenameType == FNTYPE_9660)
    {
        char nameAsOnDisk[256];
        
        rc = read(image, nameAsOnDisk, lenFileId9660);
        if(rc != lenFileId9660)
            return -1;
        
        removeCrapFromFilename(nameAsOnDisk, lenFileId9660);
        
        if( strlen(nameAsOnDisk) > NCHARS_FILE_ID_MAX - 1 )
            return -2;
        
        strcpy(file->name, nameAsOnDisk);
        
        /* padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);
    }
    else if(filenameType == FNTYPE_JOLIET)
    {
        char nameAsOnDisk[256];
        char nameInAscii[256];
        int ucsCount, byteCount;
        
        if(lenFileId9660 % 2 != 0)
            return -3;
        
        rc = read(image, nameAsOnDisk, lenFileId9660);
        if(rc != lenFileId9660)
            return -1;
        
        for(ucsCount = 1, byteCount = 0; ucsCount < lenFileId9660;
            ucsCount += 2, byteCount += 1)
        {
            nameInAscii[byteCount] = nameAsOnDisk[ucsCount];
        }
        
        removeCrapFromFilename(nameInAscii, lenFileId9660 / 2);
        
        if( strlen(nameInAscii) > NCHARS_FILE_ID_MAX - 1 )
            return -2;
        
        strcpy(file->name, nameAsOnDisk);
        
        /* padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);
    }
    else if(filenameType == FNTYPE_ROCKRIDGE)
    {
        /* skip 9660 filename */
        lseek(image, lenFileId9660, SEEK_CUR);
        /* skip padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);

        rc = readRockridgeFilename(image, file->name, lenSU);
        if(rc < 0)
            return rc;
    }
    else
        oops("readDir(): dude, what filename type did you ask for?");
    
    if(readPosix)
    {
        readPosixInfo(image, &(file->posixFileMode), lenSU);
    }
    else
    {
        file->posixFileMode = posixFileDefaults;
    }
    
    lseek(image, lenSU, SEEK_CUR);
    
    file->onImage = true;
    file->position = locExtent * NBYTES_LOGICAL_BLOCK;
    file->size = lenExtent;
    file->pathAndName = NULL;
    
    return recordLength;
}

//~ unsigned char readNextRecordLen(int image)
//~ {
    //~ unsigned char rc;
    //~ unsigned char length;
    //~ off_t origPos;
    
    //~ origPos = lseek(image, 0, SEEK_CUR);
    
    //~ rc = read711(image, &length);
    //~ if(rc != 1)
    //~ // !! i don't like this int->char cast
        //~ return rc;
    
    //~ lseek(image, origPos, SEEK_SET);
    
    //~ return length;
//~ }

int readPosixInfo(int image, unsigned* posixFileMode, int lenSU)
{
    off_t origPos;
    unsigned char suFields[256];
    int rc;
    bool foundPosix;
    int count;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, suFields, lenSU);
    if(rc != lenSU)
        return -1;
    
    lseek(image, origPos, SEEK_SET);
    
    count = 0;
    foundPosix = false;
    while(count < lenSU && !foundPosix)
    {
        if(suFields[count] == 'P' && suFields[count + 1] == 'X')
        {
            read733FromCharArray(suFields + count + 4, posixFileMode);
            
            /* not interested in anything else from this field */
            
            foundPosix = true;
        }
        else
        /* skip su record */
        {
            count += suFields[count + 2];
        }
    }
    
    return 1;
}

/*
* leaves the file pointer where it was
*/
int readRockridgeFilename(int image, char* dest, int lenSU)
{
    off_t origPos;
    unsigned char suFields[256];
    char nameAsRead[256];
    int rc;
    int count;
    int lengthAsRead;
    bool foundName;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, suFields, lenSU);
    if(rc != lenSU)
        return -1;
    
    lseek(image, origPos, SEEK_SET);
    
    count = 0;
    foundName = false;
    while(count < lenSU && !foundName)
    {
        if(suFields[count] == 'N' && suFields[count + 1] == 'M')
        {
            lengthAsRead = suFields[count + 2] - 5;
            
            strncpy(nameAsRead, suFields + count + 5, lengthAsRead);
            
            if(lengthAsRead > NCHARS_FILE_ID_MAX - 1)
                return -2;
            
            strncpy(dest, nameAsRead, lengthAsRead);
            
            foundName = true;
        }
        else
        /* skip su record */
        {
            count += suFields[count + 2];
        }
    }
    
    return 1;
}

/*
* filenames as read from 9660 Sometimes end with ;1 (terminator+version num)
* this removes the useless ending and terminates the destination with a '\0'
*/
void removeCrapFromFilename(char* filename, int length)
{
    int count;
    bool stop = false;
    
    for(count = 0; count < NCHARS_FILE_ID_MAX_READ && count < length && !stop; count++)
    {
        if(filename[count] == ';')
        {
            filename[count] = '\0';
            stop = true;
        }
    }
    
    /* if did not get a ';' terminate string anyway */
    filename[count - 1] = '\0';
}

int skipDR(int image)
{
    unsigned char dRLen;
    int rc;
    
    rc = read711(image, &dRLen);
    if(rc != 1)
        return rc;
    
    lseek(image, dRLen - 1, SEEK_CUR);
    
    return dRLen;
}

void showDir(Dir* dir, int level)
{
    DirLL* dirNode;
    FileLL* fileNode;
    int count;
    
    dirNode = dir->directories;
    
    while(dirNode != NULL)
    {
        for(count = 0; count < level; count++)
            printf("  ");
        printf("%s\n", dirNode->dir.name);
        
        showDir(&(dirNode->dir), level + 1);
        
        dirNode = dirNode->next;
    }
    
    fileNode = dir->files;
    
    while(fileNode != NULL)
    {
        for(count = 0; count < level; count++)
            printf("  ");
        printf("%s - %d bytes - %o - ", fileNode->file.name, fileNode->file.size, fileNode->file.posixFileMode);
        if(fileNode->file.onImage)
            printf("on image @%08X\n", fileNode->file.position);
        else
            printf("on disk: \'%s\'\n", fileNode->file.pathAndName);
        fileNode = fileNode->next;
    }
}
