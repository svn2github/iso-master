#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

//#define TEST_READ_DR

#define NLS_SYSTEM_AREA 16
#define NBYTES_LOGICAL_SECTOR 2048

#define VDTYPE_BOOT 0
#define VDTYPE_PRIMARY 1
#define VDTYPE_SUPPLEMENTARY 2
#define VDTYPE_VOLUMEPARTITION 3
#define VDTYPE_TERMINATOR 255

#define DRTYPE_9660 0
#define DRTYPE_JOLIET 1

#define SEPARATOR1 0x2E
#define SEPARATOR2 0x3B

#define UCSTYPE_LEVEL1 0
#define UCSTYPE_LEVEL2 1
#define UCSTYPE_LEVEL3 2

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
    int suFieldsLen;
    char suFields[223]; /* max 256 - 33 (remember minus fullNameLen) */
    struct Dir* dir; /* if a directory, otherwise null */
    
};

/* linked list because i don't know how many children a dir has
*  until i read them all */
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
    struct DrLL* children;
    
};

/* good for either a primary or a secondary volume descriptor
* whomever uses the pvd should know to ignore volumeFlags and escapeSequences
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
    bool haveSvd;
    Vd svd;
    // volume partition descriptors
    // boot records
    
} VdSet;

void displayDirTree(struct Dir* tree, bool isJoliet, int level);

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
int readSuFields(int file, struct Dr* dr);
int readVD(int file, Vd* pvd);
int readVDTypeVer(int file, unsigned char* type, unsigned char* version);
int readVDSet(int file, VdSet* vdset);

int svdGetJolietType(Vd* svd);

int main(int argc, char** argv)
{
    int image;
    VdSet vdset;
    int rc;
    
    struct Dir tree;
    
    /* open image file for reading */
    image = open(argv[1], O_RDONLY);
    if(image == -1)
        oops("unable to open image\n");
    
    /* system area */
    rc = readUnused(image, NLS_SYSTEM_AREA * NBYTES_LOGICAL_SECTOR);
    if(rc <= 0)
        oops("problem with system area()");
    
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
    
    printf("joliet type: %d\n", svdGetJolietType(&(vdset.svd)));
    printf("joliet root extent at: %d\n", vdset.svd.rootDR.locExtent);
    
    /* read directory tree */
    lseek(image, NBYTES_LOGICAL_SECTOR * vdset.pvd.rootDR.locExtent, SEEK_SET);
    
    tree.children = NULL;
    rc = readDir(image, &tree);
    if(rc <= 0)
        oops("failed to read tree");
    printf("tree: total read %d bytes\n", rc);
    
    displayDirTree(&tree, false, 0);
    
    close(image);
    
    return 0;
}

void displayDirTree(struct Dir* tree, bool isJoliet, int level)
{
    int count;
    struct DrLL* entry;
    
    entry = tree->children;
    while(entry != NULL)
    {
        /* display file or subdirectory name */
        for(count = 0; count < level * 2; count++)
            printf(" ");
        
        if(!isJoliet)
        {
            printf("%s", entry->dr.fullName);
            printf("\n");
        }
        else
        {
            printUCS2(entry->dr.fullName, 128);
            printf("\n");
        }
        
        if( drisadir(&(entry->dr)) )
        /* display subdirectory */
            displayDirTree(entry->dr.dir, isJoliet, level + 1);
        
        entry = entry->next;
    }
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
        if(count == 4)
            putchar(' ');
        byte <<= 1;
    }
}

