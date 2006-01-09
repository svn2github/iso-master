#include "bk.h"
#include "bkMangle.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* length of aaa in aaa~xxxx.bbb */
#define NCHARS_9660_BASE 3

/*
* note that some unsigned ints in mangling functions are
* required to be 32 bits long for the hashing to work
* see the samba code for details
*/

/*
mangle_get_prefix
is_valid_name

name_map

*/

bool charIsValid9660(char theChar)
{
    if( (theChar >= '0' && theChar <= '9') ||
        (theChar >= 'a' && theChar <= 'z') ||
        (theChar >= 'A' && theChar <= 'Z') ||
        strchr("_-$~", theChar) )
    {
        return true;
    }
    else
        return false;
}

/* 
   hash a string of the specified length. The string does not need to be
   null terminated 

   this hash needs to be fast with a low collision rate (what hash doesn't?)
*/
unsigned hashString(const char *str, unsigned int length)
{
    unsigned value;
    unsigned i;
    
    static const unsigned fnv1Prime = 0x01000193;
    
    /* Set the initial value from the key size. */
    /* fnv1 of the string: idra@samba.org 2002 */
    value = 0xa6b93095;
    for (i = 0; i < length; i++)
    {
        value *= (unsigned)fnv1Prime;
        value ^= (unsigned)(str[i]);
    }
    
    /* note that we force it to a 31 bit hash, to keep within the limits
       of the 36^6 mangle space */
    return value & ~0x80000000;  
}

void mangleNameFor9660(char* origName, char* newName, bool isADir)
{
    char *dot_p;
    int i;
    char base[7]; /* max 6 chars */
    char extension[4]; /* max 3 chars */
    int extensionLen;
    unsigned hash;
    unsigned v;
    /* these are the characters we use in the 8.3 hash. Must be 36 chars long */
    static const char *baseChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    /* FIND extension */
    if(isADir)
    /* no extension */
    {
        dot_p = NULL;
    }
    else
    {
        dot_p = strrchr(origName, '.');
        
        if(dot_p)
        {
            /* if the extension contains any illegal characters or
               is too long (> 3) or zero length then we treat it as part
               of the prefix */
            for(i = 0; i < 4 && dot_p[i + 1] != '\0'; i++)
            {
                if( !charIsValid9660(dot_p[i + 1]) )
                {
                    dot_p = NULL;
                    break;
                }
            }
            
            if(i == 0 || i == 4)
                dot_p = NULL;
        }
    }
    /* END FIND extension */
    
    /* GET base */
    /* the leading characters in the mangled name is taken from
       the first characters of the name, if they are ascii otherwise
       '_' is used
    */
    for(i = 0; i < NCHARS_9660_BASE && origName[i] != '\0'; i++)
    {
        base[i] = origName[i];
        
        if ( !charIsValid9660(origName[i]) )
            base[i] = '_';
        
        base[i] = toupper(base[i]);
    }
    
    /* make sure base doesn't contain part of the extension */
    if(dot_p != NULL)
    {
        //!! test this
        if(i > dot_p - origName)
            i = dot_p - origName;
    }
    
    /* fixed length */
    while(i < NCHARS_9660_BASE)
    {
        base[i] = '_';
        
        i++;
    }
    
    base[NCHARS_9660_BASE] = '\0';
    /* END GET base */
    
    /* GET extension */
    /* the extension of the mangled name is taken from the first 3
       ascii chars after the dot */
    extensionLen = 0;
    if(dot_p)
    {
        for(i = 1; extensionLen < 3 && dot_p[i] != '\0'; i++)
        {
            extension[extensionLen] = toupper(dot_p[i]);
            
            extensionLen++;
        }
    }
    
    extension[extensionLen] = '\0';
    /* END GET extension */
    
    /* find the hash for this prefix */
    hash = hashString(origName, strlen(origName));
    
    /* now form the mangled name. */
    for(i = 0; i < NCHARS_9660_BASE; i++)
    {
            newName[i] = base[i];
    }
    
    newName[NCHARS_9660_BASE] = '~';
    
    v = hash;
    newName[7] = baseChars[v % 36];
    for(i = 6; i > NCHARS_9660_BASE; i--)
    {
        v = v / 36;
        newName[i] = baseChars[v % 36];
    }
    
    /* add the extension and terminate string */
    if(extensionLen > 0)
    {
        newName[8] = '.';
        
        strcpy(newName + 9, extension);
    }
    else
    {
        newName[8] = '\0';
    }
    printf("'%s' -> '%s'\n", origName, newName);
}

