#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "testread2.h"
#include "read7x.h"
#include "vd.h"

//~ const PxInfo posixFileDefaults = {0, 0, 0, 0, 0};
//~ const PxInfo posixDirDefaults = {0, 0, 0, 0, 0};
const unsigned posixFileDefaults = 33188; /* octal 100644 */
const unsigned posixDirDefaults = 16841; /* octal 40711 */

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
    lseek(image, NLS_SYSTEM_AREA * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    
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
    
    printf("publ: \'%s\'\n", vdset.pvd.publId);
    
    printf("dprp: \'%s\'\n", vdset.pvd.dataPrepId);
    
    printf("L path table: %d\n", vdset.pvd.locTypeLPathTable);
    
    printf("M path table: %d\n", vdset.pvd.locTypeMPathTable);
    
    printf("root extent at: %d\n", vdset.pvd.rootDR.locExtent);
    
    printf("data length: %d\n", vdset.pvd.rootDR.dataLength);
    
    //printf("joliet type: %d\n", svdGetJolietType(&(vdset.svd)));
    printf("joliet root extent at: %d\n", vdset.svd.rootDR.locExtent);
    
    
    lseek(image, vdset.pvd.rootDROffset, SEEK_SET);
    
    tree.directories = NULL;
    tree.files = NULL;
    
    rc = readDir(image, &tree, FNTYPE_ROCKRIDGE, true);
    
    printf("readDir ended with %d\n", rc);
    
    showDir(&tree, 0);
    
    close(image);
    
    return 0;
}

bool dirDrFollows(int image)
{
    unsigned char fileFlags;
    off_t origPos;
    int rc;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    lseek(image, 25, SEEK_CUR);
    
    rc = read711(image, &fileFlags);
    if(rc != 1)
        return rc;
    
    lseek(image, origPos, SEEK_SET);
    
    if((fileFlags >> 1 & 1) == 1)
        return true;
    else
        return false;
}

/* if the next byte is zero returns false otherwise true
* file position remains unchanged
* returns false on read error */
bool haveNextRecordInSector(int image)
{
    off_t origPos;
    char testByte;
    int rc;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, &testByte, 1);
    if(rc != 1)
        return false;
    
    lseek(image, origPos, SEEK_SET);
    
    return (testByte == 0) ? false : true;
}

void oops(char* msg)
{
    fprintf(stderr, "OOPS, %s\n", msg);
    exit(0);
}

