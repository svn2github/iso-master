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

void sizeCellDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                      GtkTreeModel *model, GtkTreeIter *iter,
                      gpointer data)
{
    unsigned sizeInt;
    int fileType;
    char buf[20];
    
    gtk_tree_model_get(model, iter, COLUMN_SIZE, &sizeInt, 
                                    COLUMN_HIDDEN_TYPE, &fileType, -1);
    
    if(fileType == FILE_TYPE_DIRECTORY)
        snprintf(buf, sizeof(buf), "dir");
    else if(sizeInt > 1073741824)
    /* print gibibytes */
        snprintf(buf, sizeof(buf), "%.1lf GB", (double)sizeInt / 1073741824);
    else if(sizeInt > 1048576)
    /* print mebibytes */
        snprintf(buf, sizeof(buf), "%.1lf MB", (double)sizeInt / 1048576);
    else if(sizeInt > 1024)
    /* print kibibytes */
        snprintf(buf, sizeof(buf), "%.1lf KB", (double)sizeInt / 1024);
    else
    /* print bytes */
        snprintf(buf, sizeof(buf), "%d B", sizeInt);
    
    g_object_set(renderer, "text", buf, NULL);
}
