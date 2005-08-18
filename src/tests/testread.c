#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define NB_SYSTEM_AREA 32768

#define VDTYPE_BOOT 0
#define VDTYPE_PRIMARY 1
#define VDTYPE_SUPPLEMENTARY 2
#define VDTYPE_VOLUMEPARTITION 3
#define VDTYPE_TERMINATOR 255

#define DRTYPE_9660 0
#define DRTYPE_JOLIET 1

#define SEPARATOR1 0x2E
#define SEPARATOR2 0x3B

/* date/time for volume descriptors
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
    
} DateTimeVD;

/* date/time for file and directory descriptors */
typedef struct
{
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    signed char gmtOffset;
    
} DateTimeDR;

struct Dr
{
    unsigned char recordLength;
    unsigned char extAttrRecLen;
    unsigned locExtent;
    unsigned dataLength;
    DateTimeDR recordedTime;
    unsigned char fileFlags;
    unsigned char fileUnitSize; /* interleaved mode only */
    unsigned char interleaveGapSize; /* interleaved mode only */
    unsigned short volSeqNum;
    unsigned char fullNameLen; /* name + extension + separators + version */
    char fullName[128]; /* exactly as read (128 max for joliet) */
    struct Dir* dir;
    
};

struct DrLL
{
    struct Dr dr;
    struct DrLL* next;
    
};

struct Dir
{
    struct Dr self;
    struct Dr parent;
    int numEntries; /* not including self and parent */
    //Dr* child; /* all the rest of the children */
    struct DrLL* children;
        
};

/* primary volume descriptor version 1 (original iso9660 standard)
* all strings are size +1 for the null byte */
typedef struct
{
    unsigned char volumeFlags; /* only used in svd */
    char sysId[33];
    char volId[33];
    unsigned volSpaceSize;
    char escapeSequences[32]; /* only used in svd */
    unsigned short volSetSize;
    unsigned short volSeqNum;
    unsigned short lbSize;
    unsigned pathTableSize;
    unsigned locTypeLPathTable;
    unsigned locOptTypeLPathTable;
    unsigned locTypeMPathTable;
    unsigned locOptTypeMPathTable;
    struct Dr rootDR;
    char volSetId[129];
    char publId[129];
    char dataPrepId[129];
    char appId[129];
    char copyrightFid[38];
    char abstractFid[38];
    char biblFid[38];
    DateTimeVD volCreatTime;
    DateTimeVD volModTime;
    DateTimeVD volExpTime;
    DateTimeVD volEffTime;
    
} Vd;

typedef struct
{
    Vd pvd;
    int numSvds;
    
    
} VdSet;

bool drDescribesParent(struct Dr* dr);
bool drDescribesSelf(struct Dr* dr);
bool drisadir(struct Dr* dr);
bool haveNextRecordInSector(int file);
void oops(char* msg);
void printByte(char byte);
void printUCS2(char* ucsString, int numBytes);

int read711(int file, unsigned char* value);
int read712(int file, signed char* value);
int read721(int file, unsigned short* value);
int read722(int file, unsigned short* value);
int read723(int file, unsigned short* value);
int read731(int file, unsigned* value);
int read732(int file, unsigned* value);
int read733(int file, unsigned* value);

int readUnused(int file, unsigned numBytes);

int readDir(int file, struct Dir* dir);
int readDR(int file, struct Dr* dr);
int reaVD(int file, Vd* pvd);
int readVDTypeVer(int file, unsigned char* type, unsigned char* version);

