#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void fatalError(char* msg)
{
    fprintf(stderr, "ISO Master fatal error: %s\n", msg);
    exit(1);
}

void printLibWarning(char* msg, int errNum)
{
    char* str;
    
    str = (char*)malloc(strlen(msg) + 100);
    if(str == NULL)
        fatalError("printLibWarning(): malloc(strlen(msg) + 21) failed");
    
    sprintf(str, "%s, bkisofs failed with error %d", msg, errNum);
    
    printWarning(str);
    
    free(str);
}

void printWarning(char* msg)
{
    fprintf(stderr, "ISO Master warning: %s\n", msg);
}
