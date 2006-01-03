#ifndef bkMangle_h
#define bkMangle_h

//!! fix all this shit, make it actually mangle things :)

int mangleDir(Dir* origDir, DirToWrite* newDir, int fileNameTypes);
void mangleDirName(char* src, char* dest);
void mangleFileName(char* src, char* dest, char* base, char* extension);
void replaceIllegalChars(char* string);

#endif
