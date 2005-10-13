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
            printf("on image @%08X\n", fileNode->file.position);
        else
            printf("on disk: \'%s\'\n", fileNode->file.pathAndName);
        fileNode = fileNode->next;
    }
}

int main(int argc, char** argv)
{
    int image;
    //VdSet vdset;
    VolInfo volInfo;
    int rc;
    
    //Dir tree;
    FilePath filePath;
    Path srcDir;
    Path dirPath;
    char* dest; /* destination directory */
    char* fileToAdd;
    char* dirToAdd;
    //char dirName[256];
    
    /* open image file for reading */
    image = open(argv[1], O_RDONLY);
    if(image == -1)
        oops("unable to open image");
    
    /* skip system area 
    lseek(image, NLS_SYSTEM_AREA * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    
    // volume descriptor set 
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
    */
    
    /*lseek(image, vdset.pvd.rootDROffset, SEEK_SET);
    tree.directories = NULL;
    tree.files = NULL;
    rc = readDir(image, &tree, FNTYPE_ROCKRIDGE, true);
    printf("readDir ended with %d\n", rc);*/
    
    rc = readVolInfo(image, &volInfo);
    if(image <= 0)
        oops("failed to read volume info");
    
    rc = close(image);
    if(rc == -1)
        oops("faled to close image");
    
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
    
    dest = malloc(strlen("/home/andrew/prog/isomaster/src/tests/") + 1);
    strcpy(dest, "/home/andrew/prog/isomaster/src/tests/");
    
    fileToAdd = malloc(strlen("/home/andrew/prog/isomaster/src/tests/read7x.o") + 1);
    strcpy(fileToAdd, "/home/andrew/prog/isomaster/src/tests/read7x.o");
    
    dirToAdd = malloc(strlen("/etc/") + 1);
    strcpy(dirToAdd, "/etc/");
    
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
    
    //rc = addDir(&tree, dirToAdd, &dirPath);
    //if(rc <= 0)
    //    oops("problem adding dir");
    
    //showDir(&tree, 0);
    
    return 0;
}
