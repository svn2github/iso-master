#include "bk.h"
#include "bkMangle.h"

#include <string.h>

//!! origDir has to be sorted
int mangleDir(Dir* origDir, Dir* newDir, int fileNameType)
{
    DirLL* nextOrigDir;
    char nextOrigDirName[NCHARS_FILE_ID_MAX];
    char prevOrigDirName[NCHARS_FILE_ID_MAX];
    
    FileLL* nextOrigFile;
    char nextOrigFileName[NCHARS_FILE_ID_MAX];
    char nextOrigFileBase[NCHARS_FILE_ID_MAX];
    char nextOrigFileExt[5];
    char prevOrigFileName[NCHARS_FILE_ID_MAX];
    /*char prevOrigFileBase[NCHARS_FILE_ID_MAX];
    char prevOrigFileExt[6];*/
    
    DirLL** nextNewDir;
    FileLL** nextNewFile;
    
    bool takeDirNext; /* what to work on next, a dir or a file? */
    int fileNumber; /* longf~01.txt */
    
    nextOrigFile = origDir->files;
    nextOrigDir = origDir->directories;
    
    nextNewFile = &(newDir->files);
    *nextNewFile = NULL;
    nextNewDir = &(newDir->directories);
    *nextNewDir = NULL;
    
    prevOrigFileName[0] = '\0';
    prevOrigDirName[0] = '\0';
    while( !(nextOrigFile == NULL && nextOrigDir == NULL) )
    /* have a file or directory to convert */
    {
        if(nextOrigFile == NULL)
        /* no files left */
        {
            takeDirNext = true;
            printf("no files left\n");
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName, fileNameType);
            printf("next dir: %s -> '%s'\n", nextOrigDir->dir.name, nextOrigDirName);
        }
        else if(nextOrigDir == NULL)
        /* no directories left */
        {
            takeDirNext = false;
            printf("no dirs left\n");
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType);
            splitFileName(nextOrigFileName, nextOrigFileBase, nextOrigFileExt);
            printf("next file: %s -> '%s'\n", nextOrigFile->file.name, nextOrigFileName);
        }
        else
        /* have both a file and a directory */
        {
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName, fileNameType);
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType);
            splitFileName(nextOrigFileName, nextOrigFileBase, nextOrigFileExt);
            
            printf("next dir: %s -> '%s'\n", nextOrigDir->dir.name, nextOrigDirName);
            printf("next file: %s -> '%s'\n", nextOrigFile->file.name, nextOrigFileName);
            /* find the lesser string, that's what i want to use */
            if( strcmp(nextOrigFileName, nextOrigDirName) > 0 )
            /* filename > dirname */
            {
                takeDirNext = true;
            }
            else
            /* dirname > filename */
            {
                takeDirNext = false;
            }
        }
        
        if(takeDirNext)
        {
            if( strcmp(nextOrigDirName, prevOrigDirName) == 0 ||
                strcmp(nextOrigDirName, prevOrigFileName) == 0 )
            {
                // insert ~xxx in dir name 
                
                
            }
            else
            {
                fileNumber = -1;
            }
            
            // add dir with new name to new list
            
            strcpy(nextOrigDirName, prevOrigDirName);
            
            nextOrigDir = nextOrigDir->next;
        }
        else
        /* take file next */
        {
            if( strcmp(nextOrigFileName, prevOrigFileName) == 0 ||
                strcmp(nextOrigFileName, prevOrigDirName) == 0 )
            {
                // insert ~xxx in file name 
                
                
            }
            else
            {
                fileNumber = -1;
            }
            
            // add file with new name to new list
            
            strcpy(nextOrigFileName, prevOrigFileName);
            
            nextOrigFile = nextOrigFile->next;
        }
    } /* while (have a file or directory to convert) */
    printf("finished\n");
    return 1;
}

void mangleDirName(char* src, char* dest, int fileNameType)
{
    strcpy(dest, src);
    
    dest[8] = '\0';
    
    // get rid of bad characters
}

void mangleFileName(char* src, char* dest, int fileNameType, char* base, char* extension)
{
    int baseMaxLen;
    int extMaxLen;
    int lenOrig;
    int splitAt;
    
    //!! other types, 9660 should be default
    if(fileNameType & FNTYPE_9660)
    {
        baseMaxLen = 8;
        extMaxLen = 3;
    }
    
    /* SPLIT name */
    lenOrig = strlen(src);
    
    /* find the dot at most extMaxLen + 1 characters from the end */
    splitAt = lenOrig - 1;
    while(splitAt > lenOrig - extMaxLen + 1 && splitAt >= 0 && src[splitAt] != '.')
        splitAt--;
    
    /* copy base (don't want a trailing dot) */
    if(src[splitAt] == '.')
    {
        strncpy(base, src, splitAt);
        base[splitAt] = '\0';
    }
    else
    {
        strncpy(base, src, splitAt + 1);
        base[splitAt + 1] = '\0';
    }
    
    /* copy extension */
    strncpy(extension, src + splitAt + 1, extMaxLen + 1);
    extension[extMaxLen + 1] = '\0';
    /* END SPLIT name */
    
    strncpy(dest, base, baseMaxLen);
    dest[baseMaxLen] = '\0';
    
    strcat(dest, ".");
    
    extension[extMaxLen] = '\0';
    strcat(dest, extension);
    
    // get rid of bad characters
}

void splitFileName(char* src, char* base, char* extension)
{
    int lenOrig;
    int splitAt;
    
    lenOrig = strlen(src);
    
    /* find the dot at most four characters from the end */
    splitAt = lenOrig - 1;
    while(splitAt > lenOrig - 5 && splitAt >= 0 && src[splitAt] != '.')
        splitAt--;
    
    /* copy base (don't want a trailing dot) */
    if(src[splitAt] == '.')
    {
        strncpy(base, src, splitAt);
        base[splitAt] = '\0';
    }
    else
    {
        strncpy(base, src, splitAt + 1);
        base[splitAt + 1] = '\0';
    }
    
    /* copy extension */
    strncpy(extension, src + splitAt + 1, 5);
    extension[5] = '\0';
}
