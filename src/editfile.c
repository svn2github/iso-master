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

#include "isomaster.h"

void editSelected(void)
{
    GtkTreeSelection* selection;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLisoTreeView));
    
    if(gtk_tree_selection_count_selected_rows(selection) != 1)
        return;
    
    /* there's just one row selected but this is the easiest way to do it */
    gtk_tree_selection_selected_foreach(selection, editSelectedCbk, NULL);
    
    /* can't put this in the callback because gtk complains */
    refreshIsoView();
}

void editSelectedCbk(GtkTreeModel* model, GtkTreePath* path,
                     GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* pathAndNameOnFs; /* to extract to and add from */
    char* pathAndNameOnIso; /* to delete from iso */
    int rc;
    GtkWidget* warningDialog;
    
    gtk_tree_model_get(model, iterator, COLUMN_HIDDEN_TYPE, &fileType, -1);
    
    if(fileType != FILE_TYPE_REGULAR)
    {
        printf("can only edit regular files (need dialog here)\n");
        return;
    }
    
    gtk_tree_model_get(model, iterator, COLUMN_FILENAME, &itemName, -1);
    
    /* create full path and name for the extracted file */
    pathAndNameOnFs = malloc(strlen(GBLappSettings.tempDir) + strlen(itemName) + 2);
    if(pathAndNameOnFs == NULL)
        fatalError("malloc(strlen(GBLappSettings.tempDir) + strlen(itemName) + 2) failed");
    strcpy(pathAndNameOnFs, GBLappSettings.tempDir);
    strcat(pathAndNameOnFs, "/"); /* doesn't hurt even if not needed */
    strcat(pathAndNameOnFs, itemName);
    
    printf("%s\n", pathAndNameOnFs);fflush(NULL);
    
    //!! don't want this if it's on fs already
    /* create full path and name for the file on the iso */
    //!! make it an isomaster-random string
    pathAndNameOnIso = malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1);
    if(pathAndNameOnIso == NULL)
        fatalError("malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1) failed");
    strcpy(pathAndNameOnIso, GBLisoCurrentDir);
    strcat(pathAndNameOnIso, itemName);
    
    printf("%s, %s\n", pathAndNameOnFs, pathAndNameOnIso);fflush(NULL);
    
    /* disable warnings, so user isn't confused with 'continue' buttons */
    bool(*savedWarningCbk)(const char*) = GBLvolInfo.warningCbk;
    GBLvolInfo.warningCbk = NULL;
    
    /* extract the file to the temporary directory */
    rc = bk_extract(&GBLvolInfo, pathAndNameOnIso, GBLappSettings.tempDir, 
                    true, activityProgressUpdaterCbk);
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
        free(pathAndNameOnFs);
        free(pathAndNameOnIso);
        GBLvolInfo.warningCbk = savedWarningCbk;
        return;
    }
    
    /* start the editor */
    if(!fork())
    {
        execlp(GBLappSettings.textEditor, "editor", pathAndNameOnFs, NULL);
        
        printf("execl(%s, %s) failed with %d\n", GBLappSettings.textEditor, pathAndNameOnFs, errno);
        
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
        free(pathAndNameOnFs);
        free(pathAndNameOnIso);
        GBLvolInfo.warningCbk = savedWarningCbk;
        return;
    }
    
    GBLisoChangesProbable = true;
    
    /* add the file back fom tmp */
    rc = bk_add(&GBLvolInfo, pathAndNameOnFs, GBLisoCurrentDir, activityProgressUpdaterCbk);
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
    
    // add to global list of files created (to delete after writing)
    
    g_free(itemName);
    free(pathAndNameOnFs);
    free(pathAndNameOnIso);
    GBLvolInfo.warningCbk = savedWarningCbk;
}

void editSelectedClickCbk(GtkMenuItem *menuitem, gpointer data)
{
    editSelected();
}
