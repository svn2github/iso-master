#ifndef bkwrite_h
#define bkwrite_h

int writeByteBlock(int image, unsigned char byteToWrite, int numBytes);
int writeDir(int image, DirToWrite* dir, int parentLbNum, int parentNumBytes, 
             int parentPosix, time_t recordingTime, int filenameTypes,
             bool isRoot);
int writeDr(int image, DirToWrite* dir, time_t recordingTime, bool isADir, 
            bool isSelfOrParent, bool isFirstRecord, int filenameTypes);
int writeFileContents(int oldImage, int newImage, DirToWrite* dir, 
                      int filenameTypes);
int writeImage(int oldImage, int newImage, VolInfo* volInfo, Dir* oldTree,
               time_t creationTime, int filenameTypes);
int writeJolietStringField(int image, char* name, int fieldSize);
int writeVolDescriptor(int image, VolInfo* volInfo, unsigned rootDrLocation,
                       unsigned rootDrSize, time_t creationTime, bool isPrimary);
int writeRockER(int image);
int writeRockNM(int image, char* name);
int writeRockPX(int image, DirToWrite* dir, bool isADir);
int writeRockSP(int image);
int writeVdsetTerminator(int image);

#endif
