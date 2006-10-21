/******************************* LICENCE **************************************
* Any code in this file may be redistributed or modified under the terms of
* the GNU General Public Licence as published by the Free Software 
* Foundation; version 2 of the licence.
****************************** END LICENCE ***********************************/

/******************************************************************************
* Author:
* Andrew Smith, http://littlesvr.ca/misc/contactandrew.php
*
* Contributors:
* 
******************************************************************************/

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <gtk/gtk.h>
#include <dirent.h>

#include "bk/bk.h"
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
extern GtkWidget* GBLshowHiddenMenuItem;
extern bool GBLisoPaneActive;
extern VolInfo GBLvolInfo;

void buildImagePropertiesWindow(GtkWidget *widget, GdkEvent *event)
{
    GtkWidget* dialog;
    GtkWidget* table;
    GtkWidget* label;
    GtkWidget* field;
    GtkWidget* publisherField;
    GtkWidget* volNameField;
    static const int fieldLen = 30; /* to display not length of contents */
    time_t timeT;
    char* timeStr;
    gint rc;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return;
    
    dialog = gtk_dialog_new_with_buttons("Image Information",
                                         GTK_WINDOW(GBLmainWindow),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         NULL);
    //gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    //g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
    gtk_widget_show(dialog);
    
    table = gtk_table_new(1, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, TRUE, TRUE, 0);
    gtk_widget_show(table);
    
    label = gtk_label_new("Creation time:");
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    gtk_widget_show(label);
    
    timeT = bk_get_creation_time(&GBLvolInfo);
    timeStr = ctime(&timeT);
    timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
    field = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(field), timeStr);
    //gtk_entry_set_editable(GTK_ENTRY(field), FALSE);
    gtk_widget_set_sensitive(field, FALSE);
    gtk_entry_set_width_chars(GTK_ENTRY(field), fieldLen);
    gtk_table_attach_defaults(GTK_TABLE(table), field, 1, 2, 0, 1);
    gtk_widget_show(field);
    
    label = gtk_label_new("Volume name:");
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
    gtk_widget_show(label);
    
    volNameField = gtk_entry_new_with_max_length(32);
    gtk_entry_set_text(GTK_ENTRY(volNameField), bk_get_volume_name(&GBLvolInfo));
    gtk_entry_set_width_chars(GTK_ENTRY(volNameField), fieldLen);
    gtk_table_attach_defaults(GTK_TABLE(table), volNameField, 1, 2, 1, 2);
    gtk_widget_show(volNameField);
    
    label = gtk_label_new("Publisher:");
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);
    gtk_widget_show(label);
    
    publisherField = gtk_entry_new_with_max_length(128);
    gtk_entry_set_text(GTK_ENTRY(publisherField), bk_get_publisher(&GBLvolInfo));
    gtk_entry_set_width_chars(GTK_ENTRY(publisherField), fieldLen);
    gtk_table_attach_defaults(GTK_TABLE(table), publisherField, 1, 2, 2, 3);
    gtk_widget_show(publisherField);
    
    rc = gtk_dialog_run(GTK_DIALOG(dialog));
    if(rc == GTK_RESPONSE_ACCEPT)
    {
        const char* publisher;
        const char* volName;
        
        publisher = gtk_entry_get_text(GTK_ENTRY(publisherField));
        bk_set_publisher(&GBLvolInfo, publisher);
        
        volName = gtk_entry_get_text(GTK_ENTRY(volNameField));
        bk_set_vol_name(&GBLvolInfo, volName);
    }
    
    gtk_widget_destroy(dialog);
}

void findHomeDir(void)
{
    char* userHomeDir;
    int pathLen;
    
    userHomeDir = getenv("HOME");
    if(userHomeDir == NULL)
    /* pretend user's home is root */
    {
        printWarning("failed to getenv(\"HOME\"), using \"/\" as "
                     "home directory");
        GBLuserHomeDir = (char*)malloc(2);
        if(GBLuserHomeDir == NULL)
            fatalError("findHomeDir(): malloc(2) failed");
        GBLuserHomeDir[0] = '/';
        GBLuserHomeDir[1] = '\0';
        return;
    }
    
    /* MAKE sure userHomeDir is a valid directory */
    DIR* openDirTest;
    
    openDirTest = opendir(userHomeDir);
    if(openDirTest == NULL)
    {
        printf("failed to open directory described by $HOME: '%s'\n", 
                                                            userHomeDir);
        GBLuserHomeDir = (char*)malloc(2);
        if(GBLuserHomeDir == NULL)
            fatalError("findHomeDir(): malloc(pathLen + 1) failed");
        strcpy(GBLuserHomeDir, "/");
        return;
    }
    
    closedir(openDirTest);
    /* END MAKE sure userHomeDir is a valid directory */
    
    /* need the directory ending with a / (bkisofs rule) */
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
    int showHidden;
    
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
        /* pointer sharing is ok since GBLappSettings.fsCurrentDir is only written from here */
        GBLappSettings.fsCurrentDir = iniparser_getstring(GBLsettingsDictionary, 
                                                          "ui:fscurrentdir", NULL);
    }
    else
    /* no config file */
        GBLappSettings.fsCurrentDir = NULL;
    
    /* read/set show hidden files on filesystem */
    if(GBLsettingsDictionary != NULL)
    {
        showHidden = iniparser_getboolean(GBLsettingsDictionary, 
                                          "ui:showhiddenfilesfs", INT_MAX);
        if(showHidden == INT_MAX)
            GBLappSettings.showHiddenFilesFs = false;
        else
            GBLappSettings.showHiddenFilesFs = showHidden;
    }
    else
    /* no config file */
        GBLappSettings.showHiddenFilesFs = false;
    
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
    
    snprintf(numberStr, 20, "%d", GBLappSettings.showHiddenFilesFs);
    iniparser_setstr(GBLsettingsDictionary, "ui:showhiddenfilesfs", numberStr);
    
    iniparser_dump_ini(GBLsettingsDictionary, fileToWrite);
}