void printUCS2(char* ucsString, int numBytes)
{
    int count;
    printf("%d", numBytes);
    for(count = 0; count < numBytes; count += 2)
    {
        if( ucsString[count] == 0 && ucsString[count + 1] == 0 )
            break;
        
        printf("%c", ucsString[count + 1]);
    }
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
    //int unusedNB;
    
    #ifdef TEST_READ_DR
        printf("readDR: ");
    #endif
    
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
    
    #ifdef TEST_READ_DR
        printf("extent %d, ", dr->locExtent);
    #endif
    
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

    #ifdef TEST_READ_DR
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
            //~ printf("name: ");
            //~ printUCS2(dr->fullName, dr->fullNameLen);
            //~ printf(", ");
        }
    #endif
        
    if(dr->fullNameLen % 2 == 0)
    {
        rc = readUnused(file, 1);
        if(rc != 1)
            return -1;
        count += 1;
    }
    
    dr->suFieldsLen = dr->recordLength - (33 + dr->fullNameLen + (dr->fullNameLen % 2 == 0 ? 1 : 0));
    if(dr->suFieldsLen > 0)
    {
        /* !! this may modify dr->recordLength and dr->suFieldsLen !! */
        //rc = readSuFields(file, dr);
        //rc = readUnused(file, dr->suFieldsLen);
        rc = read(file, dr->suFields, dr->suFieldsLen);
        if(rc < 0)
            return -1;
        
        count += rc;
    }
    
    if( drisadir(dr) && !drDescribesSelf(dr)&& !drDescribesParent(dr) )
    /* move the file pointer to the location of the new directory
    *  and back when it's finished */
    {
        off_t origPos;
        
        origPos = lseek(file, 0, SEEK_CUR);
        
        lseek(file, NBYTES_LOGICAL_SECTOR * dr->locExtent, SEEK_SET);
        
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
    
    #ifdef TEST_READ_DR
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
            //~ printf("end ");
            //~ printUCS2(dr->fullName, dr->fullNameLen);
            //~ putchar('\n');
        }
    #endif
        
    return count;
}

/* 
* allowed fields:
* -SP
* -ER (ignored and rewritten)
* -PX (regular file)
* -NM 
* */
int readSuFields(int file, struct Dr* dr)
{
    /* read an su record
    *  decide whether it should stay
    *  write to suFields */
    
    int rc;
    unsigned char allFields[256];
    int offset1 = 0; /* for allFields (source) */
    int offset2 = 0; /* for suFields (destination) */
    int count;
    
    rc = read(file, allFields, dr->suFieldsLen);
    if(rc != dr->suFieldsLen)
        return -1;
    
    // !! check that suFieldsLen is not too big
    
    while(offset1 < dr->suFieldsLen)
    {
        if(allFields[offset1] == 0)
        /* padding zero */
            break;
        
        if( (allFields[offset1] == 'S' && allFields[offset1 + 1] == 'P') ||
            (allFields[offset1] == 'P' && allFields[offset1 + 1] == 'X') ||
            (allFields[offset1] == 'N' && allFields[offset1 + 1] == 'M') )
        {
            /* copy record */
            for(count = 0; count < allFields[offset1 + 2]; count++)
            {
                (dr->suFields)[offset2 + count] = allFields[offset1 + count];
            }
            
            offset2 += count;
        }
        /* else skip record */
        
        offset1 += allFields[offset1 + 2];
    }
    
    /* offset2 is now the new su fields length */
    
    if( (dr->recordLength - dr->suFieldsLen + offset2) % 2 == 1 )
    /* add a padding 0x00 to make it even (9660 spec) */
    {
        offset2 += 1;
        (dr->suFields)[offset2] = 0x00;
    }
    printf("%d %d -> ", dr->recordLength, dr->suFieldsLen);
    /* original length was dr->recordLength
    *  original su fields length was dr->suFieldsLen
    *  new length is dr->recordLength - dr->suFieldsLen + offset2 */
    dr->recordLength = dr->recordLength - dr->suFieldsLen + offset2;
    dr->suFieldsLen = offset2;
    
    printf("%d %d\n", dr->recordLength, dr->suFieldsLen);
    
    return rc;
}

