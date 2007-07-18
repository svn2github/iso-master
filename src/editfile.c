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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libintl.h>
#include <errno.h>

#include "isomaster.h"

#define MAX_RANDOM_BASE_NAME_LEN 26
#define RANDOM_STR_NAME_LEN 6

/* files that I created in the temp dir for editing */
TempFileCreated* GBLtempFilesList = NULL;

extern bool GBLisoPaneActive;
extern GtkWidget* GBLisoTreeView;
extern AppSettings GBLappSettings;
extern char* GBLisoCurrentDir;
extern VolInfo GBLvolInfo;
extern GtkWidget* GBLmainWindow;
extern bool GBLisoChangesProbable;

/******************************************************************************
* addToTempFilesList()
* Appends a node to the front of the list.
* */
void addToTempFilesList(const char* pathAndName)
{
    TempFileCreated* newNode;
    
    newNode = malloc(sizeof(TempFileCreated));
    if(newNode == NULL)
        fatalError("newNode = malloc(sizeof(TempFileCreated)) failed\n");
    
    newNode->pathAndName = malloc(strlen(pathAndName) + 1);
    if(newNode == NULL)
        fatalError("newNode.pathAndName = malloc(strlen(pathAndName) + 1) failed\n");
    
    strcpy(newNode->pathAndName, pathAndName);
    
    newNode->next = GBLtempFilesList;
    
    GBLtempFilesList = newNode;
}

void destroyTempFilesList(void)
{
    TempFileCreated* next;
    
    while(GBLtempFilesList != NULL)
    {
        next = GBLtempFilesList->next;
        
        unlink(GBLtempFilesList->pathAndName);
        
        free(GBLtempFilesList->pathAndName);
        free(GBLtempFilesList);
        
        GBLtempFilesList = next;
    }
}

void editSelectedBtnCbk(GtkMenuItem *menuitem, gpointer data)
{
    GtkTreeSelection* selection;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLisoTreeView));
    
    if(gtk_tree_selection_count_selected_rows(selection) != 1)
        return;
    
    /* there's just one row selected but this is the easiest way to do it */
    gtk_tree_selection_selected_foreach(selection, editSelectedRowCbk, NULL);
    
    /* can't put this in the callback because gtk complains */
    refreshIsoView();
}

void editSelectedRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* randomizedItemName;
    char* pathAndNameOnFs; /* to extract to and add from */
    char* pathAndNameOnIso; /* to delete from iso */
    int rc;
    GtkWidget* warningDialog;
    
    gtk_tree_model_get(model, iterator, COLUMN_HIDDEN_TYPE, &fileType, -1);
    
    if(fileType != FILE_TYPE_REGULAR)
    {
        printf("can only edit regular files (!!need dialog here)\n");
        return;
    }
    
    gtk_tree_model_get(model, iterator, COLUMN_FILENAME, &itemName, -1);
    
    /* create full path and name for the file on the iso */
    pathAndNameOnIso = malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1);
    if(pathAndNameOnIso == NULL)
        fatalError("malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1) failed");
    strcpy(pathAndNameOnIso, GBLisoCurrentDir);
    strcat(pathAndNameOnIso, itemName);
    
    /* create full path and name for the extracted file */
    randomizedItemName = makeRandomFilename(itemName);
    pathAndNameOnFs = malloc(strlen(GBLappSettings.tempDir) + strlen(randomizedItemName) + 2);
    if(pathAndNameOnFs == NULL)
        fatalError("malloc(strlen(GBLappSettings.tempDir) + strlen(randomizedItemName) + 2) failed");
    strcpy(pathAndNameOnFs, GBLappSettings.tempDir);
    strcat(pathAndNameOnFs, "/"); /* doesn't hurt even if not needed */
    strcat(pathAndNameOnFs, randomizedItemName);
    
    /* disable warnings, so user isn't confused with 'continue' buttons */
    bool(*savedWarningCbk)(const char*) = GBLvolInfo.warningCbk;
    GBLvolInfo.warningCbk = NULL;
    
    /* extract the file to the temporary directory */
    rc = bk_extract_as(&GBLvolInfo, pathAndNameOnIso, GBLappSettings.tempDir, 
                       randomizedItemName, false, activityProgressUpdaterCbk);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               _("Failed to extract '%s': '%s'"),
                                               pathAndNameOnIso,
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        
        g_free(itemName);
        free(randomizedItemName);
        free(pathAndNameOnFs);
        free(pathAndNameOnIso);
        GBLvolInfo.warningCbk = savedWarningCbk;
        return;
    }
    
    addToTempFilesList(pathAndNameOnFs);
    
    /* start the editor */
    if(!fork())
    {
        execlp(GBLappSettings.editor, "editor", pathAndNameOnFs, NULL);
        
        printf("!!execl(%s, %s) failed with %d\n", GBLappSettings.editor, pathAndNameOnFs, errno);
        
        exit(1);
    }
    
    /* delete the file from the iso */
    rc = bk_delete(&GBLvolInfo, pathAndNameOnIso);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               _("Failed to delete '%s': '%s'"),
                                               pathAndNameOnIso,
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        
        g_free(itemName);
        free(randomizedItemName);
        free(pathAndNameOnFs);
        free(pathAndNameOnIso);
        GBLvolInfo.warningCbk = savedWarningCbk;
        return;
    }
    
    GBLisoChangesProbable = true;
    
    /* add the file back fom tmp */
    rc = bk_add_as(&GBLvolInfo, pathAndNameOnFs, GBLisoCurrentDir, itemName, 
                   activityProgressUpdaterCbk);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               _("Failed to add '%s': '%s'"),
                                               pathAndNameOnFs,
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
    }
    
    g_free(itemName);
    free(randomizedItemName);
    free(pathAndNameOnFs);
    free(pathAndNameOnIso);
    GBLvolInfo.warningCbk = savedWarningCbk;
}

