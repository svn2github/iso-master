#include "bk.h"
#include "bkMangle.h"

#include <stdio.h>
#include <string.h>

/*
mangle_get_prefix
is_valid_name

*/

//!! origDir files and dirs have to be sorted
int mangleDir(Dir* origDir, DirToWrite* newDir, int fileNameTypes)
{
    DirLL* nextOrigDir;
    char nextOrigDirName[NCHARS_FILE_ID_MAX]; /* mangled */
    
    FileLL* nextOrigFile;
    char nextOrigFileName[NCHARS_FILE_ID_MAX]; /* mangled */
    char nextOrigFileBase[NCHARS_FILE_ID_MAX]; /* in case need to insert ~xxx */
    char nextOrigFileExt[5]; /* in case need to insert ~xxx */
    
    char prevOrigName[NCHARS_FILE_ID_MAX]; /* either file or dir */
    
    DirToWriteLL** nextNewDir;
    FileToWriteLL** nextNewFile;
    
    bool takeDirNext; /* what to work on next, a dir or a file? */
    int fileNumber; /* long~001.txt */
    
    int rc;
    
    //!! i really really don't want to do this:
    newDir->posixFileMode = origDir->posixFileMode;
    
    nextOrigDir = origDir->directories;
    nextOrigFile = origDir->files;
    
    nextNewDir = &(newDir->directories);
    *nextNewDir = NULL;
    nextNewFile = &(newDir->files);
    *nextNewFile = NULL;
    
    prevOrigName[0] = '\0';
    fileNumber = -1;
    while( nextOrigFile != NULL || nextOrigDir != NULL )
    /* have a file or directory to convert */
    {
        if(nextOrigFile == NULL)
        /* no files left */
        {
            takeDirNext = true;
            
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName);
        }
        else if(nextOrigDir == NULL)
        /* no directories left */
        {
            takeDirNext = false;
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, 
                           nextOrigFileBase, nextOrigFileExt);
        }
        else
        /* have both a file and a directory */
        {
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName);
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, 
                           nextOrigFileBase, nextOrigFileExt);
            
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
            if( strcmp(nextOrigDirName, prevOrigName) == 0 )
            {
                fileNumber++;
                
                /* there will only be a conflict if names have been shortned,
                * so the name will always be 8 chars long */
                sprintf(nextOrigDirName + 4, "~%03d", fileNumber);
            }
            else
            {
                /* save name */
                strcpy(prevOrigName, nextOrigDirName);
                
                fileNumber = -1;
            }
            
            *nextNewDir = malloc(sizeof(DirToWriteLL));
            if(*nextNewDir == NULL)
                return -1;
            
            strcpy((*nextNewDir)->dir.name9660, nextOrigDirName);
            
            if(fileNameTypes | FNTYPE_ROCKRIDGE)
                strcpy((*nextNewDir)->dir.nameRock, nextOrigDir->dir.name);
            else
                (*nextNewDir)->dir.nameRock[0] = '\0';
            
            if(fileNameTypes | FNTYPE_JOLIET)
                strcpy((*nextNewDir)->dir.nameJoliet, nextOrigDir->dir.name);
            else
                (*nextNewDir)->dir.nameJoliet[0] = '\0';
            
            (*nextNewDir)->dir.posixFileMode = nextOrigDir->dir.posixFileMode;
            
            (*nextNewDir)->dir.extentLocationOffset = 0;
            
            (*nextNewDir)->dir.extentNumber = 0;
            
            (*nextNewDir)->dir.dataLength = 0;
                
            rc = mangleDir( &(nextOrigDir->dir), &((*nextNewDir)->dir), fileNameTypes);
            if(rc < 0)
                return rc;
            
            nextNewDir = &((*nextNewDir)->next);
            *nextNewDir = NULL;
            
            nextOrigDir = nextOrigDir->next;
        }
        else
        /* take file next */
        {
            if( strcmp(nextOrigFileName, prevOrigName) == 0 )
            {
                fileNumber++;
                
                /* there will only be a conflict if names have been shortned,
                * so the name will always be 8 chars long */
                sprintf(nextOrigFileBase + 4, "~%03d", fileNumber);
            }
            else
            {
                /* save name */
                strcpy(prevOrigName, nextOrigFileName);
                
                fileNumber = -1;
            }
            
            *nextNewFile = malloc(sizeof(FileToWriteLL));
            if(*nextNewFile == NULL)
                return -1;
            
            strcpy((*nextNewFile)->file.name9660, nextOrigFileName);
            
            if(fileNameTypes | FNTYPE_ROCKRIDGE)
                strcpy((*nextNewFile)->file.nameRock, nextOrigFile->file.name);
            else
                (*nextNewFile)->file.nameRock[0] = '\0';
            
            if(fileNameTypes | FNTYPE_JOLIET)
                strcpy((*nextNewFile)->file.nameJoliet, nextOrigFile->file.name);
            else
                (*nextNewFile)->file.nameJoliet[0] = '\0';
            
            (*nextNewFile)->file.posixFileMode = nextOrigFile->file.posixFileMode;
            
            (*nextNewFile)->file.extentLocationOffset = 0;
            
            (*nextNewFile)->file.extentNumber = 0;
            
            (*nextNewFile)->file.dataLength = 0;
            
            (*nextNewFile)->file.size = nextOrigFile->file.size;
            
            (*nextNewFile)->file.onImage = nextOrigFile->file.onImage;
            
            (*nextNewFile)->file.position = nextOrigFile->file.position;
                
            nextNewFile = &((*nextNewFile)->next);
            *nextNewFile = NULL;
            
            nextOrigFile = nextOrigFile->next;
        }
        
    } /* while (have a file or directory to convert) */
    
    return 1;
}

void mangleDirName(char* src, char* dest)
{
    static int maxLen = 8;
    
    strncpy(dest, src, maxLen);
    
    dest[maxLen] = '\0';
    
    replaceIllegalChars(dest);
}

// will this work with file "a" ?
void mangleFileName(char* src, char* dest, char* base, char* extension)
{
    static int baseMaxLen = 8;
    static int extMaxLen = 3;
    int lenOrig;
    int splitAt; /* index of dot or if no dot exists,
                    character just before extension, or if no extension exists,
                    last character in name */
    int count;
    int count2;
    
    /* SPLIT name */
    lenOrig = strlen(src);
    
    /* find the dot at most extMaxLen + 1 characters from the end */
    splitAt = lenOrig - 1;
    while(splitAt > lenOrig - 1 - extMaxLen && splitAt > 1 && src[splitAt] != '.')
        splitAt--;
    
    /* copy base (don't want a trailing dot) */
    if(src[splitAt] == '.')
    {
        /* index of dot = len of string before dot */
        strncpy(base, src, splitAt);
        base[splitAt] = '\0';
    }
    else
    {
        strncpy(base, src, splitAt + 1);
        base[splitAt + 1] = '\0';
    }
    
    replaceIllegalChars(base);
    
    /* copy extension including null byte. works if no extension exists */
    for(count = splitAt + 1, count2 = 0; count <= lenOrig; count++, count2++)
        extension[count2] = src[count];
    
    replaceIllegalChars(extension);
    /* END SPLIT name */
    
    strncpy(dest, base, baseMaxLen);
    dest[baseMaxLen] = '\0';
    
    strcat(dest, ".");
    
    strcat(dest, extension);
}

void replaceIllegalChars(char* string)
{
    int count;
    int stringLen;
    
    stringLen = strlen(string);
    
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
            string[count] = '_';
        }
    }
}
