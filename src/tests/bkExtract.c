#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "bk.h"
#include "bkExtract.h"
#include "bkPath.h"

const unsigned posixFileDefaults = 33188; /* octal 100644 */
const unsigned posixDirDefaults = 16877; /* octal 40711 */

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
            return -3;
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
* !! am i overwriting files?
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
            return -10;
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
        return -10;
    
    return pointerToIt->file.size;
}
