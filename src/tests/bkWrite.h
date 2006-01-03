#ifndef bkwrite_h
#define bkwrite_h

int writeByteBlock(int image, unsigned char byteToWrite, int numBytes);
int writeDir(int image, DirToWrite* dir, int parentLbNum, int parentNumBytes, 
             int parentPosix, time_t recordingTime, int filenameTypes,
             bool isRoot);
int writeDr(int image, DirToWrite* dir, time_t recordingTime, bool isADir, 
            bool isSelfOrParent, bool isFirstRecord, int filenameTypes);
int writePriVolDescriptor(int image, VolInfo* volInfo, unsigned rootDrLocation,
                          unsigned rootDrSize, time_t creationTime);
int writeRockER(int image);
int writeRockNM(int image, char* name);
int writeRockPX(int image, DirToWrite* dir, bool isADir);
int writeRockSP(int image);
int writeVdsetTerminator(int image);

#endif
