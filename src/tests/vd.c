#include <unistd.h>
#include "vd.h"

int readDR(int file, RootDR* dr)
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
        /*rc = readUnused(file, 1);
        if(rc != 1)
            return -1;*/
        lseek(file, 1, SEEK_CUR);
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

int readVD(int file, Vd* vd)
{
    int rc;
    unsigned char fsver; /* file structure version */
    
    rc = read(file, &(vd->volumeFlags), 1);
    if(rc != 1)
        return -1;
    
    rc = read(file, vd->sysId, 32);
    if(rc != 32)
        return -1;
    vd->sysId[32] = '\0';
    
    rc = read(file, vd->volId, 32);
    if(rc != 32)
        return -1;
    vd->volId[32] = '\0';
    
    /*rc = readUnused(file, 8);
    if(rc != 8)
        return -1;*/
    lseek(file, 8, SEEK_CUR);
    
    rc = read733(file, &(vd->volSpaceSize));
    if(rc != 4)
        return -1;

    rc = read(file, vd->escapeSequences, 32);
    if(rc != 32)
        return -1;

    
    rc = read723(file, &(vd->volSetSize));
    if(rc != 2)
        return -1;
    
    rc = read723(file, &(vd->volSeqNum));
    if(rc != 2)
        return -1;
    
    rc = read723(file, &(vd->lbSize));
    if(rc != 2)
        return -1;
    if(vd->lbSize != NBYTES_LOGICAL_SECTOR)
        return -2;
    
    rc = read733(file, &(vd->pathTableSize));
    if(rc != 4)
        return -1;
    
    rc = read731(file, &(vd->locTypeLPathTable));
    if(rc != 4)
        return -1;
    
    rc = read731(file, &(vd->locOptTypeLPathTable));
    if(rc != 4)
        return -1;
    
    rc = read732(file, &(vd->locTypeMPathTable));
    if(rc != 4)
        return -1;
    
    rc = read732(file, &(vd->locOptTypeMPathTable));
    if(rc != 4)
        return -1;
    
    rc = readDR(file, &(vd->rootDR));
    if(rc != 34)
        return -1;
    
    rc = read(file, vd->volSetId, 128);
    if(rc != 128)
        return -1;
    vd->volSetId[128] = '\0';
    
    rc = read(file, vd->publId, 128);
    if(rc != 128)
        return -1;
    vd->publId[128] = '\0';
    
    rc = read(file, vd->dataPrepId, 128);
    if(rc != 128)
        return -1;
    vd->dataPrepId[128] = '\0';
    
    rc = read(file, vd->appId, 128);
    if(rc != 128)
        return -1;
    vd->appId[128] = '\0';
    
    rc = read(file, vd->copyrightFid, 37);
    if(rc != 37)
        return -1;
    vd->copyrightFid[37] = '\0';
    
    rc = read(file, vd->abstractFid, 37);
    if(rc != 37)
        return -1;
    vd->abstractFid[37] = '\0';
    
    rc = read(file, vd->biblFid, 37);
    if(rc != 37)
        return -1;
    vd->biblFid[37] = '\0';
    
    rc = read(file, vd->volCreatTime.year, 4);
    if(rc != 4)
        return -1;
    vd->volCreatTime.year[4] = '\0';
    
    rc = read(file, vd->volCreatTime.month, 2);
    if(rc != 2)
        return -1;
    vd->volCreatTime.month[2] = '\0';
    
    rc = read(file, vd->volCreatTime.day, 2);
    if(rc != 2)
        return -1;
    vd->volCreatTime.day[2] = '\0';
    
    rc = read(file, vd->volCreatTime.hour, 2);
    if(rc != 2)
        return -1;
    vd->volCreatTime.hour[2] = '\0';
    
    rc = read(file, vd->volCreatTime.minute, 2);
    if(rc != 2)
        return -1;
    vd->volCreatTime.minute[2] = '\0';
    
    rc = read(file, vd->volCreatTime.second, 2);
    if(rc != 2)
        return -1;
    vd->volCreatTime.second[2] = '\0';
    
    rc = read(file, vd->volCreatTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    vd->volCreatTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(vd->volCreatTime.gmtOffset));
    if(rc != 1)
        return -1;
    
    rc = read(file, vd->volModTime.year, 4);
    if(rc != 4)
        return -1;
    vd->volModTime.year[4] = '\0';
    
    rc = read(file, vd->volModTime.month, 2);
    if(rc != 2)
        return -1;
    vd->volModTime.month[2] = '\0';
    
    rc = read(file, vd->volModTime.day, 2);
    if(rc != 2)
        return -1;
    vd->volModTime.day[2] = '\0';
    
    rc = read(file, vd->volModTime.hour, 2);
    if(rc != 2)
        return -1;
    vd->volModTime.hour[2] = '\0';
    
    rc = read(file, vd->volModTime.minute, 2);
    if(rc != 2)
        return -1;
    vd->volModTime.minute[2] = '\0';
    
    rc = read(file, vd->volModTime.second, 2);
    if(rc != 2)
        return -1;
    vd->volModTime.second[2] = '\0';
    
    rc = read(file, vd->volModTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    vd->volModTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(vd->volModTime.gmtOffset));
    if(rc != 1)
        return -1;
        
    rc = read(file, vd->volExpTime.year, 4);
    if(rc != 4)
        return -1;
    vd->volExpTime.year[4] = '\0';
    
    rc = read(file, vd->volExpTime.month, 2);
    if(rc != 2)
        return -1;
    vd->volExpTime.month[2] = '\0';
    
    rc = read(file, vd->volExpTime.day, 2);
    if(rc != 2)
        return -1;
    vd->volExpTime.day[2] = '\0';
    
    rc = read(file, vd->volExpTime.hour, 2);
    if(rc != 2)
        return -1;
    vd->volExpTime.hour[2] = '\0';
    
    rc = read(file, vd->volExpTime.minute, 2);
    if(rc != 2)
        return -1;
    vd->volExpTime.minute[2] = '\0';
    
    rc = read(file, vd->volExpTime.second, 2);
    if(rc != 2)
        return -1;
    vd->volExpTime.second[2] = '\0';
    
    rc = read(file, vd->volExpTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    vd->volExpTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(vd->volExpTime.gmtOffset));
    if(rc != 1)
        return -1;
        
    rc = read(file, vd->volEffTime.year, 4);
    if(rc != 4)
        return -1;
    vd->volEffTime.year[4] = '\0';
    
    rc = read(file, vd->volEffTime.month, 2);
    if(rc != 2)
        return -1;
    vd->volEffTime.month[2] = '\0';
    
    rc = read(file, vd->volEffTime.day, 2);
    if(rc != 2)
        return -1;
    vd->volEffTime.day[2] = '\0';
    
    rc = read(file, vd->volEffTime.hour, 2);
    if(rc != 2)
        return -1;
    vd->volEffTime.hour[2] = '\0';
    
    rc = read(file, vd->volEffTime.minute, 2);
    if(rc != 2)
        return -1;
    vd->volEffTime.minute[2] = '\0';
    
    rc = read(file, vd->volEffTime.second, 2);
    if(rc != 2)
        return -1;
    vd->volEffTime.second[2] = '\0';
    
    rc = read(file, vd->volEffTime.hundredthSecond, 2);
    if(rc != 2)
        return -1;
    vd->volEffTime.hundredthSecond[2] = '\0';
    
    rc = read712(file, &(vd->volEffTime.gmtOffset));
    if(rc != 1)
        return -1;
    
    rc = read711(file, &fsver);
    if(rc != 1)
        return -1;
    if(fsver != 1) /* should be 1 for a pvd/svd */
        return -1;
    
    /* the rest of the NBYTES_LOGICAL_SECTOR bytes, always this size for a pvd/svd */
    /*rc = readUnused(file, 1166); 
    if(rc != 1166)
        return -1;*/
    lseek(file, 1166, SEEK_CUR);
    
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
            /*rc = readUnused(file, NBYTES_LOGICAL_SECTOR - 7);
            if(rc != NBYTES_LOGICAL_SECTOR - 7)
                return -1;*/
            lseek(file, NBYTES_LOGICAL_SECTOR - 7, SEEK_CUR);
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
            /*rc = readUnused(file, NBYTES_LOGICAL_SECTOR - 7);
            if(rc != NBYTES_LOGICAL_SECTOR - 7)
                return -1;*/
            lseek(file, NBYTES_LOGICAL_SECTOR - 7, SEEK_CUR);
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
