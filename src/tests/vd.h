#ifndef vd_h
#define vd_h

#include <stdbool.h>

#define NBYTES_LOGICAL_SECTOR 2048

#define VDTYPE_BOOT 0
#define VDTYPE_PRIMARY 1
#define VDTYPE_SUPPLEMENTARY 2
#define VDTYPE_VOLUMEPARTITION 3
#define VDTYPE_TERMINATOR 255

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

typedef struct
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
    
} RootDR;

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
    RootDR rootDR;
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

int readVD(int file, Vd* pvd);
int readVDTypeVer(int file, unsigned char* type, unsigned char* version);
int readVDSet(int file, VdSet* vdset);

#endif
