/******************************* LICENCE **************************************
* Any code in this file may be redistributed or modified under the terms of
* the GNU General Public Licence as published by the Free Software 
* Foundation; version 2 of the licence.
****************************** END LICENCE ***********************************/

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <gtk/gtk.h>

#include "settings.h"
#include "error.h"

/* used by iniparser */
dictionary* GBLsettingsDictionary;
/* home directory with full path or "/" if don't know it */
char* GBLuserHomeDir;
/* iso master runtime and stored settings */
AppSettings GBLappSettings;

extern GtkWidget* GBLmainWindow;
extern GtkWidget* GBLbrowserPaned;
extern char* GBLfsCurrentDir;

void findHomeDir(void)
{
    char* userHomeDir;
    
    userHomeDir = getenv("HOME");
    if(userHomeDir == NULL)
    /* pretend user's home is root */
    {
        printWarning("failed to getenv(\"HOME\"), using \"/\" as home directory");
        GBLuserHomeDir = (char*)malloc(2);
        if(GBLuserHomeDir == NULL)
            fatalError("findHomeDir(): malloc(2) failed");
        GBLuserHomeDir[0] = '/';
        GBLuserHomeDir[1] = '\0';
    }
    else
    {
        int pathLen;
        
        /* need the directory ending with a / (bkisofs rule) */
        
        //!! need to make sure userHomeDir is a valid directory
        
        pathLen = strlen(userHomeDir);
        if(userHomeDir[pathLen] == '/')
        {
            GBLuserHomeDir = (char*)malloc(pathLen + 1);
            if(GBLuserHomeDir == NULL)
                fatalError("findHomeDir(): malloc(pathLen + 1) failed");
            strcpy(GBLuserHomeDir, userHomeDir);
        }
        else
        {
            GBLuserHomeDir = (char*)malloc(pathLen + 2);
            if(GBLuserHomeDir == NULL)
                fatalError("findHomeDir(): malloc(pathLen + 2) failed");
            strcpy(GBLuserHomeDir, userHomeDir);
            strcat(GBLuserHomeDir, "/");
        }
    }
}

