#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "testread2.h"
#include "read7x.h"
#include "vd.h"

void oops(char* msg);

int main(int argc, char** argv)
{
    int image;
    VdSet vdset;
    int rc;
    
    Dir tree;
    
    /* open image file for reading */
    image = open(argv[1], O_RDONLY);
    printf("%s\n", argv[1]);
    if(image == -1)
        oops("unable to open image\n");
    
    /* skip system area */
    lseek(image, NLS_SYSTEM_AREA * NBYTES_LOGICAL_SECTOR, SEEK_SET);
    
    /* volume descriptor set */
    rc = readVDSet(image, &vdset);
    if(rc <= 0)
        oops("problem reading vd set");
    
    printf("lb size: %d\n", vdset.pvd.lbSize);
    
    printf("volume space size: %u\n", vdset.pvd.volSpaceSize);
    
    printf("human-readable volume size: %dB, %dMB, %dMiB\n", vdset.pvd.lbSize * vdset.pvd.volSpaceSize,
                                                             vdset.pvd.lbSize * vdset.pvd.volSpaceSize / 1024000,
                                                             vdset.pvd.lbSize * vdset.pvd.volSpaceSize / 1048576);
    printf("pathtable size: %d\n", vdset.pvd.pathTableSize);
    
    printf("vsid: \'%s\'\n", vdset.pvd.volSetId);
    
    printf("publ: \'%s\'\n", vdset.pvd.publId);
    
    printf("dprp: \'%s\'\n", vdset.pvd.dataPrepId);
    
    printf("created: %s-%s-%s, %s:%s:%s:%s GMT%d\n", vdset.pvd.volCreatTime.day,
                                                     vdset.pvd.volCreatTime.month,
                                                     vdset.pvd.volCreatTime.year,
                                                     vdset.pvd.volCreatTime.hour,
                                                     vdset.pvd.volCreatTime.minute,
                                                     vdset.pvd.volCreatTime.second,
                                                     vdset.pvd.volCreatTime.hundredthSecond,
                                                     vdset.pvd.volCreatTime.gmtOffset);
    
    
    printf("L path table: %d\n", vdset.pvd.locTypeLPathTable);
    
    printf("M path table: %d\n", vdset.pvd.locTypeMPathTable);
    
    printf("root extent at: %d\n", vdset.pvd.rootDR.locExtent);
    
    printf("data length: %d\n", vdset.pvd.rootDR.dataLength);
    
    //printf("joliet type: %d\n", svdGetJolietType(&(vdset.svd)));
    printf("joliet root extent at: %d\n", vdset.svd.rootDR.locExtent);
    
    lseek(image, vdset.svd.rootDR.locExtent * NBYTES_LOGICAL_SECTOR, SEEK_SET);
    
    readDir(image, &tree, FNTYPE_9660);
    
    close(image);
    
    return 0;
}

void oops(char* msg)
{
    fprintf(stderr, "OOPS, %s\n", msg);
    exit(0);
}

int readDir(int image, Dir* dir, int filenameType)
{
    unsigned char recordLength;
    // 4 bytes extAttrRecLen;
    // 4 bytes locExtent;
    // 4 bytes dataLength;
    // 7 bytes time
    unsigned char fileFlags;
    // 2 bytes file unit + interleave gap size
    // 4 bytes volume sq. number
    unsigned char fullNameLen9660;
    // name
    // if(dr->fullNameLen % 2 == 0)
    // dr->suFieldsLen = dr->recordLength - (33 + dr->fullNameLen + (dr->fullNameLen % 2 == 0 ? 1 : 0));
    
    rc = read711(file, &(dr->recordLength));
    if(rc != 1)
        return -1;
    count += 1;
    
    lseek(image, 4, SEEK_CUR);
    lseek(image, 4, SEEK_CUR);
    lseek(image, 4, SEEK_CUR);
    lseek(image, 7, SEEK_CUR);
    
    
}
