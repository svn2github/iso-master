#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define NB_SYSTEM_AREA 32768

#define VDTYPE_BOOT 0
#define VDTYPE_PRIMARY 1
#define VDTYPE_SUPPLEMENTARY 2
#define VDTYPE_VOLUMEPARTITION 3
#define VDTYPE_TERMINATOR 255

/* date/time as described in 8.4.26.1 (pvd)
* all strings are size +1 for the null byte */
typedef struct
{
    char year[5]; /* ex. 2005 */
    char month[3];
    char day[3];
    char hour[3];
    char minute[3];
    char second[3];
    char hundredthSecond[3];
    signed char gmtOffset; /* number not character */
    
} dateTime;

/* primary volume descriptor version 1 (original iso9660 standard)
* all strings are size +1 for the null byte */
typedef struct
{
    char sysId[33];
    char volId[33];
    unsigned volSpaceSize;
    unsigned short volSetSize;
    unsigned short volSeqNum;
    unsigned short lbSize;
    unsigned pathTableSize;
    unsigned locTypeLPathTable;
    unsigned locOptTypeLPathTable;
    unsigned locTypeMPathTable;
    unsigned locOptTypeMPathTable;
    //root dir record
    char volSetId[129];
    char publId[129];
    char dataPrepId[129];
    char appId[129];
    char copyrightFid[38];
    char abstractFid[38];
    char biblFid[38];
    dateTime volCreatTime;
    dateTime volModTime;
    dateTime volExpTime;
    dateTime volEffTime;
    
} pvdv1;

int read711(int file, unsigned char* value);
int read712(int file, signed char* value);
int read721(int file, unsigned short* value);
int read722(int file, unsigned short* value);
int read723(int file, unsigned short* value);
int read731(int file, unsigned* value);
int read732(int file, unsigned* value);
int read733(int file, unsigned* value);

int readUnused(int file, unsigned numBytes);

int readVdTypeVer(int file, unsigned char* type, unsigned char* version);

int readPVDv1(int file, pvdv1* pvd);

void oops(char* msg)
{
    fprintf(stderr, "OOPS, %s\n", msg);
    exit(0);
}

void printByte(char byte)
{
    int count = 8;
    while(count--)
    {
        printf("%d", ( byte & 128 ) ? 1 : 0 );
        if(count == 4)
            putchar(' ');
        byte <<= 1;
    }
}

