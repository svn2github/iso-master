#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void fatalError(char* msg)
{
    fprintf(stderr, "ISO Master fatal error: %s\n", msg);
    exit(1);
}

void printWarning(char* msg)
{
    fprintf(stderr, "ISO Master warning: %s\n", msg);
}