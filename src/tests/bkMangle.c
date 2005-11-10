#include "bk.h"

#include <string.h>

//!! origDir has to be sorted
int mangleDir(Dir* origDir, Dir* newDir, int fileNameType)
{
    // apply filenametype rules to possible file and dir following
    // decide whether to take from directories or files
    // find entry name base, ext
    // compare new name to old file name
    // compare new name to old dir name
      // if same, num++
      // else, num = -1
    // new name = base + num (opt) + ext (opt)
    
    FileLL* nextOrigFile;
    char nextOrigFileName[NCHARS_FILE_ID_MAX];
    char nextOrigFileBase[NCHARS_FILE_ID_MAX]; /* used for mangling */
    char nextOrigFileExt[6]; /* used for mangling */
    char prevOrigFileBase[NCHARS_FILE_ID_MAX];
    char prevOrigFileExt[6];
    char prevOrigFileName[NCHARS_FILE_ID_MAX];
    
    DirLL* nextOrigDir;
    char nextOrigDirName[NCHARS_FILE_ID_MAX];
    char prevOrigDirName[NCHARS_FILE_ID_MAX];
    
    FileLL** nextNewFile;
    DirLL** nextNewDir;
    
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
            
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType);
            splitFileName(nextOrigFileName, nextOrigFileBase, nextOrigFileExt);
        }
        else if(nextOrigDir == NULL)
        /* no directories left */
        {
            takeDirNext = false;
            
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName);
        }
        else
        /* have both a file and a directory */
        {
            mangleFileName(nextOrigFile->file.name, nextOrigFileName, fileNameType);
            splitFileName(nextOrigFileName, nextOrigFileBase, nextOrigFileExt);
            
            mangleDirName(nextOrigDir->dir.name, nextOrigDirName);
            
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
                dirNameSameAsFileName(nextOrigDirName, prevOrigFileBase, prevOrigFileExt) )
            {
                // insert ~xxx in dir name 
                
                
            }
            else
            {
                fileNumber = -1;
            }
            
            // add dir with new name to new list
            
            // copy next to previous
            
            nextOrigDir = nextOrigDir->next;
        }
        else
        /* take file next */
        {
            if( fileNameSameAsFileName(nextOrigFileBase, nextOrigFileExt,
                                       prevOrigFileBase, prevOrigFileExt) ||
                dirNameSameAsFileName(prevOrigDirName, 
                                      nextOrigFileBase, nextOrigFileExt) )
            {
                // insert ~xxx in file name 
                
                
            }
            else
            {
                fileNumber = -1;
            }
            
            // add file with new name to new list
            
            // move next to previous
            
            nextOrigFile = nextOrigFile->next;
        }
        
    } /* while (have a file or directory to convert) */
    
    return 1;
}
