#define MAX_WARNING_LEN 1024

extern char GBLwarningStr[MAX_WARNING_LEN];

void clearWarningLog(void);
void logWarning(const char* str);