int main(int argc, char** argv)
{
    int image;
    Vd pvd1;
    //Vd svd1;
    int rc;
    
    /*struct Dr dirRec;
    char str[256];
    int count;
    */
    
    struct Dir tree;
    
    unsigned char vdType;
    unsigned char vdVersion;
    
    image = open(argv[1], O_RDONLY);
    if(image == -1)
        oops("unable to open image\n");
    
    rc = readUnused(image, NB_SYSTEM_AREA);
    if(rc <= 0)
        oops("problem with system area()");
    
    rc = readVDTypeVer(image, &vdType, &vdVersion);
    if(rc <= 0)
        oops("problem with readVDTypeVer()");
    printf("vd type: %d, version: %d\n", vdType, vdVersion);
    if(vdType != VDTYPE_PRIMARY)
        oops("primary vd expected");
    
    rc = readVD(image, &pvd1);
    if(rc <= 0)
        oops("problem with pvd1");
    
    //~ readUnused(image, 2048);
    
    //~ rc = readVDTypeVer(image, &vdType, &vdVersion);
    //~ if(rc <= 0)
        //~ oops("problem with readVDTypeVer()");
    //~ printf("vd type: %d, version: %d\n", vdType, vdVersion);
    //~ if(vdType != VDTYPE_SUPPLEMENTARY)
        //~ oops("suppl vd expected");

    //~ rc = readVD(image, &svd1);
    //~ if(rc <= 0)
        //~ oops("problem with svd1");
    
    printf("sysid: \'%s\'\n", pvd1.sysId);
    //~ printf("sysid: ");printUCS2(svd1.sysId, 128);putchar('\n');
    
    printf("volid: \'%s\'\n", pvd1.volId);
    //~ printf("volid: ");printUCS2(svd1.volId, 128);putchar('\n');
    
    printf("lb size: %d\n", pvd1.lbSize);
    //~ printf("lb size: %d\n", svd1.lbSize);

    printf("volume space size: %u\n", pvd1.volSpaceSize);
    //~ printf("volume space size: %u\n", svd1.volSpaceSize);

    printf("human-readable volume size: %dB, %dMB, %dMiB\n", pvd1.lbSize * pvd1.volSpaceSize,
                                                             pvd1.lbSize * pvd1.volSpaceSize / 1024000,
                                                             pvd1.lbSize * pvd1.volSpaceSize / 1048576);
    printf("pathtable size: %d\n", pvd1.pathTableSize);
    //~ printf("pathtable size: %d\n", svd1.pathTableSize);

    printf("vsid: \'%s\'\n", pvd1.volSetId);
    //~ printf("vsid: ");printUCS2(svd1.volSetId, 128);putchar('\n');
    
    printf("publ: \'%s\'\n", pvd1.publId);
    //~ printf("publ: ");printUCS2(svd1.publId, 128);putchar('\n');
    
    printf("dprp: \'%s\'\n", pvd1.dataPrepId);
    //~ printf("dprp: ");printUCS2(svd1.dataPrepId, 128);putchar('\n');
    
    printf("created: %s-%s-%s, %s:%s:%s:%s GMT%d\n", pvd1.volCreatTime.day,
                                                     pvd1.volCreatTime.month,
                                                     pvd1.volCreatTime.year,
                                                     pvd1.volCreatTime.hour,
                                                     pvd1.volCreatTime.minute,
                                                     pvd1.volCreatTime.second,
                                                     pvd1.volCreatTime.hundredthSecond,
                                                     pvd1.volCreatTime.gmtOffset);
    
    //~ printf("created: %s-%s-%s, %s:%s:%s:%s GMT%d\n", svd1.volCreatTime.day,
                                                      //~ svd1.volCreatTime.month,
                                                      //~ svd1.volCreatTime.year,
                                                      //~ svd1.volCreatTime.hour,
                                                      //~ svd1.volCreatTime.minute,
                                                      //~ svd1.volCreatTime.second,
                                                      //~ svd1.volCreatTime.hundredthSecond,
                                                      //~ svd1.volCreatTime.gmtOffset);
    
    printf("L: %d\n", pvd1.locTypeLPathTable);
    //~ printf("L: %d\n", svd1.locTypeLPathTable);
    
    printf("M: %d\n", pvd1.locTypeMPathTable);
    //~ printf("M: %d\n", svd1.locTypeMPathTable);
    
    printf("root extent at: %d\n", pvd1.rootDR.locExtent);
    //~ printf("root extent at: %d\n", svd1.rootDR.locExtent);
    
    printf("data length; %d\n", pvd1.rootDR.dataLength);
    //~ printf("data length; %d\n", svd1.rootDR.dataLength);

    lseek(image, 2048 * pvd1.rootDR.locExtent, SEEK_SET);
    
    rc = readDir(image, &tree);
    if(rc <= 0)
        oops("failed to read tree");
    printf("TOTAL READ %d bytes\n", rc);
    
    close(image);
    
    return 0;
}

