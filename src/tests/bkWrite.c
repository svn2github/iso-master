#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "bk.h"
#include "bkWrite7x.h"
#include "bkTime.h"
#include "bkWrite.h"

int writeByteBlock(int image, unsigned char byteToWrite, int numBytes)
{
    int rc;
    int count;
    
    for(count = 0; count < numBytes; count++)
    {
        rc = write(image, &byteToWrite, 1);
        if(rc != 1)
            return -1;
    }
    
    return count;
}

int writeDir(int image, DirToWrite* dir, int parentLbNum, int parentNumBytes, 
             int parentPosix, time_t recordingTime, int filenameTypes,
             bool isRoot)
{
    int rc;
    
    off_t startPos;
    int numUnusedBytes;
    off_t endPos;
    
    DirToWrite selfDir;
    DirToWrite parentDir;
    
    bool takeDirNext;
    DirToWriteLL* nextDir;
    FileToWriteLL* nextFile;
    
    selfDir.name9660[0] = 0x00;
    selfDir.posixFileMode = dir->posixFileMode;
    
    parentDir.name9660[0] = 0x01;
    parentDir.name9660[1] = '\0';
    // joliet, rockridge
    if(isRoot)
        parentDir.posixFileMode = selfDir.posixFileMode;
    else
        parentDir.posixFileMode = parentPosix;
    
    startPos = lseek(image, 0, SEEK_CUR);
    
    if( startPos % NBYTES_LOGICAL_BLOCK != 0 )
    /* this should never happen */
        return -10;
    
    selfDir.extentNumber = startPos / NBYTES_LOGICAL_BLOCK;
    
    /* write self */
    if(isRoot)
        rc = writeDr(image, &selfDir, recordingTime, true, true, true, filenameTypes);
    else
        rc = writeDr(image, &selfDir, recordingTime, true, true, false, filenameTypes);
    if(rc < 0)
        return rc;
    
    /* write parent */
    rc = writeDr(image, &parentDir, recordingTime, true, true, false, filenameTypes);
    if(rc < 0)
        return rc;
    
    nextDir = dir->directories;
    nextFile = dir->files;
    
    /* write children drs */
    while( nextDir != NULL || nextFile != NULL )
    /* have a file or directory */
    {
        if(nextDir == NULL)
        /* no directories left */
            takeDirNext = false;
        else if(nextFile == NULL)
        /* no files left */
            takeDirNext = true;
        else
        {
            if( strcmp(nextFile->file.name9660, nextDir->dir.name9660) > 0 )
                takeDirNext = true;
            else
                takeDirNext = false;
        }
        
        if(takeDirNext)
        {
            rc = writeDr(image, &(nextDir->dir), recordingTime, 
                         true,  false, false, filenameTypes);
            if(rc < 0)
                return rc;
            
            nextDir = nextDir->next;
        }
        else
        {
            rc = writeDr(image, (DirToWrite*)&(nextFile->file), recordingTime, 
                         false,  false, false, filenameTypes);
            if(rc < 0)
                return rc;
            
            nextFile = nextFile->next;fflush(NULL);
        }
    }
    
    /* write blank to conclude extent */
    numUnusedBytes = NBYTES_LOGICAL_BLOCK - 
                     lseek(image, 0, SEEK_CUR) % NBYTES_LOGICAL_BLOCK;
    rc = writeByteBlock(image, 0x00, numUnusedBytes);
    if(rc != numUnusedBytes)
        return rc;
    
    selfDir.dataLength = lseek(image, 0, SEEK_CUR) - startPos;
    
    nextDir = dir->directories;
    while(nextDir != NULL)
    {
        nextDir->dir.extentNumber = lseek(image, 0, SEEK_CUR) / NBYTES_LOGICAL_BLOCK;
        
        rc = writeDir(image, &(nextDir->dir), selfDir.extentNumber, 
                      selfDir.dataLength, dir->posixFileMode, recordingTime,
                      filenameTypes, false);
        if(rc < 0)
            return rc;
        
        nextDir->dir.dataLength = lseek(image, 0, SEEK_CUR) - 
                                  nextDir->dir.extentNumber * NBYTES_LOGICAL_BLOCK;
        
        nextDir = nextDir->next;
    }
    
    endPos = lseek(image, 0, SEEK_CUR);
    
    /* SELF extent location and size */
    lseek(image, selfDir.extentLocationOffset, SEEK_SET);
    
    rc = write733(image, &(selfDir.extentNumber));
    if(rc < 0)
        return rc;
    
    rc = write733(image, &(selfDir.dataLength));
    if(rc < 0)
        return rc;
    /* END SELF extent location and size */
    
    /* PARENT extent location and size */
    lseek(image, parentDir.extentLocationOffset, SEEK_SET);
    
    if(parentLbNum == 0)
    /* root, parent is same as self */
    {
        rc = write733(image, &(selfDir.extentNumber));
        if(rc < 0)
            return rc;
        
        rc = write733(image, &(selfDir.dataLength));
        if(rc < 0)
            return rc;
    }
    else
    /* normal parent */
    {
        rc = write733(image, &(parentLbNum));
        if(rc < 0)
            return rc;
        
        rc = write733(image, &(parentNumBytes));
        if(rc < 0)
            return rc;
    }
    /* END PARENT extent location and size */
    
    /* ALL subdir extent locations and sizes */
    nextDir = dir->directories;
    while(nextDir != NULL)
    {
        lseek(image, nextDir->dir.extentLocationOffset, SEEK_SET);
        
        rc = write733(image, &(nextDir->dir.extentNumber));
        if(rc < 0)
            return rc;
        
        rc = write733(image, &(nextDir->dir.dataLength));
        if(rc < 0)
            return rc;
        
        nextDir = nextDir->next;
    }
    /* ALL subdir extent locations and sizes */
    
    lseek(image, endPos, SEEK_SET);
    
    return 1;
}