/*
* if the root dr (inside vd) is read, it's filename will be ""
* note: directory identifiers do not end with ";1"
*/
int readDir(int image, Dir* dir, int filenameType, bool readPosix)
{
    int rc;
    unsigned char recordLength;
    unsigned locExtent; /* to know where to go before readDir9660() */
    unsigned lenExtent; /* parameter to readDir9660() */
    unsigned char lenFileId9660; /* also len joliet fileid (bytes) */
    int lenSU; /* calculated as recordLength - 33 - lenFileId9660 */
    off_t origPos;
    char rootTestByte;
    bool isRoot;
    
    rc = read(image, &recordLength, 1);
    if(rc != 1)
        return -1;
    
    lseek(image, 1, SEEK_CUR);
    
    rc = read733(image, &locExtent);
    if(rc != 8)
        return -1;
    
    rc = read733(image, &lenExtent);
    if(rc != 8)
        return -1;
    
    lseek(image, 14, SEEK_CUR);
    
    rc = read(image, &lenFileId9660, 1);
    if(rc != 1)
        return -1;
    
    lenSU = recordLength - 33 - lenFileId9660;
    if(lenFileId9660 % 2 == 0)
        lenSU -= 1;
    
    /* FIND out if root */
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, &rootTestByte, 1);
    if(rc != 1)
        return -1;
    
    lseek(image, origPos, SEEK_SET);
    
    if(lenFileId9660 == 1 && rootTestByte == 0x00)
    {
        isRoot = true;
        dir->name[0] = '\0';
    }
    else
        isRoot = false;
    /* END FIND out if root */
    
    if(filenameType == FNTYPE_9660)
    {
        if(!isRoot)
        {
            char nameAsOnDisk[256];
            
            rc = read(image, nameAsOnDisk, lenFileId9660);
            if(rc != lenFileId9660)
                return -1;
            
            strncpy(dir->name, nameAsOnDisk, lenFileId9660);
            
            dir->name[lenFileId9660] = '\0';
            
            if( strlen(dir->name) > NCHARS_FILE_ID_MAX - 1 )
                return -2;
        
            /* padding field */
            if(lenFileId9660 % 2 == 0)
                lseek(image, 1, SEEK_CUR);
        }
    }
    else if(filenameType == FNTYPE_JOLIET)
    {
        if(!isRoot)
        {
            char nameAsOnDisk[256];
            char nameInAscii[256];
            int ucsCount, byteCount;
            
            if(lenFileId9660 % 2 != 0)
                return -3;
            
            rc = read(image, nameAsOnDisk, lenFileId9660);
            if(rc != lenFileId9660)
                return -1;
            
            for(ucsCount = 1, byteCount = 0; ucsCount < lenFileId9660;
                ucsCount += 2, byteCount += 1)
            {
                nameInAscii[byteCount] = nameAsOnDisk[ucsCount];
            }
            nameInAscii[byteCount] = '\0';
            
            if( strlen(nameInAscii) > NCHARS_FILE_ID_MAX - 1 )
                return -2;
            
            strcpy(dir->name, nameInAscii);
            
            /* padding field */
            if(lenFileId9660 % 2 == 0)
                lseek(image, 1, SEEK_CUR);
        }
    }
    else if(filenameType == FNTYPE_ROCKRIDGE)
    {
        /* skip 9660 filename */
        lseek(image, lenFileId9660, SEEK_CUR);
        /* skip padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);
        
        if(!isRoot)
        {
            rc = readRockridgeFilename(image, dir->name, lenSU);
            if(rc < 0)
                return rc;
        }
    }
    else
        oops("readDir(): dude, what filename type did you ask for?");
    
    if(readPosix)
    {
        if(isRoot)
        {
            unsigned char realRootRecordLen;
            
            origPos = lseek(image, 0, SEEK_CUR);
            
            /* go to real root record */
            lseek(image, locExtent * NBYTES_LOGICAL_BLOCK, SEEK_SET);
            
            /* read record length */
            read(image, &realRootRecordLen, 1);
            if(rc != 1)
                return -1;
            
            /*go to sys use fields */
            lseek(image, 33, SEEK_CUR);
            
            readPosixInfo(image, &(dir->posixFileMode), realRootRecordLen - 34);
            
            /* return */
            lseek(image, origPos, SEEK_SET);
        }
        else
        {
            readPosixInfo(image, &(dir->posixFileMode), lenSU);
        }
    }
    else
    {
        /* this is good for root also */
        dir->posixFileMode = posixDirDefaults;
    }
    
    lseek(image, lenSU, SEEK_CUR);

    origPos = lseek(image, 0, SEEK_CUR);
    
    lseek(image, locExtent * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    
    rc = readDir9660(image, dir, lenExtent, filenameType, readPosix);
    if(rc < 0)
        return rc;
    
    lseek(image, origPos, SEEK_SET);
    
    return recordLength;
}