int main(int argc, char** argv)
{
    int image;
    pvdv1 pvd1, svd1;
    
    int rc;
    
    unsigned char vdType;
    unsigned char vdVersion;
    
    //image = open("abc.iso", O_RDONLY);
    image = open(argv[1], O_RDONLY);
    if(image == -1)
        oops("unable to open image\n");
    
    readUnused(image, NB_SYSTEM_AREA);
    
    rc = readVdTypeVer(image, &vdType, &vdVersion);
    if(rc <= 0)
        oops("problem with readVdTypeVer()");
    printf("vd type: %d, version: %d\n", vdType, vdVersion);
    if(vdType != VDTYPE_PRIMARY)
        oops("primary vd expected");
    
    rc = readPVDv1(image, &pvd1);
    if(rc <= 0)
        oops("problem with pvd1");
    
    readUnused(image, 2048);
    
    
    rc = readVdTypeVer(image, &vdType, &vdVersion);
    if(rc <= 0)
        oops("problem with readVdTypeVer()");
    printf("vd type: %d, version: %d\n", vdType, vdVersion);
    if(vdType != VDTYPE_SUPPLEMENTARY)
        oops("suppl vd expected");

    rc = readPVDv1(image, &svd1);
    if(rc <= 0)
        oops("problem with svd1");
    
    printf("sysid: \'%s\'\n", pvd1.sysId);
    printf("sysid: \'%s\'\n", svd1.sysId);
    
    printf("volid: \'%s\'\n", pvd1.volId);
    printf("volid: \'%s\'\n", svd1.volId);
    
    printf("lb size: %d\n", pvd1.lbSize);
    printf("lb size: %d\n", svd1.lbSize);

    printf("volume space size: %u\n", pvd1.volSpaceSize);
    printf("volume space size: %u\n", svd1.volSpaceSize);

    printf("human-readable volume size: %dB, %dMB, %dMiB\n", pvd1.lbSize * pvd1.volSpaceSize,
                                                             pvd1.lbSize * pvd1.volSpaceSize / 1024000,
                                                             pvd1.lbSize * pvd1.volSpaceSize / 1048576);
    printf("pathtable size: %d\n", pvd1.pathTableSize);
    printf("pathtable size: %d\n", svd1.pathTableSize);

    printf("vsid: \'%s\'\n", pvd1.volSetId);
    printf("vsid: \'%s\'\n", svd1.volSetId);
    
    printf("publ: \'%s\'\n", pvd1.publId);
    printf("publ: \'%s\'\n", svd1.publId);
    
    printf("dprp: \'%s\'\n", pvd1.dataPrepId);
    printf("dprp: \'%s\'\n", svd1.dataPrepId);
    
    printf("created: %s-%s-%s, %s:%s:%s:%s GMT%d\n", pvd1.volCreatTime.day,
                                                      pvd1.volCreatTime.month,
                                                      pvd1.volCreatTime.year,
                                                      pvd1.volCreatTime.hour,
                                                      pvd1.volCreatTime.minute,
                                                      pvd1.volCreatTime.second,
                                                      pvd1.volCreatTime.hundredthSecond,
                                                      pvd1.volCreatTime.gmtOffset);
    
    printf("created: %s-%s-%s, %s:%s:%s:%s GMT%d\n", svd1.volCreatTime.day,
                                                      svd1.volCreatTime.month,
                                                      svd1.volCreatTime.year,
                                                      svd1.volCreatTime.hour,
                                                      svd1.volCreatTime.minute,
                                                      svd1.volCreatTime.second,
                                                      svd1.volCreatTime.hundredthSecond,
                                                      svd1.volCreatTime.gmtOffset);
    
    printf("L: %d\n", pvd1.locTypeLPathTable);
    printf("L: %d\n", svd1.locTypeLPathTable);
    
    printf("M: %d\n", pvd1.locTypeMPathTable);
    printf("M: %d\n", svd1.locTypeMPathTable);
    
    close(image);
    
    return 0;
}

int read711(int file, unsigned char* value)
{
    return read(file, value, 1);
}

int read712(int file, signed char* value)
{
    return read(file, value, 1);
}

int read721(int file, unsigned short* value)
{
    return read(file, value, 2);
}

int read722(int file, unsigned short* value)
{
    int rc;
    char byte;
    
    rc = read(file, value, 2);
    if(rc != 2)
        return rc;
    
    byte = *value >> 8;
    *value <<= 8;
    *value |= byte;
    
    return rc;
}

int read723(int file, unsigned short* value)
{
    int rc;
    short trash;
    
    rc = read(file, value, 2);
    if(rc != 2)
        return rc;
    
    return read(file, &trash, 2);
}

int read731(int file, unsigned* value)
{
    return read(file, value, 4);
}

int read732(int file, unsigned* value)
{
    int rc;
    char byte2;
    char byte3;
    char byte4;
    
    rc = read(file, value, 4);
    if(rc != 4)
        return rc;
    
    byte2 = *value >> 8;
    byte3 = *value >> 16;
    byte4 = *value >> 24;
    
    *value <<= 8;
    *value |= byte2;
    *value <<= 8;
    *value |= byte3;
    *value <<= 8;
    *value |= byte4;
    
    return rc;
}

int read733(int file, unsigned* value)
{
    int rc;
    int trash;
    
    rc = read(file, value, 4);
    if(rc != 4)
        return rc;
    
    return read(file, &trash, 4);
}

int readUnused(int file, unsigned numBytes)
{
    unsigned count;
    char byte;
    int rc;
    
    for(count = 0; count < numBytes; count++)
    {
        rc = read(file, &byte, 1);
        if(rc != 1)
            return count;
    }
    
    return count;
}