int writeDr(int image, DirToWrite* dir, time_t recordingTime, bool isADir, 
            bool isSelfOrParent, bool isFirstRecord, int filenameTypes)
{
    int rc;
    unsigned char byte;
    char aString[256];
    unsigned short aShort;
    off_t startPos;
    off_t endPos;
    unsigned char lenFileId;
    
    startPos = lseek(image, 0, SEEK_CUR);
    
    /* record length is recorded in the end */
    lseek(image, 1, SEEK_CUR);
    
    /* extended attribute record length */
    byte = 0;
    rc = write711(image, &byte);
    if(rc != 1)
        return rc;
    
    /* location of extent (logical bock number)
    * not recorded in this function */
    dir->extentLocationOffset = lseek(image, 0, SEEK_CUR);
    lseek(image, 8, SEEK_CUR);
    
    /* data length (number of bytes)
    * not recorded in this function */
    lseek(image, 8, SEEK_CUR);
    
    /* RECORDING time and date */
    epochToShortString(recordingTime, aString);
    
    rc = write711(image, &(aString[0]));
    if(rc != 1)
        return rc;
    rc = write711(image, &(aString[1]));
    if(rc != 1)
        return rc;
    rc = write711(image, &(aString[2]));
    if(rc != 1)
        return rc;
    rc = write711(image, &(aString[3]));
    if(rc != 1)
        return rc;
    rc = write711(image, &(aString[4]));
    if(rc != 1)
        return rc;
    rc = write711(image, &(aString[5]));
    if(rc != 1)
        return rc;
    rc = write711(image, &(aString[6]));
    if(rc != 1)
        return rc;
    /* END RECORDING time and date */
    
    /* FILE flags  */
    if(isADir)
    /* (only directory bit on) */
        byte = 0x02;
    else
    /* nothing on */
        byte = 0x00;
    
    rc = write(image, &byte, 1);
    if(rc != 1)
        return -1;
    /* END FILE flags  */
    
    /* file unit size (always 0, non-interleaved mode) */
    byte = 0;
    rc = write711(image, &byte);
    if(rc != 1)
        return rc;
    
    /* interleave gap size (also always 0, non-interleaved mode) */
    rc = write711(image, &byte);
    if(rc != 1)
        return rc;
    
    /* volume sequence number (always 1) */
    aShort = 1;
    rc = write723(image, &aShort);
    if(rc != 4)
        return rc;
    
    /* LENGTH of file identifier */
    if(dir->name9660[0] == 0x00 || dir->name9660[0] == 0x01)
    /* self or parent */
        lenFileId = 1;
    else
        lenFileId = strlen(dir->name9660);
    
    rc = write711(image, &lenFileId);
    if(rc != 1)
        return rc;
    /* END LENGTH of file identifier */
    
    /* FILE identifier */
    if(dir->name9660[0] == 0x00 || dir->name9660[0] == 0x01)
    /* self or parent */
    {
        rc = write711(image, &(dir->name9660[0]));
        if(rc != 1)
            return rc;
    }
    else
    {
        rc = write(image, dir->name9660, lenFileId);
        if(rc != lenFileId)
            return -1;
    }
    /* END FILE identifier */
    
    /* padding field */
    if(lenFileId % 2 == 0)
    {
        byte = 0;
        rc = write711(image, &byte);
        if(rc != 1)
            return rc;
    }
    
    if(filenameTypes | FNTYPE_ROCKRIDGE)
    {
        if(isFirstRecord)
        {
            rc = writeRockSP(image);
            if(rc < 0)
                return rc;
            
            rc = writeRockER(image);
            if(rc < 0)
                return rc;
        }
        
        if(!isSelfOrParent)
        {
            rc = writeRockNM(image, dir->nameRock);
            if(rc < 0)
                return rc;
        }
        
        rc = writeRockPX(image, dir, isADir);
        if(rc < 0)
            return rc;
    }
    
    /* RECORD length */
    endPos = lseek(image, 0, SEEK_CUR);
    
    lseek(image, startPos, SEEK_SET);
    
    byte = endPos - startPos;
    rc = write711(image, &byte);
    if(rc != 1)
        return rc;
    
    lseek(image, endPos, SEEK_SET);
    /* END RECORD length */
    
    return 1;
}

