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
    while(nextOrigFile != NULL && nextOrigDir != NULL)
    /* have a file or directory to convert */
    {
        if(nextOrigFile == NULL)
        /* no files left */
        {
            takeDirNext = true;
            
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName, fileNameType);
        }
        else if(nextOrigDir == NULL)
        /* no directories left */
        {
            takeDirNext = false;
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType);
            splitFileName(nextOrigFileName, nextOrigFileBase, nextOrigFileExt);
        }
        else
        /* have both a file and a directory */
        {
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName, fileNameType);
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType);
            splitFileName(nextOrigFileName, nextOrigFileBase, nextOrigFileExt);
            
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
    
    return 1;
}

void mangleDirName(char* src, char* dest, int fileNameType)
{
    strcpy(src, dest);
    
    dest[8] = '\0';
    
    // get rid of bad characters
}

void mangleFileName(char* src, char* dest, int fileNameType)
{
    char base[NCHARS_FILE_ID_MAX];
    char extension[5];
    
    splitFileName(src, base, extension);
    
    strncpy(dest, base, 8);
    
    strcat(dest, ".");
    
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
    
    /* copy base */
    strncpy(base, src, splitAt);
    
    /* copy extension */
    strncpy(extension, src + splitAt + 1, 4);
}