/* caller must free the string returned */
char* makeRandomFilename(const char* sourceName)
{
    int sourceNameLen;
    char* newName;
    char randomStr[RANDOM_STR_NAME_LEN];
    int numRandomCharsFilled;
    
    if(strlen(sourceName) > MAX_RANDOM_BASE_NAME_LEN)
        sourceNameLen = MAX_RANDOM_BASE_NAME_LEN;
    else
        sourceNameLen = strlen(sourceName);
    
    newName = malloc(sourceNameLen + RANDOM_STR_NAME_LEN + 2);
    if(newName == NULL)
        fatalError("newName = malloc(sourceNameLen + RANDOM_STR_NAME_LEN + 2) failed\n");
    
    srandom((int)time(NULL));
    
    numRandomCharsFilled = 0;
    while(numRandomCharsFilled < RANDOM_STR_NAME_LEN)
    {
        char oneRandomChar;
        bool gotGoodChar;
        
        gotGoodChar = false;
        while(!gotGoodChar)
        {
            oneRandomChar = random();
            if(64 < oneRandomChar && oneRandomChar < 91)
            {
                gotGoodChar = true;
            }
        }
        
        randomStr[numRandomCharsFilled] = oneRandomChar;
        
        numRandomCharsFilled++;
    }
    
    strncpy(newName, randomStr, RANDOM_STR_NAME_LEN);
    newName[RANDOM_STR_NAME_LEN] = '\0';
    strcat(newName, "-");
    strncat(newName, sourceName, sourceNameLen);
    newName[RANDOM_STR_NAME_LEN + sourceNameLen + 1] = '\0';
    
    return newName;
}

void viewSelectedBtnCbk(GtkMenuItem *menuitem, gpointer data)
{
    GtkTreeSelection* selection;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLisoTreeView));
    
    if(gtk_tree_selection_count_selected_rows(selection) != 1)
        return;
    
    /* there's just one row selected but this is the easiest way to do it */
    gtk_tree_selection_selected_foreach(selection, viewSelectedRowCbk, NULL);
}

void viewSelectedRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* randomizedItemName;
    char* pathAndNameOnFs; /* to extract to and add from */
    char* pathAndNameOnIso; /* to delete from iso */
    int rc;
    GtkWidget* warningDialog;
    
    gtk_tree_model_get(model, iterator, COLUMN_HIDDEN_TYPE, &fileType, -1);
    
    if(fileType != FILE_TYPE_REGULAR)
    {
        printf("can only view regular files (!!need dialog here)\n");
        return;
    }
    
    gtk_tree_model_get(model, iterator, COLUMN_FILENAME, &itemName, -1);
    
    /* create full path and name for the file on the iso */
    pathAndNameOnIso = malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1);
    if(pathAndNameOnIso == NULL)
        fatalError("malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1) failed");
    strcpy(pathAndNameOnIso, GBLisoCurrentDir);
    strcat(pathAndNameOnIso, itemName);
    
    /* create full path and name for the extracted file */
    randomizedItemName = makeRandomFilename(itemName);
    pathAndNameOnFs = malloc(strlen(GBLappSettings.tempDir) + strlen(randomizedItemName) + 2);
    if(pathAndNameOnFs == NULL)
        fatalError("malloc(strlen(GBLappSettings.tempDir) + strlen(randomizedItemName) + 2) failed");
    strcpy(pathAndNameOnFs, GBLappSettings.tempDir);
    strcat(pathAndNameOnFs, "/"); /* doesn't hurt even if not needed */
    strcat(pathAndNameOnFs, randomizedItemName);
    
    /* disable warnings, so user isn't confused with 'continue' buttons */
    bool(*savedWarningCbk)(const char*) = GBLvolInfo.warningCbk;
    GBLvolInfo.warningCbk = NULL;
    
    /* extract the file to the temporary directory */
    rc = bk_extract_as(&GBLvolInfo, pathAndNameOnIso, GBLappSettings.tempDir, 
                       randomizedItemName, false, activityProgressUpdaterCbk);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               _("Failed to extract '%s': '%s'"),
                                               pathAndNameOnIso,
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        
        g_free(itemName);
        free(randomizedItemName);
        free(pathAndNameOnFs);
        free(pathAndNameOnIso);
        GBLvolInfo.warningCbk = savedWarningCbk;
        return;
    }
    
    addToTempFilesList(pathAndNameOnFs);
    
    /* start the viewer */
    if(!fork())
    {
        execlp(GBLappSettings.viewer, "viewer", pathAndNameOnFs, NULL);
        
        printf("!!execl(%s, %s) failed with %d\n", GBLappSettings.editor, pathAndNameOnFs, errno);
        
        exit(1);
    }
    
    g_free(itemName);
    free(randomizedItemName);
    free(pathAndNameOnFs);
    free(pathAndNameOnIso);
    GBLvolInfo.warningCbk = savedWarningCbk;
}