/* filenametypes is all types required in the end */
int mangleDir2(Dir* origDir, DirToWrite* newDir, int filenameTypes)
{
    bool haveCollisions;
    int numTimesTried;
    int numCollisions;
    
    DirLL* currentOrigDir;
    DirToWriteLL** currentNewDir;
    FileLL* currentOrigFile;
    FileToWriteLL** currentNewFile;
    
    DirToWriteLL* currentDir;
    FileToWriteLL* currentFile;
    DirToWriteLL* currentDirToCompare;
    FileToWriteLL* currentFileToCompare;
    
    /* MANGLE all names, create new dir/file lists */
    currentOrigDir = origDir->directories;
    currentNewDir = &(newDir->directories);
    while(currentOrigDir != NULL)
    /* have directories */
    {
        *currentNewDir = malloc(sizeof(DirToWriteLL));
        if(*currentNewDir == NULL)
            return -1;
        
        bzero(*currentNewDir, sizeof(DirToWriteLL));
        
        mangleNameFor9660(currentOrigDir->dir.name, (*currentNewDir)->dir.name9660, true);
        
        if(filenameTypes | FNTYPE_ROCKRIDGE)
            strcpy((*currentNewDir)->dir.nameRock, currentOrigDir->dir.name);
        else
            (*currentNewDir)->dir.nameRock[0] = '\0';
        
        if(filenameTypes | FNTYPE_JOLIET)
            strcpy((*currentNewDir)->dir.nameJoliet, currentOrigDir->dir.name);
        else
            (*currentNewDir)->dir.nameJoliet[0] = '\0';
        
        // recursive call
        
        currentOrigDir = currentOrigDir->next;
        
        currentNewDir = &((*currentNewDir)->next);
    }
    
    currentOrigFile = origDir->files;
    currentNewFile = &(newDir->files);
    while(currentOrigFile != NULL)
    /* have files */
    {
        *currentNewFile = malloc(sizeof(FileToWriteLL));
        if(*currentNewFile == NULL)
            return -1;
        
        bzero(*currentNewFile, sizeof(FileToWriteLL));
        
        mangleNameFor9660(currentOrigFile->file.name, (*currentNewFile)->file.name9660, false);
        
        if(filenameTypes | FNTYPE_ROCKRIDGE)
            strcpy((*currentNewFile)->file.nameRock, currentOrigFile->file.name);
        else
            (*currentNewFile)->file.nameRock[0] = '\0';
        
        if(filenameTypes | FNTYPE_JOLIET)
            strcpy((*currentNewFile)->file.nameJoliet, currentOrigFile->file.name);
        else
            (*currentNewFile)->file.nameJoliet[0] = '\0';
        
        currentOrigFile = currentOrigFile->next;
        
        currentNewFile = &((*currentNewFile)->next);
    }
    /* END MANGLE all names, create new dir/file lists */
    
    haveCollisions = true;
    numTimesTried = 0;
    while(haveCollisions && numTimesTried < 5)
    {
        haveCollisions = false;
        
        // for each subdir
          // look through entire dir list and count collisions
          // look through entire file list and count collisions
          // if more then 1, remangle name
        
        // for each file
          // look through entire dir list and count collisions
          // look through entire file list and count collisions
          // if more then 1, remangle name
        
        currentDir = newDir->directories;
        while(currentDir != NULL)
        {
            numCollisions = 0;
            
            currentDirToCompare = newDir->directories;
            while(currentDirToCompare != NULL)
            {
                if(strcmp(currentDir->dir.name9660, 
                          currentDirToCompare->dir.name9660) == 0)
                {
                    numCollisions++;
                }
                
                currentDirToCompare = currentDirToCompare->next;
            }
            
            currentFileToCompare = newDir->files;
            while(currentFileToCompare != NULL)
            {
                if(strcmp(currentDir->dir.name9660, 
                          currentFileToCompare->file.name9660) == 0)
                {
                    numCollisions++;
                }
                
                currentFileToCompare = currentFileToCompare->next;
            }
            
            if(numCollisions != 1)
            {
                haveCollisions = true;
                
                // remangle currentDirLL->dir.name9660
            }
            
            currentDir = currentDir->next;
        }
        
        currentFile = newDir->files;
        while(currentFile != NULL)
        {
            numCollisions = 0;
            
            currentDirToCompare = newDir->directories;
            while(currentDirToCompare != NULL)
            {
                if(strcmp(currentFile->file.name9660, 
                          currentDirToCompare->dir.name9660) == 0)
                {
                    numCollisions++;
                }
                
                currentDirToCompare = currentDirToCompare->next;
            }
            
            currentFileToCompare = newDir->files;
            while(currentFileToCompare != NULL)
            {
                if(strcmp(currentFile->file.name9660, 
                          currentFileToCompare->file.name9660) == 0)
                {
                    numCollisions++;
                }
                
                currentFileToCompare = currentFileToCompare->next;
            }
            
            if(numCollisions != 1)
            {
                haveCollisions = true;
                
                // remangle currentFileLL->file.name9660
            }
            
            currentFile = currentFile->next;
        }
        
        numTimesTried++;
    }
    
    if(haveCollisions)
        return -2;
    
    return 1;
}

//!! origDir files and dirs have to be sorted
int mangleDir(Dir* origDir, DirToWrite* newDir, int fileNameTypes)
{
    //char newMangleTest[NCHARS_FILE_ID_MAX];
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
            //mangleNameFor9660(nextOrigDir->dir.name, newMangleTest, true);
            //printf("%s\n", newMangleTest);
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
            
            (*nextNewDir)->dir.extentLocationOffset2 = 0;
            
            (*nextNewDir)->dir.extentNumber2 = 0;
            
            (*nextNewDir)->dir.dataLength2 = 0;
                
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
            //mangleNameFor9660(nextOrigFile->file.name, newMangleTest, false);
            //printf("%s\n", newMangleTest);
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
            
            (*nextNewFile)->file.extentLocationOffset2 = 0;
            
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
