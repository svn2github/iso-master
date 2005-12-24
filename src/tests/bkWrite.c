#include <unistd.h>
#include <string.h>
#include <time.h>

#include "bk.h"
#include "bkWrite7x.h"
#include "bkTime.h"

int writeByteBlock(int image, unsigned char byteToWrite, int numBytes)
{
    int rc;
    int count;
    
    for(count = 0; count < numBytes; count++)
    {
        rc = write(image, &byteToWrite, 1);
        if(rc != 1)
            return rc;
    }
    
    return count;
}

/*
* -has to be called after the files were written so that the 
*  volume size is recorded properly
* -rootdr location size is in bytes
* -note strings are not terminated on image
*/
int writePriVolDescriptor(int image, VolInfo* volInfo, unsigned rootDrLocation,
                          unsigned rootDrSize, time_t creationTime)
{
    int rc;
    int count;
    
    unsigned char byte;
    char aString[129];
    unsigned anUnsigned;
    unsigned short anUnsignedShort;
    size_t currPos;
    
    /* volume descriptor type */
    byte = 1;
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    
    /* standard identifier */
    strcpy(aString, "CD001");
    rc = write(image, &aString, 5);
    if(rc != 5)
        return -1;
    
    /* volume descriptor version (also 1) */
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    
    /* unused field */
    byte = 0;
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    
    /* system identifier (32 spaces) */
    strcpy(aString, "                                ");
    rc = write(image, &aString, 32);
    if(rc != 32)
        return -1;
    
    /* VOLUME identifier */
    strcpy(aString, volInfo->volId);
    
    for(count = strlen(aString); count < 32; count++)
        aString[count] = ' ';
    
    rc = write(image, &aString, 32);
    if(rc != 32)
        return -1;
    /* END VOLUME identifier */
    
    /* unused field */
    rc = writeByteBlock(image, 0, 8);
    if(rc != 8)
        return -1;
    
    /* VOLUME space size (number of logical blocks, absolutely everything) */
    currPos = lseek(image, 0, SEEK_CUR);
    
    lseek(image, 0, SEEK_END);
    anUnsigned = lseek(image, 0, SEEK_CUR) / NBYTES_LOGICAL_BLOCK;
    
    lseek(image, currPos, SEEK_SET);
    
    rc = write733(image, &anUnsigned);
    if(rc != 8)
        return -1;
    /* END VOLUME space size (number of logical blocks, absolutely everything) */
    
    /* unused field */
    rc = writeByteBlock(image, 0, 32);
    if(rc != 32)
        return -1;
    
    /* volume set size (always 1) */
    anUnsignedShort = 1;
    rc = write723(image, &anUnsignedShort);
    if(rc != 4)
        return -1;
    
    /* volume sequence number (also always 1) */
    rc = write723(image, &anUnsignedShort);
    if(rc != 4)
        return -1;
    
    /* logical block size (always 2048) */
    anUnsignedShort = NBYTES_LOGICAL_BLOCK;
    rc = write723(image, &anUnsignedShort);
    if(rc != 4)
        return -1;
    
    /* path table size */
    anUnsigned = 0;
    rc = write733(image, &anUnsigned);
    if(rc != 8)
        return -1;
    
    /*!! 4 path table locations (don't have them yet) */
    rc = writeByteBlock(image, 0, 16);
    if(rc != 16)
        return -1;
    
    /* ROOT dr */
        /* record length (always 34 here) */
        byte = 34;
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
        
        /* extended attribute record length (always none) */
        byte = 0;
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
        
        /* location of extent */
        anUnsigned = rootDrLocation / NBYTES_LOGICAL_BLOCK;
        rc = write733(image, &anUnsigned);
        if(rc != 8)
            return -1;
        
        /* data length */
        anUnsigned = rootDrSize;
        rc = write733(image, &anUnsigned);
        if(rc != 8)
            return -1;
        
        /* recording time */
        epochToShortString(creationTime, aString);
        rc = write(image, aString, 7);
        if(rc != 7)
            return -1;
        
        /* file flags (always binary 00000010 here) */
        byte = 0x02;
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
        
        /* file unit size (not in interleaved mode -> 0) */
        byte = 0;
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
        
        /* interleave gap size (not in interleaved mode -> 0) */
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
         
        /* volume sequence number */
        anUnsignedShort = 1;
        rc = write723(image, &anUnsignedShort);
        if(rc != 4)
            return -1;
        
        /* length of file identifier */
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
        
        /* file identifier */
        byte = 0;
        rc = write711(image, &byte);
        if(rc != 1)
            return -1;
    /* END ROOT dr */
    
    /* volume set identidier */
    rc = writeByteBlock(image, ' ', 128);
    if(rc != 128)
        return -1;
    
    /* PUBLISHER identifier */
    strcpy(aString, volInfo->publisher);
    
    for(count = strlen(aString); count < 128; count++)
        aString[count] = ' ';
    
    rc = write(image, aString, 128);
    if(rc != 128)
        return -1;
    /* PUBLISHER identifier */
    
    /* DATA preparer identifier */
    rc = write(image, "ISO Master", 10);
    if(rc != 10)
        return -1;
    
    rc = writeByteBlock(image, ' ', 118);
    if(rc != 118)
        return -1;
    /* END DATA preparer identifier */
    
    /* application identifier, copyright file identifier, abstract file 
    * identifier, bibliographic file identifier (128 + 4*37) */
    rc = writeByteBlock(image, ' ', 239);
    if(rc != 239)
        return -1;
    
    /* VOLUME creation date */
    epochToLongString(creationTime, aString);
    
    rc = write(image, aString, 17);
    if(rc != 17)
        return -1;
    /* END VOLUME creation date */
    
    /* volume modification date (same as creation) */
    rc = write(image, aString, 17);
    if(rc != 17)
        return -1;
    
    /* VOLUME expiration date (none) */
    rc = writeByteBlock(image, '0', 16);
    if(rc != 16)
        return -1;
    
    byte = 0;
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    /* END VOLUME expiration date (none) */
    
    /* volume effective date (same as creation) */
    rc = write(image, aString, 17);
    if(rc != 17)
        return -1;
    
    /* file structure version */
    byte = 1;
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    
    /* reserved, applications use, reserved */
    rc = writeByteBlock(image, 0, 1166);
    if(rc != 1166)
        return -1;
    
    return 1;
}

int writeVdsetTerminator(int image)
{
    int rc;
    unsigned char byte;
    char aString[6];
    
    /* volume descriptor type */
    byte = 255;
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    
    /* standard identifier */
    strcpy(aString, "CD001");
    rc = write(image, &aString, 5);
    if(rc != 5)
        return -1;
    
    /* volume descriptor version */
    byte = 1;
    rc = write711(image, &byte);
    if(rc != 1)
        return -1;
    
    rc = writeByteBlock(image, 0, 2041);
    if(rc != 2041)
        return -1;
    
    return 1;
}
