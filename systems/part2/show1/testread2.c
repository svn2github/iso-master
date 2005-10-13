#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>

#include "bk.h"
#include "bkAdd.h"
#include "bkRead.h"
#include "bkDelete.h"
#include "bkExtract.h"

#include "vd.h"

void oops(char* msg)
{
    fflush(NULL);
    fprintf(stderr, "OOPS, %s\n", msg);
    exit(0);
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
            printf("@%08X\n", fileNode->file.position);
        fileNode = fileNode->next;
    }
}

int main(int argc, char** argv)
{
    int image;
    VdSet vdset;
    int rc;
    
    Dir tree;
    
    if(argc != 2)
        oops("usage: testread2 image.iso");
    
    /* open image file for reading */
    image = open(argv[1], O_RDONLY);
    if(image == -1)
        oops("unable to open image");
    
    /* skip system area */
    lseek(image, NLS_SYSTEM_AREA * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    
    // volume descriptor set 
    rc = readVDSet(image, &vdset);
    if(rc <= 0)
        oops("problem reading vd set");
    
    lseek(image, vdset.svd.rootDROffset, SEEK_SET);
    tree.directories = NULL;
    tree.files = NULL;
    rc = readDir(image, &tree, FNTYPE_9660, true);
    
    rc = close(image);
    if(rc == -1)
        oops("faled to close image");
    
    showDir(&tree, 0);
    
    return 0;
}