bool drDescribesParent(struct Dr* dr)
{
    if(dr->fullNameLen == 1 && dr->fullName[0] == 0x01)
        return true;
    else
        return false;
}

bool drDescribesSelf(struct Dr* dr)
{
    if(dr->fullNameLen == 1 && dr->fullName[0] == 0x00)
        return true;
    else
        return false;
}

bool drisadir(struct Dr* dr)
{
    if( dr->fileFlags >> 1 & 1 )
        return true;
    else
        return false;
}

/* if the next byte is zero returns false otherwise true
* file position remains unchanged
* returns false on read error */
bool haveNextRecordInSector(int file)
{
    off_t origPos;
    char testByte;
    int rc;
    
    origPos = lseek(file, 0, SEEK_CUR);
    
    rc = read(file, &testByte, 1);
    if(rc != 1)
        return false;
    
    lseek(file, origPos, SEEK_SET);
    
    return (testByte == 0) ? false : true;
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

void printUCS2(char* ucsString, int numBytes)
{
    int count;
    
    putchar('\'');
    
    for(count = 0; count < numBytes; count++)
    {
        if( ucsString[count] == 0 && ucsString[count + 1] == 0 )
            break;
        
        printf("%c", ucsString[count + 1]);
    }
    
    printf("\' (UCS-2)");
}

void oops(char* msg)
{
    fprintf(stderr, "OOPS, %s\n", msg);
    exit(0);
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
    int count;
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

int readDR(int file, struct Dr* dr)
{
    int rc;
    int count = 0;
    int unusedNB;
    printf("readDR: ");
    rc = read711(file, &(dr->recordLength));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &(dr->extAttrRecLen));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read733(file, &(dr->locExtent));
    if(rc != 4)
        return -1;
    count += 8;
    printf("extent %d, ", dr->locExtent);
    rc = read733(file, &(dr->dataLength));
    if(rc != 4)
        return -1;
    count += 8;
    
    /* BEGIN read time */    
    rc = read711(file, &((dr->recordedTime).year));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &((dr->recordedTime).month));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &((dr->recordedTime).day));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &((dr->recordedTime).hour));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &((dr->recordedTime).minute));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &((dr->recordedTime).second));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read712(file, &((dr->recordedTime).gmtOffset));
    if(rc != 1)
        return -1;
    count += 1;
    /* END read time */
    
    rc = read711(file, &(dr->fileFlags));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &(dr->fileUnitSize));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read711(file, &(dr->interleaveGapSize));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read723(file, &(dr->volSeqNum));
    if(rc != 2)
        return -1;
    count += 4;
    
    rc = read711(file, &(dr->fullNameLen));
    if(rc != 1)
        return -1;
    count += 1;
    
    rc = read(file, dr->fullName, dr->fullNameLen);
    if(rc != dr->fullNameLen)
        return -1;
    count += dr->fullNameLen;
    if(drDescribesSelf(dr))
        printf("name: SELF, ");
    else if(drDescribesParent(dr))
        printf("name: PARENT, ");
    else
    {
        char temp[200];
        strncpy(temp, dr->fullName, dr->fullNameLen);
        dr->fullName[dr->fullNameLen] = '\0';
        printf("name: %s, ", dr->fullName);
    }
    if(dr->fullNameLen % 2 == 0)
    {
        rc = readUnused(file, 1);
        if(rc != 1)
            return -1;
        count += 1;
    }
    
    unusedNB = dr->recordLength - (33 + dr->fullNameLen + (dr->fullNameLen % 2 == 0 ? 1 : 0));
    if(unusedNB > 0)
    {
        rc = readUnused(file, unusedNB);
        if(rc != unusedNB)
            return -1;
        count += unusedNB;
    }
    
    if( drisadir(dr) && !drDescribesSelf(dr)&& !drDescribesParent(dr) )
    /* move the file pointer to the location of the new directory
    *  and back when it's finished */
    {
        off_t origPos;
        
        origPos = lseek(file, 0, SEEK_CUR);
        
        lseek(file, 2048 * dr->locExtent, SEEK_SET);
        
        dr->dir = (struct Dir*)malloc(sizeof(struct Dir));
        if(dr->dir == NULL)
            return -2;
        
        dr->dir->children = NULL;
        rc = readDir( file, dr->dir );
        if(rc <= 0)
            return -1;
        
        lseek(file, origPos, SEEK_SET);
        
        count += rc;
    }
    else
    {
        dr->dir = NULL;
    }
    if(drDescribesSelf(dr))
        printf("end SELF\n");
    else if(drDescribesParent(dr))
        printf("end PARENT\n");
    else
    {
        char temp[200];
        strncpy(temp, dr->fullName, dr->fullNameLen);
        dr->fullName[dr->fullNameLen] = '\0';
        printf("end %s\n", dr->fullName);
    }
    return count;
}

