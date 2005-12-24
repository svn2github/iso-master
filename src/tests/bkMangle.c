#include "bk.h"
#include "bkMangle.h"

#include <stdio.h>
#include <string.h>

//!! origDir files and dirs have to be sorted
int mangleDir(Dir* origDir, Dir* newDir, int fileNameType)
{
    DirLL* nextOrigDir;
    char nextOrigDirName[NCHARS_FILE_ID_MAX]; /* mangled */
    //char prevOrigDirName[NCHARS_FILE_ID_MAX];
    
    FileLL* nextOrigFile;
    char nextOrigFileName[NCHARS_FILE_ID_MAX]; /* mangled */
    char nextOrigFileBase[NCHARS_FILE_ID_MAX]; /* in case need to insert ~xxx */
    char nextOrigFileExt[5]; /* in case need to insert ~xxx */
    //char prevOrigFileName[NCHARS_FILE_ID_MAX];
    /*char prevOrigFileBase[NCHARS_FILE_ID_MAX];
    char prevOrigFileExt[6];*/
    
    char prevOrigName[NCHARS_FILE_ID_MAX]; /* either file or dir */
    
    DirLL** nextNewDir;
    FileLL** nextNewFile;
    
    bool takeDirNext; /* what to work on next, a dir or a file? */
    int fileNumber; /* long~001.txt */
    
    nextOrigFile = origDir->files;
    nextOrigDir = origDir->directories;
    
    nextNewFile = &(newDir->files);
    *nextNewFile = NULL;
    nextNewDir = &(newDir->directories);
    *nextNewDir = NULL;
    
    //prevOrigFileName[0] = '\0';
    //prevOrigDirName[0] = '\0';
    prevOrigName[0] = '\0';
    fileNumber = -1;
    while( nextOrigFile != NULL || nextOrigDir != NULL )
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
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType, 
                           nextOrigFileBase, nextOrigFileExt);
            printf("next file: %s -> '%s'\n", nextOrigFile->file.name, nextOrigFileName);
        }
        else
        /* have both a file and a directory */
        {
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName, fileNameType);
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType, 
                           nextOrigFileBase, nextOrigFileExt);
            
            printf("c next dir: %s -> '%s'\n", nextOrigDir->dir.name, nextOrigDirName);
            printf("c next file: %s -> '%s'\n", nextOrigFile->file.name, nextOrigFileName);
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
            //~ if( strcmp(nextOrigDirName, prevOrigDirName) == 0 ||
                //~ strcmp(nextOrigDirName, prevOrigFileName) == 0 )
            if( strcmp(nextOrigDirName, prevOrigName) == 0 )
            {
                // insert ~xxx in dir name 
                
                fileNumber++;
            }
            else
            {
                /* save name */
                strcpy(prevOrigName, nextOrigDirName);
                
                fileNumber = -1;
            }
            
            // add dir with new name to new list
            
            nextOrigDir = nextOrigDir->next;
        }
        else
        /* take file next */
        {
            //~ if( strcmp(nextOrigFileName, prevOrigFileName) == 0 ||
                //~ strcmp(nextOrigFileName, prevOrigDirName) == 0 )
            if( strcmp(nextOrigFileName, prevOrigName) == 0 )
            {
                // insert ~xxx in file name 
                
                fileNumber++;
            }
            else
            {
                /* save name */
                strcpy(prevOrigName, nextOrigFileName);
                
                fileNumber = -1;
            }
            
            // add file with new name to new list
            
            nextOrigFile = nextOrigFile->next;
        }
        
    } /* while (have a file or directory to convert) */
    printf("finished\n");
    return 1;
}

void mangleDirName(char* src, char* dest, int fileNameType)
{
    int maxLen;
    
    //!! other types, 9660 should be default
    if(fileNameType == FNTYPE_9660)
    {
        maxLen = 8;
    }
    
    strcpy(dest, src);
    
    dest[maxLen] = '\0';
    
    replaceIllegalChars(dest, fileNameType);
}

// will this work with file "a" ?
void mangleFileName(char* src, char* dest, int fileNameType, char* base, char* extension)
{
    int baseMaxLen;
    int extMaxLen;
    int lenOrig;
    int splitAt;
    
    //!! other types, 9660 should be default
    if(fileNameType == FNTYPE_9660)
    {
        baseMaxLen = 8;
        extMaxLen = 3;
    }
    
    /* SPLIT name */
    lenOrig = strlen(src);
    
    /* find the dot at most extMaxLen + 1 characters from the end */
    splitAt = lenOrig - 1;
    while(splitAt > lenOrig - extMaxLen - 1 && splitAt >= 1 && src[splitAt] != '.')
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
    
    replaceIllegalChars(base, fileNameType);
    
    /* copy extension */
    strncpy(extension, src + splitAt + 1, extMaxLen + 1);
    extension[extMaxLen + 1] = '\0';
    
    replaceIllegalChars(extension, fileNameType);
    /* END SPLIT name */
    
    strncpy(dest, base, baseMaxLen);
    dest[baseMaxLen] = '\0';
    
    strcat(dest, ".");
    
    extension[extMaxLen] = '\0';
    strcat(dest, extension);
}

void replaceIllegalChars(char* string, int fileNameType)
{
    int count;
    int stringLen;
    
    stringLen = strlen(string);
    
    //!! other types, 9660 should be default
    if(fileNameType == FNTYPE_9660)
    {
        for(count = 0; count < stringLen; count++)
        {
            if( string[count] >= 97 && string[count] <= 122 )
            /* lowercase alpha */
            {
                /* convert to uppercase */
                string[count] = string[count] - 32;
            }
            else if( (string[count] >= 48 && string[count] <= 57) ||
                     (string[count] >= 65 && string[count] <= 90) ||
                     string[count] == '.' ||
                     string[count] == '-' ||
                     string[count] == '_' )
            /* these are ok */
            {
                /* do nothing */
                ;
            }
            else
            /* some fancy character */
            {
                /* convert to '0' */
                string[count] = '0';
            }
        }
    }
}