void openConfigFile(char* configFileName)
{
    GBLsettingsDictionary = iniparser_load(configFileName);
    if(GBLsettingsDictionary == NULL)
    {
        printWarning("failed to open config file for reading, trying to create");
        
        int newConfigFile;
        
        newConfigFile = creat(configFileName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if(newConfigFile <= 0)
        {
            printWarning("failed to create '.isomaster'");
            return;
        }
        close(newConfigFile);
        
        GBLsettingsDictionary = iniparser_load(configFileName);
        if(GBLsettingsDictionary == NULL)
        {
            printWarning("iniparser failed to load the '.isomaster' just created, "
                         "this is a bug, but not critical (please do report it though).");
            return;
        }
    }
}

void loadSettings(void)
{
    char* configFileName;
    int width;
    int height;
    
    configFileName = malloc(strlen(GBLuserHomeDir) + strlen(".isomaster") + 1);
    if(configFileName == NULL)
        fatalError("loadSettings(): malloc(strlen(GBLuserHomeDir) + "
                   "strlen('.isomaster') + 1) failed");
    
    strcpy(configFileName, GBLuserHomeDir);
    strcat(configFileName, ".isomaster");
    
    if(strcmp(GBLuserHomeDir, "/") != 0)
        openConfigFile(configFileName);
    else
        printWarning("don't know user's home directory, so will not try to "
                     "load config file (~/.isomaster)");
    
    /* read/set window width */
    if(GBLsettingsDictionary != NULL)
    {
        width = iniparser_getint(GBLsettingsDictionary, "ui:windowwidth", INT_MAX);
        if(width == INT_MAX)
        /* not found in config file */
            GBLappSettings.windowWidth = ISOMASTER_DEFAULT_WINDOW_WIDTH;
        else
            GBLappSettings.windowWidth = width;
    }
    else
    /* no config file */
        GBLappSettings.windowWidth = ISOMASTER_DEFAULT_WINDOW_WIDTH;
    
    /* read/set window height */
    if(GBLsettingsDictionary != NULL)
    {
        height = iniparser_getint(GBLsettingsDictionary, "ui:windowheight", INT_MAX);
        if(height == INT_MAX)
        /* not found in config file */
            GBLappSettings.windowHeight = ISOMASTER_DEFAULT_WINDOW_HEIGHT;
        else
            GBLappSettings.windowHeight = height;
    }
    else
    /* no config file */
        GBLappSettings.windowHeight = ISOMASTER_DEFAULT_WINDOW_HEIGHT;
    
    /* read/set top pane height */
    if(GBLsettingsDictionary != NULL)
    {
        height = iniparser_getint(GBLsettingsDictionary, "ui:toppaneheight", INT_MAX);
        if(height == INT_MAX)
        /* not found in config file */
            GBLappSettings.topPaneHeight = ISOMASTER_DEFAULT_TOPPANE_HEIGHT;
        else
            GBLappSettings.topPaneHeight = height;
    }
    else
    /* no config file */
        GBLappSettings.topPaneHeight = ISOMASTER_DEFAULT_TOPPANE_HEIGHT;
    
    /* read/set fs browser directory */
    if(GBLsettingsDictionary != NULL)
    {
        //!! pointer sharing is ok but need to make sure ui:fscurrentdir is a 
        // valid directory and '/' terminated
        GBLappSettings.fsCurrentDir = iniparser_getstring(GBLsettingsDictionary, 
                                                          "ui:fscurrentdir", NULL);
    }
    else
    /* no config file */
        GBLappSettings.fsCurrentDir = NULL;
    
    //~ printf("read %dx%d, %d\n", GBLappSettings.windowWidth, 
                               //~ GBLappSettings.windowHeight, 
                               //~ GBLappSettings.topPaneHeight);
    
    free(configFileName);
}

void writeSettings(void)
{
    char* configFileName;
    FILE* fileToWrite;
    char numberStr[20];
    int width;
    int height;
    
    if(strcmp(GBLuserHomeDir, "/") == 0)
    {
        printWarning("don't know user's home directory, so will not try to save "
                     "config file (~/.isomaster)");
        return;
    }
    if(GBLsettingsDictionary == NULL)
    {
        printWarning("failed to create config file (~/.isomaster) when app started, "
                     "will not try again, settings not saved");
        return;
    }
    
    configFileName = malloc(strlen(GBLuserHomeDir) + strlen(".isomaster") + 1);
    if(configFileName == NULL)
        fatalError("writeSettings(): malloc(strlen(GBLuserHomeDir) + "
                   "strlen('.isomaster') + 1) failed");
    
    strcpy(configFileName, GBLuserHomeDir);
    strcat(configFileName, ".isomaster");
    
    fileToWrite = fopen(configFileName, "w");
    if(fileToWrite == NULL)
    {
        printWarning("could not open ~/.isomaster for writing, config not saved");
        free(configFileName);
        return;
    }
    
    free(configFileName);
    
    iniparser_setstr(GBLsettingsDictionary, "ui", NULL);
    
    gtk_window_get_size(GTK_WINDOW(GBLmainWindow), &width, &height);
    snprintf(numberStr, 20, "%d", width);
    iniparser_setstr(GBLsettingsDictionary, "ui:windowwidth", numberStr);
    snprintf(numberStr, 20, "%d", height);
    iniparser_setstr(GBLsettingsDictionary, "ui:windowheight", numberStr);
    
    height = gtk_paned_get_position(GTK_PANED(GBLbrowserPaned));
    snprintf(numberStr, 20, "%d", height);
    iniparser_setstr(GBLsettingsDictionary, "ui:toppaneheight", numberStr);
    
    iniparser_setstr(GBLsettingsDictionary, "ui:fscurrentdir", GBLfsCurrentDir);
    
    iniparser_dump_ini(GBLsettingsDictionary, fileToWrite);
}
