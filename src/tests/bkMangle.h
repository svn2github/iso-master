#ifndef bkMangle_h
#define bkMangle_h

void mangleDirName(char* src, char* dest, int fileNameType);
void mangleFileName(char* src, char* dest, int fileNameType);
void splitFileName(char* src, char* base, char* extension);

#endif
