#ifndef bkwrite_h
#define bkwrite_h

int writeByteBlock(int image, unsigned char byteToWrite, unsigned numBytes);
int writePriVolDescriptor(int image, VolInfo* volInfo, unsigned rootDrLocation,
                          unsigned rootDrSize, time_t creationTime);
int writeVdsetTerminator(int image);

#endif
