#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include "bk/bk.h"
#include "bk/bkRead.h"
#include "browser.h"
#include "isobrowser.h"
#include "error.h"

/* the view used for the contents of the fs browser */
static GtkWidget* GBLisoTreeView;
/* the list store used for the contents of the fs browser */
static GtkListStore* GBLisoListStore;
/* slash-terminated, the dir being displayed in the browser */
static char* GBLisoCurrentDir = NULL;
/* iso file open()ed for reading */
static int GBLisoForReading;

/* menu-sized pixbufs of a directory and a file */
extern GdkPixbuf* GBLdirPixbuf;
extern GdkPixbuf* GBLfilePixbuf;

void buildIsoBrowser(GtkWidget* boxToPackInto)
{
    GtkWidget* scrolledWindow;
    GtkTreeSelection *selection;
    GtkCellRenderer* renderer;
    GtkTreeViewColumn* column;
    
    GBLisoListStore = gtk_list_store_new(NUM_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(boxToPackInto), scrolledWindow, TRUE, TRUE, 0);
    gtk_widget_show(scrolledWindow);
    
    /* view widget */
    GBLisoTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(GBLisoListStore));
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(GBLisoTreeView), COLUMN_FILENAME);
    g_object_unref(GBLisoListStore); /* destroy model automatically with view */
    gtk_container_add(GTK_CONTAINER(scrolledWindow), GBLisoTreeView );
    //g_signal_connect(GBLisoTreeView , "row-activated", (GCallback)fsRowDblClickCbk, NULL);
    gtk_widget_show(GBLisoTreeView);
    
    /* enable multi-line selection */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLisoTreeView));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    
    /* filename column */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, "Name");
    gtk_tree_view_column_set_resizable(column, TRUE);
    
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", COLUMN_ICON);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", COLUMN_FILENAME);
    
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_FILENAME);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(GBLisoTreeView), column);
    
    /* size column */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_set_title(column, "Size");
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", COLUMN_SIZE);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_SIZE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(GBLisoTreeView), column);
}

void openIso(char* filename)
{
    VolInfo volInfo;
    int rc;
    Dir tree;
    
    /* open image file for reading */
    GBLisoForReading = open(filename, O_RDONLY);
    if(GBLisoForReading == -1)
        printWarning("failed to open iso file for reading");
    
    rc = readVolInfo(GBLisoForReading, &volInfo);
    if(rc <= 0)
    {
        printLibWarning("failed to read volume info", rc);
        return;
    }
    
    /* READ entire directory tree */
    tree.directories = NULL;
    tree.files = NULL;
    if(volInfo.filenameTypes & FNTYPE_ROCKRIDGE)
    {
        lseek(GBLisoForReading, volInfo.pRootDrOffset, SEEK_SET);
        rc = readDir(GBLisoForReading, &tree, FNTYPE_ROCKRIDGE, true);
    }
    else if(volInfo.filenameTypes & FNTYPE_JOLIET)
    {
        lseek(GBLisoForReading, volInfo.sRootDrOffset, SEEK_SET);
        rc = readDir(GBLisoForReading, &tree, FNTYPE_JOLIET, true);
    }
    else
    {
        lseek(GBLisoForReading, volInfo.pRootDrOffset, SEEK_SET);
        rc = readDir(GBLisoForReading, &tree, FNTYPE_9660, true);
    }
    if(rc <= 0)
    {
        printLibWarning("failed to read directory tree", rc);
        return;
    }  
    /* END READ entire directory tree */
    
    // change iso directory
}

void openIsoCbk(GtkMenuItem* menuItem, gpointer data)
{
    GtkWidget *dialog;
    char* filename;
    
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        openIso(filename);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}