/*
* size is number of bytes
* hope you love pointers
*/
int readDir9660(int image, Dir* dir, unsigned size, int filenameType, bool readPosix)
{
    int rc;
    int bytesRead = 0;
    int childrenBytesRead;
    DirLL** nextDir; /* pointer to pointer to modify pointer :) */
    FileLL** nextFile;
    
    /* skip self and parent */
    bytesRead += skipDR(image);
    bytesRead += skipDR(image);
    
    nextDir = &(dir->directories);
    nextFile = &(dir->files);
    childrenBytesRead = 0;
    while(childrenBytesRead + bytesRead < size)
    {
        if(haveNextRecordInSector(image))
        /* read it */
        {
            if( dirDrFollows(image) )
            /* directory descriptor record */
            {
                int recordLength;
                
                *nextDir = malloc(sizeof(DirLL));
                if(*nextDir == NULL)
                    return -2;
                
                recordLength = readDir(image, &((*nextDir)->dir), filenameType, readPosix);
                if(recordLength < 0)
                    return recordLength;
                
                childrenBytesRead += recordLength;
                
                nextDir = &((*nextDir)->next);
                *nextDir = NULL;
            }
            else
            /* file descriptor record */
            {
                int recordLength;

                *nextFile = malloc(sizeof(FileLL));
                if(*nextFile == NULL)
                    return -2;
                
                recordLength = readFileInfo(image, &((*nextFile)->file), filenameType, readPosix);
                if(recordLength < 0)
                    return recordLength;
                
                childrenBytesRead += recordLength;
                
                nextFile = &((*nextFile)->next);
                *nextFile = NULL;
            }
        }
        else
        /* read zeroes until get to next record (that would be in the next
        *  sector btw) or get to the end of data (dir->self.dataLength) */
        {
            char testByte;
            off_t origPos;
            
            do
            {
                origPos = lseek(image, 0, SEEK_CUR);
                
                rc = read(image, &testByte, 1);
                if(rc != 1)
                    return -1;
                
                if(testByte != 0)
                {
                    lseek(image, origPos, SEEK_SET);
                    break;
                }
                
                childrenBytesRead += 1;
                
            } while (childrenBytesRead + bytesRead < size);
        }
    }
    
    return bytesRead;
}

int readFileInfo(int image, File* file, int filenameType, bool readPosix)
{
    int rc;
    unsigned char recordLength;
    unsigned locExtent; /* block num where the file is */
    unsigned lenExtent; /* in bytes */
    unsigned char lenFileId9660; /* also len joliet fileid (bytes) */
    int lenSU; /* calculated as recordLength - 33 - lenFileId9660 */
    
    rc = read(image, &recordLength, 1);
    if(rc != 1)
        return -1;
    
    lseek(image, 1, SEEK_CUR);
    
    rc = read733(image, &locExtent);
    if(rc != 8)
        return -1;
    
    rc = read733(image, &lenExtent);
    if(rc != 8)
        return -1;
    
    lseek(image, 14, SEEK_CUR);
    
    rc = read(image, &lenFileId9660, 1);
    if(rc != 1)
        return -1;
    
    lenSU = recordLength - 33 - lenFileId9660;
    if(lenFileId9660 % 2 == 0)
        lenSU -= 1;
    
    if(filenameType == FNTYPE_9660)
    {
        char nameAsOnDisk[256];
        
        rc = read(image, nameAsOnDisk, lenFileId9660);
        if(rc != lenFileId9660)
            return -1;
        
        removeCrapFromFilename(nameAsOnDisk, file->name, lenFileId9660);
        
        if( strlen(file->name) > NCHARS_FILE_ID_MAX - 1 )
            return -2;
    
        /* padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);
    }
    else if(filenameType == FNTYPE_JOLIET)
    {
        char nameAsOnDisk[256];
        char nameInAscii[256];
        int ucsCount, byteCount;
        
        if(lenFileId9660 % 2 != 0)
            return -3;
        
        rc = read(image, nameAsOnDisk, lenFileId9660);
        if(rc != lenFileId9660)
            return -1;
        
        for(ucsCount = 1, byteCount = 0; ucsCount < lenFileId9660;
            ucsCount += 2, byteCount += 1)
        {
            nameInAscii[byteCount] = nameAsOnDisk[ucsCount];
        }
        
        removeCrapFromFilename(nameInAscii, file->name, lenFileId9660 / 2);
        
        if( strlen(file->name) > NCHARS_FILE_ID_MAX - 1 )
            return -2;
    
        /* padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);
    }
    else if(filenameType == FNTYPE_ROCKRIDGE)
    {
        /* skip 9660 filename */
        lseek(image, lenFileId9660, SEEK_CUR);
        /* skip padding field */
        if(lenFileId9660 % 2 == 0)
            lseek(image, 1, SEEK_CUR);

        rc = readRockridgeFilename(image, file->name, lenSU);
        if(rc < 0)
            return rc;
    }
    else
        oops("readDir(): dude, what filename type did you ask for?");
    
    if(readPosix)
    {
        readPosixInfo(image, &(file->posixFileMode), lenSU);
    }
    else
    {
        file->posixFileMode = posixFileDefaults;
    }
    
    lseek(image, lenSU, SEEK_CUR);
    
    file->onImage = true;
    file->position = locExtent * NBYTES_LOGICAL_BLOCK;
    file->size = lenExtent;
    
    return recordLength;
}

