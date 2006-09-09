#ifndef bkRead_h
#define bkRead_h

bool dirDrFollows(int image);
bool haveNextRecordInSector(int image);
int readDir(int image, VolInfo* volInfo, Dir* dir, int filenameType, 
            bool readPosix);
int readDirContents(int image, VolInfo* volInfo, Dir* dir, unsigned size, 
                    int filenameType, bool readPosix);
int readFileInfo(int image, VolInfo* volInfo, File* file, int filenameType, 
                 bool readPosix);
unsigned char readNextRecordLen(int image);
int readPosixInfo(int image, unsigned* posixFileMode, int lenSU);
int readRockridgeFilename(int image, char* dest, int lenSU);
void removeCrapFromFilename(char* filename, int length);
int skipDR(int image);
void stripSpacesFromEndOfString(char* str);

#endif
