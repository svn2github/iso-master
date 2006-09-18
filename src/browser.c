/******************************* LICENCE **************************************
* Any code in this file may be redistributed or modified under the terms of
* the GNU General Public Licence as published by the Free Software 
* Foundation; version 2 of the licence.
****************************** END LICENCE ***********************************/

#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>

#include "browser.h"
#include "fsbrowser.h"
#include "isobrowser.h"
#include "bk/bk.h"
#include "error.h"

/* this file has thigs shared by the fs and the iso browser */

/* menu-sized pixbufs of a directory and a file */
GdkPixbuf* GBLdirPixbuf;
GdkPixbuf* GBLfilePixbuf;

/* the view used for the contents of the fs browser */
GtkWidget* GBLfsTreeView;
/* the list store used for the contents of the fs browser */
GtkListStore* GBLfsListStore;
/* slash-terminated, the dir being displayed in the fs browser */
char* GBLfsCurrentDir = NULL;

/* the view used for the contents of the fs browser */
GtkWidget* GBLisoTreeView;
/* the list store used for the contents of the fs browser */
GtkListStore* GBLisoListStore;
/* slash-terminated, the dir being displayed in the iso browser */
char* GBLisoCurrentDir = NULL;

extern GtkWidget* GBLmainWindow;
extern VolInfo GBLvolInfo;
extern bool GBLisoPaneActive;

void createDirCbk(GtkButton *button, gpointer onFs)
{
    GtkWidget* dialog;
    GtkWidget* warningDialog;
    int response;
    GtkWidget* textEntry;
    const char* newDirName;
    int rc;
    
    if(!onFs && !GBLisoPaneActive)
    /* asked to create dir on iso but no iso is open */
        return;
    
    dialog = gtk_dialog_new_with_buttons("Enter name for new directory",
                                         GTK_WINDOW(GBLmainWindow),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         NULL);
    
    textEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(textEntry), 40);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), textEntry);
    gtk_widget_show(textEntry);
    
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if(response == GTK_RESPONSE_ACCEPT)
    {
        newDirName = gtk_entry_get_text(GTK_ENTRY(textEntry));
        
        if(onFs)
        {
            char* pathAndName;
            
            pathAndName = malloc(strlen(GBLfsCurrentDir) + strlen(newDirName) + 1);
            if(pathAndName == NULL)
                fatalError("createDirCbk(): malloc(strlen(GBLfsCurrentDir) + "
                           "strlen(newDirName) + 1) failed");
            
            strcpy(pathAndName, GBLfsCurrentDir);
            strcat(pathAndName, newDirName);
            
            rc = mkdir(pathAndName, 0755);
            if(rc == -1)
            {
                warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_ERROR,
                                                       GTK_BUTTONS_CLOSE,
                                                       "Failed to create directory %s",
                                                       pathAndName);
                gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
                gtk_dialog_run(GTK_DIALOG(warningDialog));
                gtk_widget_destroy(warningDialog);
                gtk_widget_destroy(dialog);
                return;
            }
            
            free(pathAndName);
            
            refreshFsView();
        }
        else
        /* on iso */
        {
            rc = bk_create_dir(&GBLvolInfo, GBLisoCurrentDir, newDirName);
            if(rc <= 0)
            {
                warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_ERROR,
                                                       GTK_BUTTONS_CLOSE,
                                                       "Failed to create directory %s: '%s'",
                                                       newDirName,
                                                       bk_get_error_string(rc));
                gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
                gtk_dialog_run(GTK_DIALOG(warningDialog));
                gtk_widget_destroy(warningDialog);
                gtk_widget_destroy(dialog);
                return;
            }
            
            refreshIsoView();
        }
        fflush(NULL);
    }
    
    gtk_widget_destroy(dialog);
}

void formatSize(unsigned sizeInt, char* sizeStr, int sizeStrLen)
{
    if(sizeInt > 1073741824)
    /* print gibibytes */
        snprintf(sizeStr, sizeStrLen, "%.1lf GB", (double)sizeInt / 1073741824);
    else if(sizeInt > 1048576)
    /* print mebibytes */
        snprintf(sizeStr, sizeStrLen, "%.1lf MB", (double)sizeInt / 1048576);
    else if(sizeInt > 1024)
    /* print kibibytes */
        snprintf(sizeStr, sizeStrLen, "%.1lf KB", (double)sizeInt / 1024);
    else
    /* print bytes */
        snprintf(sizeStr, sizeStrLen, "%d B", sizeInt);
    
    sizeStr[sizeStrLen - 1] = '\0';
}

/* formats the file size text for displaying */
void sizeCellDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                      GtkTreeModel *model, GtkTreeIter *iter,
                      gpointer data)
{
    unsigned sizeInt;
    int fileType;
    char sizeStr[20];
    
    gtk_tree_model_get(model, iter, COLUMN_SIZE, &sizeInt, 
                                    COLUMN_HIDDEN_TYPE, &fileType, -1);
    
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        snprintf(sizeStr, sizeof(sizeStr), "dir");
        sizeStr[sizeof(sizeStr) - 1] = '\0';
    }
    else
        formatSize(sizeInt, sizeStr, sizeof(sizeStr));
    
    g_object_set(renderer, "text", sizeStr, NULL);
}
