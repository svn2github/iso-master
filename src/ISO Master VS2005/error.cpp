#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#include "error.h"

#define WARNING_LOG_FILE "c:\\isomaster-warning.log"

char GBLwarningStr[MAX_WARNING_LEN];

void clearWarningLog(void)
{
    FILE* logFile;

    logFile = fopen(WARNING_LOG_FILE, "w");
    
    if(logFile != NULL)
        fclose(logFile);
    
    logWarning("app started");
}

void logWarning(const char* str)
{
    FILE* logFile;

    logFile = fopen(WARNING_LOG_FILE, "a");
    
    if(logFile == NULL)
        return;
    
    fprintf(logFile, "%s\n", str);
    
    fclose(logFile);
}