/*******************************************************************************
* readVdTypeVer()
* read type and version of a volume descriptor
*
* type can be:
* - VDTYPE_BOOT
* - VDTYPE_PRIMARY
* - VDTYPE_SUPPLEMENTARY
* - VDTYPE_VOLUMEPARTITION
* - VDTYPE_TERMINATOR
*
* version can be:
* - 1
* 
* Parameters:
* - int file to read from
* - unsigned char* vd type
* - unsigned char* vd version
* Return:
* - 1 if all ok
* - -1 if failed to read anything
* - -2 if vd type unknown
* - -3 if sid not right
* - -4 if vd version unknown
*  */
int readVdTypeVer(int file, unsigned char* type, unsigned char* version)
{
    char sid[5];
    int rc;
    
    rc = read711(file, type);
    if(rc != 1)
        return -1;
    if(*type != VDTYPE_BOOT && *type != VDTYPE_PRIMARY && 
       *type != VDTYPE_SUPPLEMENTARY && *type != VDTYPE_VOLUMEPARTITION &&
       *type != VDTYPE_TERMINATOR)
        return -2;
    
    rc = read(file, sid, 5);
    if(rc != 5)
        return -1;
    if( strncmp(sid, "CD001", 5) != 0 )
        return -3;
    
    rc = read711(file, version);
    if(rc != 1)
        return -1;
    if(*version != 1)
        return -4;
    
    return 1;
}

