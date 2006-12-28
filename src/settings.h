#ifndef window_h
#define window_h

#include <stdbool.h>
#include "iniparser-2.15/src/iniparser.h"

#define ISOMASTER_DEFAULT_WINDOW_WIDTH 500
#define ISOMASTER_DEFAULT_WINDOW_HEIGHT 550
#define ISOMASTER_DEFAULT_TOPPANE_HEIGHT 200

typedef struct
{
    /* stuff only read from the config file */
    int windowWidth;
    int windowHeight;
    int topPaneHeight;
    char* fsCurrentDir;
    
    /* stuff read from the config file that will also be written back from here */
    bool showHiddenFilesFs;
    bool sortDirectoriesFirst;
    int filenameTypesToWrite;
    char* lastIsoDir;
    char* lastBootRecordDir;
    
} AppSettings;

void buildImagePropertiesWindow(GtkWidget *widget, GdkEvent *event);
void findHomeDir(void);
void openConfigFile(char* configFileName);
void loadSettings(void);
void writeSettings(void);

#endif
