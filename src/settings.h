#ifndef window_h
#define window_h

#include <stdbool.h>
#include "iniparser-2.15/src/iniparser.h"

#define ISOMASTER_DEFAULT_WINDOW_WIDTH 500
#define ISOMASTER_DEFAULT_WINDOW_HEIGHT 550
#define ISOMASTER_DEFAULT_TOPPANE_HEIGHT 200

/* not putting this in the makefile because i really can't think of a
* distro that doesn't have a writeable /tmp directory */
#define DEFAULT_TEMP_DIR "/tmp"

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
    bool scanForDuplicateFiles;
    bool followSymLinks;
    int filenameTypesToWrite;
    char* lastIsoDir;
    bool appendExtension;
    char* lastBootRecordDir;
    char* textEditor;
    char* tempDir;
    
} AppSettings;

void buildImagePropertiesWindow(GtkWidget *widget, GdkEvent *event);
void changeTextEditorCbk(GtkButton *button, gpointer data);
void changeTempDirCbk(GtkButton *button, gpointer data);
void findHomeDir(void);
void followSymLinksCbk(GtkButton *button, gpointer data);
void openConfigFile(char* configFileName);
void loadSettings(void);
void scanForDuplicatesCbk(GtkButton *button, gpointer data);
void writeSettings(void);

#endif
