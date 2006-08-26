/******************************* LICENCE **************************************
* Any code in this file may be redistributed or modified under the terms of
* the GNU General Public Licence as published by the Free Software 
* Foundation; version 2 of the licence.
****************************** END LICENCE ***********************************/

#include <gtk/gtk.h>

#include "browser.h"

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