int readPVDv1(int file, pvdv1* pvd)
{
    int rc;
    
    unsigned char fsver; /* file structure version */
    
    rc = readUnused(file, 1);
    if(rc != 1)
        return -1;
    
    rc = read(file, pvd->sysId, 32);
    if(rc != 32)
        return -1;
    pvd->sysId[32] = '\0';
    
    rc = read(file, pvd->volId, 32);
    if(rc != 32)
        return -1;
    pvd->volId[32] = '\0';
    
    rc = readUnused(file, 8);
    if(rc != 8)
        return -1;
    
    rc = read733(file, &(pvd->volSpaceSize));
    if(rc != 4)
        return -1;
    
    rc = readUnused(file, 32);
    if(rc != 32)
        return -1;
    
    rc = read723(file, &(pvd->volSetSize));
    if(rc != 2)
        return -1;
    
    rc = read723(file, &(pvd->volSeqNum));
    if(rc != 2)
        return -1;
    
    rc = read723(file, &(pvd->lbSize));
    if(rc != 2)
        return -1;
    
    rc = read733(file, &(pvd->pathTableSize));
    if(rc != 4)
        return -1;
    
    rc = read731(file, &(pvd->locTypeLPathTable));
    if(rc != 4)
        return -1;
    
    rc = read731(file, &(pvd->locOptTypeLPathTable));
    if(rc != 4)
        return -1;
    
    rc = read732(file, &(pvd->locTypeMPathTable));
    if(rc != 4)
        return -1;
    
    rc = read732(file, &(pvd->locOptTypeMPathTable));
    if(rc != 4)
        return -1;
    
    // root dir record
    readUnused(file, 34);
    
    rc = read(file, pvd->volSetId, 128);
    if(rc != 128)
        return -1;
    pvd->volSetId[128] = '\0';
    
    rc = read(file, pvd->publId, 128);
    if(rc != 128)
        return -1;
    pvd->publId[128] = '\0';

    rc = read(file, pvd->dataPrepId, 128);
    if(rc != 128)
        return -1;
    pvd->dataPrepId[128] = '\0';
    
    rc = read(file, pvd->appId, 128);
    if(rc != 128)
        return -1;
    pvd->appId[128] = '\0';
    
    rc = read(file, pvd->copyrightFid, 37);
    if(rc != 37)
        return -1;
    pvd->copyrightFid[37] = '\0';
    
    rc = read(file, pvd->abstractFid, 37);
    if(rc != 37)
        return -1;
    pvd->abstractFid[37] = '\0';
    
    rc = read(file, pvd->biblFid, 37);
    if(rc != 37)
        return -1;
    pvd->biblFid[37] = '\0';
    
    rc = read(file, pvd->volCreatTime.year, 4);
    if(rc != 4)
        return -1;
    pvd->volCreatTime.year[4] = '\0';
    
    rc = read(file, pvd->volCreatTime.month, 2);
    if(rc != 2)
        return -1;
    pvd->volCreatTime.month[2] = '\0';
    
    rc = read(file, pvd->volCreatTime.day, 2);
    if(rc != 2)
        return -1;
    pvd->volCreatTime.day[2] = '\0';
    
    rc = read(file, pvd->volCreatTime.hour, 2);
    if(rc != 2)
        return -1;
    pvd->volCreatTime.hour[2] = '\0';
    
    rc = read(file, pvd->volCreatTime.minute, 2);
    if(rc != 2)
        return -1;
    pvd->volCreatTime.minute[2] = '\0';
    
    rc = read(file, pvd->volCreatTime.second, 2);
    if(rc != 2)
        return -1;
    pvd->volCreatTime.second[2] = '\0';
    
    rc = read(file, pvd->volCreatTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    pvd->volCreatTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(pvd->volCreatTime.gmtOffset));
    if(rc != 1)
        return -1;
    
    rc = read(file, pvd->volModTime.year, 4);
    if(rc != 4)
        return -1;
    pvd->volModTime.year[4] = '\0';
    
    rc = read(file, pvd->volModTime.month, 2);
    if(rc != 2)
        return -1;
    pvd->volModTime.month[2] = '\0';
    
    rc = read(file, pvd->volModTime.day, 2);
    if(rc != 2)
        return -1;
    pvd->volModTime.day[2] = '\0';
    
    rc = read(file, pvd->volModTime.hour, 2);
    if(rc != 2)
        return -1;
    pvd->volModTime.hour[2] = '\0';
    
    rc = read(file, pvd->volModTime.minute, 2);
    if(rc != 2)
        return -1;
    pvd->volModTime.minute[2] = '\0';
    
    rc = read(file, pvd->volModTime.second, 2);
    if(rc != 2)
        return -1;
    pvd->volModTime.second[2] = '\0';
    
    rc = read(file, pvd->volModTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    pvd->volModTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(pvd->volModTime.gmtOffset));
    if(rc != 1)
        return -1;
        
    rc = read(file, pvd->volExpTime.year, 4);
    if(rc != 4)
        return -1;
    pvd->volExpTime.year[4] = '\0';
    
    rc = read(file, pvd->volExpTime.month, 2);
    if(rc != 2)
        return -1;
    pvd->volExpTime.month[2] = '\0';
    
    rc = read(file, pvd->volExpTime.day, 2);
    if(rc != 2)
        return -1;
    pvd->volExpTime.day[2] = '\0';
    
    rc = read(file, pvd->volExpTime.hour, 2);
    if(rc != 2)
        return -1;
    pvd->volExpTime.hour[2] = '\0';
    
    rc = read(file, pvd->volExpTime.minute, 2);
    if(rc != 2)
        return -1;
    pvd->volExpTime.minute[2] = '\0';
    
    rc = read(file, pvd->volExpTime.second, 2);
    if(rc != 2)
        return -1;
    pvd->volExpTime.second[2] = '\0';
    
    rc = read(file, pvd->volExpTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    pvd->volExpTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(pvd->volExpTime.gmtOffset));
    if(rc != 1)
        return -1;
        
    rc = read(file, pvd->volEffTime.year, 4);
    if(rc != 4)
        return -1;
    pvd->volEffTime.year[4] = '\0';
    
    rc = read(file, pvd->volEffTime.month, 2);
    if(rc != 2)
        return -1;
    pvd->volEffTime.month[2] = '\0';
    
    rc = read(file, pvd->volEffTime.day, 2);
    if(rc != 2)
        return -1;
    pvd->volEffTime.day[2] = '\0';
    
    rc = read(file, pvd->volEffTime.hour, 2);
    if(rc != 2)
        return -1;
    pvd->volEffTime.hour[2] = '\0';
    
    rc = read(file, pvd->volEffTime.minute, 2);
    if(rc != 2)
        return -1;
    pvd->volEffTime.minute[2] = '\0';
    
    rc = read(file, pvd->volEffTime.second, 2);
    if(rc != 2)
        return -1;
    pvd->volEffTime.second[2] = '\0';
    
    rc = read(file, pvd->volEffTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    pvd->volEffTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(pvd->volEffTime.gmtOffset));
    if(rc != 1)
        return -1;
    
    rc = read711(file, &fsver);
    if(rc != 1)
        return -1;
    if(fsver != 1)
        return -1;
    
    rc = readUnused(file, 1166);
    if(rc != 1166)
        return -1;
    
    return 1;
}