int writeRockER(int image)
{
    int rc;
    unsigned char record[46];
    
    /* identification */
    record[0] = 'E';
    record[1] = 'R';
    
    /* record length */
    record[2] = 46;
    
    /* entry version */
    record[3] = 1;
    
    /* extension identifier length */
    record[4] = 10;
    
    /* extension descriptor length */
    record[5] = 10;
    
    /* extension source length */
    record[6] = 18;
    
    /* extension version */
    record[7] = 1;
    
    /* extension identifier */
    strncpy(&(record[8]), "IEEE_P1282", 10);
    
    /* extension descriptor */
    strncpy(&(record[18]), "DRAFT_1_12", 10);
    
    /* extension source */
    strncpy(&(record[28]), "ADOPTED_1994_07_08", 18);
    
    rc = write(image, record, 46);
    if(rc != 46)
        return -1;
    
    return 1;
}

int writeRockNM(int image, char* name)
{
    int rc;
    unsigned char recordStart[5];
    int nameLen;
    
    nameLen = strlen(name);
    
    /* identification */
    recordStart[0] = 'N';
    recordStart[1] = 'M';
    
    /* record length */
    recordStart[2] = 5 + nameLen;
    
    /* entry version */
    recordStart[3] = 1;
    
    /* flags */
    recordStart[4] = 0;
    
    rc = write(image, recordStart, 5);
    if(rc != 5)
        return -1;
    
    rc = write(image, name, nameLen);
    if(rc != nameLen)
        return -1;
    
    return 1;
}

/* the slackware cd has 36 byte PX entries, missing the file serial number
* so i will do the same */
int writeRockPX(int image, DirToWrite* dir, bool isADir)
{
    int rc;
    unsigned char record[36];
    //DirToWriteLL* nextDir;
    unsigned posixFileLinks;
    
    /* identification */
    record[0] = 'P';
    record[1] = 'X';
    
    /* record length */
    record[2] = 36;
    
    /* entry version */
    record[3] = 1;
    
    /* posix file mode */
    write733ToByteArray(&(record[4]), dir->posixFileMode);
    
    /* POSIX file links */
    //!! this i think is number of subdirectories + 2 (self and parent)
    // and 1 for a file
    // it's probably not used on read-only filesystems
    // to add it, i will need to pass the number of links in a parent dir
    // recursively in writeDir()
    if(isADir)
        posixFileLinks = 2;
    else
        posixFileLinks = 1;
    
    write733ToByteArray(&(record[12]), posixFileLinks);
    /* END POSIX file links */
    
    /* posix file user id, posix file group id */
    bzero(&(record[20]), 16);
    
    rc = write(image, record, 36);
    if(rc != 36)
        return -1;
    
    return 1;
}

int writeRockSP(int image)
{
    int rc;
    unsigned char record[7];
    
    /* identification */
    record[0] = 'S';
    record[1] = 'P';
    
    /* record length */
    record[2] = 7;
    
    /* entry version */
    record[3] = 1;
    
    /* check bytes */
    record[4] = 0xBE;
    record[5] = 0xEF;
    
    /* bytes skipped */
    record[6] = 0;
    
    rc = write(image, record, 7);
    if(rc != 7)
        return -1;
    
    return 1;
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