int readDir(int file, struct Dir* dir)
{
    int rc;
    int bytesRead = 0;
    int childrenBytesRead;
    struct DrLL* last;
    
    #ifdef TEST_READ_DR
        printf("\n");
    #endif

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
    
    if(dir->numEntries > 0)
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
    if(pvd->lbSize != NBYTES_LOGICAL_SECTOR)
        return -2;
    
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
    
    /* the rest of the NBYTES_LOGICAL_SECTOR bytes, always this size for a pvd */
    rc = readUnused(file, 1166); 
    if(rc != 1166)
        return -1;
    
    return 2041;
}

/*
* this will:
* - read one pvd
* - skip everything before next
* - read one svd
* - skip everything before next
* - read terminator
* */
int readVDSet(int file, VdSet* vdset)
{
    int rc;
    unsigned char vdType;
    unsigned char vdVersion;
    int bytesRead = 0;
    bool keepGoing;
    
    /* READ PVD */
    rc = readVDTypeVer(file, &vdType, &vdVersion);
    if(rc <= 0)
        return -1;
    bytesRead += rc;
    
    if(vdType != VDTYPE_PRIMARY)
        return -2;
    
    rc = readVD(file, &(vdset->pvd));
    if(rc <= 0)
        return -1;
    bytesRead += rc;
    /* END READ PVD */
    
    vdset->haveSvd = false;
    keepGoing = true;
    do
    {
        rc = readVDTypeVer(file, &vdType, &vdVersion);
        if(rc <= 0)
            return -1;
        bytesRead += rc;
        
        if(vdType == VDTYPE_TERMINATOR)
        {
            rc = readUnused(file, NBYTES_LOGICAL_SECTOR - 7);
            if(rc != NBYTES_LOGICAL_SECTOR - 7)
                return -1;
            bytesRead += rc;
            keepGoing = false;
        }
        else if(vdType == VDTYPE_SUPPLEMENTARY && vdset->haveSvd == false)
        /* will only read one svd (should be joliet btw) */
        {
            vdset->haveSvd = true;
            rc = readVD(file, &(vdset->svd));
            if(rc <= 0)
                return -1;
            bytesRead += rc;
        }
        else
        /* ignore all other vds */
        {
            rc = readUnused(file, NBYTES_LOGICAL_SECTOR - 7);
            if(rc != NBYTES_LOGICAL_SECTOR - 7)
                return -1;
            bytesRead += rc;
        }
        
    } while (keepGoing);
    
    return bytesRead;
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
* - 7 (bytes read) if all ok
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

/*
* a joliet svd will have:
* -bit 0 of volumeFlags set to 0 (only registered escape sequences)
* -one of the following in escapeSequences[] and no others:
* UCS-2 Level 1: (25)(2F)(40) = "%\@"
* UCS-2 Level 2: (25)(2F)(43) = "%\C"
* UCS-2 Level 3: (25)(2F)(45) = "%\E"
* 
* returns one of:
* -1 (not valid joliet)
* UCSTYPE_LEVEL1
* UCSTYPE_LEVEL2
* UCSTYPE_LEVEL3
*/
int svdGetJolietType(Vd* svd)
{
    if( ((svd->volumeFlags >> 1) & 1) != 0 )
        return -1;
    
    if(svd->escapeSequences[0] == 0x25 &&
       svd->escapeSequences[1] == 0x2F &&
       svd->escapeSequences[2] == 0x40)
    {
        return UCSTYPE_LEVEL1;
    }
    else if(svd->escapeSequences[0] == 0x25 &&
            svd->escapeSequences[1] == 0x2F &&
            svd->escapeSequences[2] == 0x43)
    {
        return UCSTYPE_LEVEL2;
    }
    else if(svd->escapeSequences[0] == 0x25 &&
            svd->escapeSequences[1] == 0x2F &&
            svd->escapeSequences[2] == 0x45)
    {
        return UCSTYPE_LEVEL3;
    }
    else
        return -1;
}