int readDir(int file, struct Dir* dir)
{
    printf("\n");
    int rc;
    int bytesRead = 0;
    int childrenBytesRead;
    struct DrLL* last;
    
    /* read self */
    rc = readDR(file, &(dir->self));
    if(rc <= 0)
        return -1;
    bytesRead += rc;
    
    /* read parent */
    rc = readDR(file, &(dir->parent));
    if(rc <= 0)
        return -1;
    bytesRead += rc;
    
    /* BEGIN READ CHILDREN and increase numEntries */
    childrenBytesRead = 0;
    while(childrenBytesRead + bytesRead < dir->self.dataLength)
    {
        if(haveNextRecordInSector(file))
        /* read it */
        {
            if(dir->children == NULL)
            /* first one in dir */
            {
                dir->children = (struct DrLL*)malloc(sizeof(struct DrLL));
                if(dir->children == NULL)
                    return -2;
                last = dir->children;
            }
            else
            {
                last->next = (struct DrLL*)malloc(sizeof(struct DrLL));
                if(last->next == NULL)
                    return -2;
                last = last->next;
            }
            
            rc = readDR(file, &(last->dr));
            if(rc <= 0)
                return -1;
            childrenBytesRead += last->dr.recordLength;
            
            dir->numEntries++;
        }
        else
        /* read zeroes until get to next record (that would be in the next
        *  sector btw) or get to the end of data (dir->self.dataLength) */
        {
            char testByte;
            off_t origPos;
            
            do
            {
                origPos = lseek(file, 0, SEEK_CUR);
                
                rc = read(file, &testByte, 1);
                if(rc != 1)
                    return -1;
                
                if(testByte != 0)
                {
                    lseek(file, origPos, SEEK_SET);
                    break;
                }
                
                childrenBytesRead += 1;
                
            } while (childrenBytesRead + bytesRead < dir->self.dataLength);
        }
    }
    /* END READ CHILDREN and increase numEntries */
    
    if(dir->numEntries == 0)
    /* read no children */
        dir->children = NULL; //!! this is already done in readDR
    else
    /* terminate DrLL* dir->children */
        last->next = NULL;
    
    bytesRead += childrenBytesRead;
    
    return bytesRead;
}

int readVD(int file, Vd* pvd)
{
    int rc;
    unsigned char fsver; /* file structure version */
    
    /*rc = readUnused(file, 1);
    if(rc != 1)
        return -1;*/
    rc = read(file, &(pvd->volumeFlags), 1);
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

    /*rc = readUnused(file, 32);
    if(rc != 32)
        return -1;*/
    rc = read(file, pvd->escapeSequences, 32);
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
    
    rc = readDR(file, &(pvd->rootDR));
    if(rc != 34)
        return -1;
    
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
    if(fsver != 1) /* should be 1 for a pvd */
        return -1;
    
    /* the rest of the 2048 bytes, always this size for a pvd */
    rc = readUnused(file, 1166); 
    if(rc != 1166)
        return -1;
    
    return 2041;
}

/*******************************************************************************
* readVDTypeVer()
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
int readVDTypeVer(int file, unsigned char* type, unsigned char* version)
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
    
    return 7;
}