unsigned char readNextRecordLen(int image)
{
    unsigned char rc;
    unsigned char length;
    off_t origPos;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read711(image, &length);
    if(rc != 1)
    // !! i don't like this int->char cast
        return rc;
    
    lseek(image, origPos, SEEK_SET);
    
    return length;
}

int readPosixInfo(int image, unsigned* posixFileMode, int lenSU)
{
    off_t origPos;
    unsigned char suFields[256];
    int rc;
    bool foundPosix;
    int count;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, suFields, lenSU);
    if(rc != lenSU)
        return -1;
    
    lseek(image, origPos, SEEK_SET);
    
    count = 0;
    foundPosix = false;
    while(count < lenSU && !foundPosix)
    {
        if(suFields[count] == 'P' && suFields[count + 1] == 'X')
        {
            read733FromCharArray(suFields + count + 4, posixFileMode);
            
            /* not interested in anything else from this field */
            
            foundPosix = true;
        }
        else
        /* skip su record */
        {
            count += suFields[count + 2];
        }
    }
    
    return 1;
}

/*
* leaves the file pointer where it was
*/
int readRockridgeFilename(int image, char* dest, int lenSU)
{
    off_t origPos;
    unsigned char suFields[256];
    char nameAsRead[256];
    int rc;
    int count;
    int lengthAsRead;
    bool foundName;
    
    origPos = lseek(image, 0, SEEK_CUR);
    
    rc = read(image, suFields, lenSU);
    if(rc != lenSU)
        return -1;
    
    lseek(image, origPos, SEEK_SET);
    
    count = 0;
    foundName = false;
    while(count < lenSU && !foundName)
    {
        if(suFields[count] == 'N' && suFields[count + 1] == 'M')
        {
            lengthAsRead = suFields[count + 2] - 5;
            
            strncpy(nameAsRead, suFields + count + 5, lengthAsRead);
            
            if(lengthAsRead > NCHARS_FILE_ID_MAX - 1)
                return -2;
            
            strncpy(dest, nameAsRead, lengthAsRead);
            
            foundName = true;
        }
        else
        /* skip su record */
        {
            count += suFields[count + 2];
        }
    }
    
    return 1;
}

/*
* filenames as read from 9660 Sometimes end with ;1 (terminator+version num)
* this removes the useless ending and terminates the destination with a '\0'
*/
void removeCrapFromFilename(char* src, char* dest, int length)
{
    int count;
    bool stop = false;
    
    for(count = 0; count < NCHARS_FILE_ID_MAX_READ && count < length && !stop; count++)
    {
        if(src[count] != ';')
            dest[count] = src[count];
        else
            stop = true;
    }
    
    dest[count] = '\0';
}

int skipDR(int image)
{
    unsigned char dRLen;
    int rc;
    
    rc = read711(image, &dRLen);
    if(rc != 1)
        return rc;
    
    lseek(image, dRLen - 1, SEEK_CUR);
    
    return dRLen;
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
        printf("%s - %d bytes @%08X\n", fileNode->file.name, fileNode->file.size, fileNode->file.position);
        fileNode = fileNode->next;
    }

}
