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
#include <libintl.h>
#include <time.h>

#include "isomaster.h"

/* used by iniparser */
dictionary* GBLsettingsDictionary;
/* home directory with full path or "/" if don't know it */
char* GBLuserHomeDir;
/* iso master runtime and stored settings */
AppSettings GBLappSettings;

extern GtkWidget* GBLmainWindow;
extern GtkWidget* GBLbrowserPaned;
extern char* GBLfsCurrentDir;
extern bool GBLisoPaneActive;
extern VolInfo GBLvolInfo;
extern bool GBLisoChangesProbable;

void buildImagePropertiesWindow(GtkWidget *widget, GdkEvent *event)
{
    GtkWidget* dialog;
    GtkWidget* table;
    GtkWidget* label;
    GtkWidget* field;
    GtkWidget* publisherField;
    GtkWidget* volNameField;
    GtkWidget* hBox;
    GtkWidget* rockridgeCheck;
    GtkWidget* jolietCheck;
    static const int fieldLen = 30; /* to display not length of contents */
    time_t timeT;
    char* timeStr;
    gint rc;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return;
    
    dialog = gtk_dialog_new_with_buttons(_("Image Information"),
                                         GTK_WINDOW(GBLmainWindow),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         NULL);
    g_signal_connect(dialog, "close", G_CALLBACK(rejectDialogCbk), NULL);
    
    table = gtk_table_new(1, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, TRUE, TRUE, 0);
    gtk_widget_show(table);
    
    label = gtk_label_new(_("Creation time:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
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
    
    label = gtk_label_new(_("Volume name:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
    gtk_widget_show(label);
    
    volNameField = gtk_entry_new_with_max_length(32);
    gtk_entry_set_text(GTK_ENTRY(volNameField), bk_get_volume_name(&GBLvolInfo));
    gtk_entry_set_width_chars(GTK_ENTRY(volNameField), fieldLen);
    g_signal_connect(volNameField, "activate", (GCallback)acceptDialogCbk, dialog);
    gtk_table_attach_defaults(GTK_TABLE(table), volNameField, 1, 2, 1, 2);
    gtk_widget_show(volNameField);
    
    label = gtk_label_new(_("Publisher:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);
    gtk_widget_show(label);
    
    publisherField = gtk_entry_new_with_max_length(128);
    gtk_entry_set_text(GTK_ENTRY(publisherField), bk_get_publisher(&GBLvolInfo));
    gtk_entry_set_width_chars(GTK_ENTRY(publisherField), fieldLen);
    g_signal_connect(publisherField, "activate", (GCallback)acceptDialogCbk, dialog);
    gtk_table_attach_defaults(GTK_TABLE(table), publisherField, 1, 2, 2, 3);
    gtk_widget_show(publisherField);
    
    hBox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hBox, TRUE, TRUE, 5);
    gtk_widget_show(hBox);
    
    label = gtk_label_new(_("Filename types (both recommended):"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    gtk_box_pack_start(GTK_BOX(hBox), label, TRUE, TRUE, 0);
    gtk_widget_show(label);
    
    rockridgeCheck = gtk_check_button_new_with_label("RockRidge");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rockridgeCheck), 
                                 GBLappSettings.filenameTypesToWrite & FNTYPE_ROCKRIDGE);
    gtk_box_pack_start(GTK_BOX(hBox), rockridgeCheck, TRUE, TRUE, 0);
    gtk_widget_show(rockridgeCheck);
    
    jolietCheck = gtk_check_button_new_with_label("Joliet");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(jolietCheck), 
                                 GBLappSettings.filenameTypesToWrite & FNTYPE_JOLIET);
    gtk_box_pack_start(GTK_BOX(hBox), jolietCheck, TRUE, TRUE, 0);
    gtk_widget_show(jolietCheck);
    
    gtk_widget_show(dialog);
    
    rc = gtk_dialog_run(GTK_DIALOG(dialog));
    if(rc == GTK_RESPONSE_ACCEPT)
    {
        const char* publisher;
        const char* volName;
        
        publisher = gtk_entry_get_text(GTK_ENTRY(publisherField));
        bk_set_publisher(&GBLvolInfo, publisher);
        
        volName = gtk_entry_get_text(GTK_ENTRY(volNameField));
        bk_set_vol_name(&GBLvolInfo, volName);
        
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rockridgeCheck)))
            GBLappSettings.filenameTypesToWrite |= FNTYPE_ROCKRIDGE;
        else
            GBLappSettings.filenameTypesToWrite &= ~FNTYPE_ROCKRIDGE;
        
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(jolietCheck)))
            GBLappSettings.filenameTypesToWrite |= FNTYPE_JOLIET;
        else
            GBLappSettings.filenameTypesToWrite &= ~FNTYPE_JOLIET;
        
        GBLisoChangesProbable = true;
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

void followSymLinksCbk(GtkButton *button, gpointer data)
{
    GBLappSettings.followSymLinks = !GBLappSettings.followSymLinks;
    
    bk_set_follow_symlinks(&GBLvolInfo, GBLappSettings.followSymLinks);
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
    int sortDirsFirst;
    int scanForDuplicateFiles;
    int followSymLinks;
    char* tempStr;
    int appendExtension;
    
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
    
    /* read/set sort directories first */
    if(GBLsettingsDictionary != NULL)
    {
        sortDirsFirst = iniparser_getboolean(GBLsettingsDictionary, 
                                             "ui:sortdirsfirst", INT_MAX);
        if(sortDirsFirst == INT_MAX)
            GBLappSettings.sortDirectoriesFirst = true;
        else
            GBLappSettings.sortDirectoriesFirst = sortDirsFirst;
    }
    else
    /* no config file */
        GBLappSettings.sortDirectoriesFirst = true;
    
    /* read/set scan for duplicate files */
    if(GBLsettingsDictionary != NULL)
    {
        scanForDuplicateFiles = iniparser_getboolean(GBLsettingsDictionary, 
                                                     "ui:scanforduplicatefiles", INT_MAX);
        if(scanForDuplicateFiles == INT_MAX)
            GBLappSettings.scanForDuplicateFiles = true;
        else
            GBLappSettings.scanForDuplicateFiles = scanForDuplicateFiles;
    }
    else
    /* no config file */
        GBLappSettings.scanForDuplicateFiles = true;
    
    /* read/set follow symbolic links */
    if(GBLsettingsDictionary != NULL)
    {
        followSymLinks = iniparser_getboolean(GBLsettingsDictionary, 
                                              "ui:followsymlinks", INT_MAX);
        if(followSymLinks == INT_MAX)
            GBLappSettings.followSymLinks = false;
        else
            GBLappSettings.followSymLinks = followSymLinks;
    }
    else
    /* no config file */
        GBLappSettings.followSymLinks = false;
    
    /* read/set last iso open/save dir */
    if(GBLsettingsDictionary != NULL)
    {
        tempStr = iniparser_getstring(GBLsettingsDictionary, 
                                      "ui:lastisodir", NULL);
        if(tempStr == NULL)
            GBLappSettings.lastIsoDir = NULL;
        else
        {
            GBLappSettings.lastIsoDir = malloc(strlen(tempStr) +1);
            if(GBLappSettings.lastIsoDir == NULL)
                fatalError("GBLappSettings.lastIsoDir = malloc(strlen(tempStr) +1) failed");
            strcpy(GBLappSettings.lastIsoDir, tempStr);
        }
    }
    else
    /* no config file */
        GBLappSettings.lastIsoDir = NULL;
    
    /* read/set last boot record dir */
    if(GBLsettingsDictionary != NULL)
    {
        tempStr = iniparser_getstring(GBLsettingsDictionary, 
                                      "ui:lastbootrecorddir", NULL);
        if(tempStr == NULL)
            GBLappSettings.lastBootRecordDir = NULL;
        else
        {
            GBLappSettings.lastBootRecordDir = malloc(strlen(tempStr) +1);
            if(GBLappSettings.lastBootRecordDir == NULL)
                fatalError("GBLappSettings.lastBootRecordDir = malloc(strlen(tempStr) +1) failed");
            strcpy(GBLappSettings.lastBootRecordDir, tempStr);
        }
    }
    else
    /* no config file */
        GBLappSettings.lastBootRecordDir = NULL;
    
    /* read/set show hidden files on filesystem */
    if(GBLsettingsDictionary != NULL)
    {
        appendExtension = iniparser_getboolean(GBLsettingsDictionary, 
                                              "ui:appendextension", INT_MAX);
        if(showHidden == INT_MAX)
            GBLappSettings.appendExtension = true;
        else
            GBLappSettings.appendExtension = appendExtension;
    }
    else
    /* no config file */
        GBLappSettings.appendExtension = true;
    
    /* read/set text editor */
    if(GBLsettingsDictionary != NULL)
    {
        tempStr = iniparser_getstring(GBLsettingsDictionary, 
                                      "ui:texteditor", NULL);
        if(tempStr == NULL)
        {
            //!! put this in the makefile
            #define DEFAULT_TEXT_EDITOR "scite"
            GBLappSettings.textEditor = malloc(strlen(DEFAULT_TEXT_EDITOR) + 1);
            if(GBLappSettings.textEditor == NULL)
                fatalError("GBLappSettings.textEditor = malloc(strlen(DEFAULT_TEXT_EDITOR) +1) failed");
            strcpy(GBLappSettings.textEditor, DEFAULT_TEXT_EDITOR);
        }
        else
        {
            GBLappSettings.textEditor = malloc(strlen(tempStr) +1);
            if(GBLappSettings.textEditor == NULL)
                fatalError("GBLappSettings.lastBootRecordDir = malloc(strlen(tempStr) +1) failed");
            strcpy(GBLappSettings.textEditor, tempStr);
        }
    }
    else
    /* no config file */
        GBLappSettings.textEditor = NULL;
    
    /* read/set temporary directory */
    if(GBLsettingsDictionary != NULL)
    {
        tempStr = iniparser_getstring(GBLsettingsDictionary, 
                                      "ui:tempdir", NULL);
        if(tempStr == NULL)
        {
            //!! put this in the makefile
            #define DEFAULT_TEMP_DIR "/tmp"
            GBLappSettings.tempDir = malloc(strlen(DEFAULT_TEMP_DIR) + 1);
            if(GBLappSettings.tempDir == NULL)
                fatalError("GBLappSettings.tempDir = malloc(strlen(DEFAULT_TEMP_DIR) +1) failed");
            strcpy(GBLappSettings.tempDir, DEFAULT_TEMP_DIR);
        }
        else
        {
            GBLappSettings.tempDir = malloc(strlen(tempStr) +1);
            if(GBLappSettings.tempDir == NULL)
                fatalError("GBLappSettings.lastBootRecordDir = malloc(strlen(tempStr) +1) failed");
            strcpy(GBLappSettings.tempDir, tempStr);
        }
    }
    else
    /* no config file */
        GBLappSettings.tempDir = NULL;
    
    free(configFileName);
}

void scanForDuplicatesCbk(GtkButton *button, gpointer data)
{
    GBLappSettings.scanForDuplicateFiles = !GBLappSettings.scanForDuplicateFiles;
    
    if(GBLisoPaneActive)
    {
        GtkWidget* warningDialog;
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_CLOSE,
                                               _("You will have to close and reopen the ISO for"
                                               " this change to take effect"));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
    }
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
    
    snprintf(numberStr, 20, "%d", GBLappSettings.sortDirectoriesFirst);
    iniparser_setstr(GBLsettingsDictionary, "ui:sortdirsfirst", numberStr);
    
    snprintf(numberStr, 20, "%d", GBLappSettings.scanForDuplicateFiles);
    iniparser_setstr(GBLsettingsDictionary, "ui:scanforduplicatefiles", numberStr);
    
    snprintf(numberStr, 20, "%d", GBLappSettings.followSymLinks);
    iniparser_setstr(GBLsettingsDictionary, "ui:followsymlinks", numberStr);
    
    if(GBLappSettings.lastIsoDir != NULL)
        iniparser_setstr(GBLsettingsDictionary, "ui:lastisodir", GBLappSettings.lastIsoDir);
    
    if(GBLappSettings.lastBootRecordDir != NULL)
        iniparser_setstr(GBLsettingsDictionary, "ui:lastbootrecorddir", GBLappSettings.lastBootRecordDir);
    
    snprintf(numberStr, 20, "%d", GBLappSettings.appendExtension);
    iniparser_setstr(GBLsettingsDictionary, "ui:appendextension", numberStr);
    
    iniparser_dump_ini(GBLsettingsDictionary, fileToWrite);
}
