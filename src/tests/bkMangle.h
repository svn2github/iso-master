#ifndef bkMangle_h
#define bkMangle_h

// insert ~xxx in file name 
int mangleDir(Dir* origDir, Dir* newDir, int fileNameType);
void mangleDirName(char* src, char* dest, int fileNameType);
void mangleFileName(char* src, char* dest, int fileNameType, char* base, char* extension);
void replaceIllegalChars(char* string, int fileNameType);

#endif
