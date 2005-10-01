#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "bk.h"

/*
* when working on this make sure tree is not modified if cannot opendir()
*/
int addDir(Dir* tree, char* srcPath, Path* destDir)
{
    int count;
    int rc;
    
    /* vars to add dir to tree */
    char srcDirName[256];
    Dir* destDirInTree;
    DirLL* searchDir;
    bool dirFound;
    DirLL** lastDir;
    struct stat statStruct;
    
    /* vars to read contents of a dir on fs */
    DIR* srcDir;
    struct dirent* dirEnt;
    struct stat anEntry;
    
    /* vars for children */
    Path* newDestDir;
    int newSrcPathLen; /* length of new path (including trailing '/' but not filename) */
    char* newSrcPathAndName; /* both for child dirs and child files */
    
    if(srcPath[strlen(srcPath - 1)] != '/')
    /* must have trailing slash */
        return -9;
    
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
    
    /* get the name of the directory to be added */
    rc = getLastDirFromString(srcPath, srcDirName);
    if(rc <= 0)
        return rc;
    
    // check length of dir
    
    /* find last dir in list */
    //!! if not sorting, might as well append to beginnig of list
    lastDir = &(destDirInTree->directories);
    while(*lastDir != NULL)
        lastDir = &((*lastDir)->next);
    
    /* ADD directory to tree */
    rc = stat(srcPath, &statStruct);
    if(rc == -1)
        return -4;
    
    if( !(statStruct.st_mode & S_IFDIR) )
    /* not a directory */
        return -5;
    
    *lastDir = malloc(sizeof(DirLL));
    if(*lastDir == NULL)
        return -3;
    
    (*lastDir)->next = NULL;
    
    strcpy((*lastDir)->dir.name, srcDirName);
    
    (*lastDir)->dir.posixFileMode = statStruct.st_mode;
    
    (*lastDir)->dir.directories = NULL;
    (*lastDir)->dir.files = NULL;
    /* END ADD directory to tree */
    
    /* remember length of original */
    newSrcPathLen = strlen(srcPath);
    
    /* including the file/dir name and the trailing '/' and the '\0' */
    newSrcPathAndName = malloc(newSrcPathLen + 257);
    if(newSrcPathAndName == NULL)
        return -3;
    
    strcpy(newSrcPathAndName, srcPath);
    
    rc = makeLongerPath(destDir, srcDirName, &newDestDir);
    if(rc <= 0)
        return rc;
    
    /* ADD contents of directory */
    srcDir = opendir(srcPath);
    if(srcDir == NULL)
        return -2;
    
    /* it may be possible but in any case very unlikely that readdir() will fail
    * if it does, null is returned (same as end of dir) */
    while( (dirEnt = readdir(srcDir)) != NULL )
    {
        if( strcmp(dirEnt->d_name, ".") != 0 && strcmp(dirEnt->d_name, "..") != 0 )
        /* not "." or ".." */
        {
            /* append file/dir name */
            strcpy(newSrcPathAndName + newSrcPathLen, dirEnt->d_name);
            
            rc = stat(newSrcPathAndName, &anEntry);
            if(rc == -1)
                return -6;
            
            if(anEntry.st_mode & S_IFDIR)
            /* directory */
            {
                strcat(newSrcPathAndName, "/");
                
                addDir(tree, newSrcPathAndName, newDestDir);
            }
            else if(anEntry.st_mode & S_IFREG)
            /* regular file */
            {
                addFile(tree, newSrcPathAndName, newDestDir);
            }
            else
            /* not regular file or directory */
            {
                return -7;
            }
            
        } /* if */
        
    } /* while */
    
    rc = closedir(srcDir);
    if(rc != 0)
    /* exotic error */
        return -8;
    /* END ADD contents of directory */
    
    free(newSrcPathAndName);
    freePath(newDestDir);
    
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
    
    // check length of filename
    
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
