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

extern GtkWidget* GBLisoTreeView;
extern GtkListStore* GBLisoListStore;
extern char* GBLisoCurrentDir;

extern GtkWidget* GBLfsTreeView;
extern GtkListStore* GBLfsListStore;
extern char* GBLfsCurrentDir;

/* iso file open()ed for reading */
static int GBLisoForReading;
/* directory tree of the iso that's being worked on */
static Dir GBLisoTree;

extern GdkPixbuf* GBLdirPixbuf;
extern GdkPixbuf* GBLfilePixbuf;

void addToIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* fullItemName; /* with full path */
    
    gtk_tree_model_get(model, iterator, COLUMN_HIDDEN_TYPE, &fileType, 
                                        COLUMN_FILENAME, &itemName, -1);
    
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        fullItemName = (char*)malloc(strlen(GBLfsCurrentDir) + strlen(itemName) + 2);
        if(fullItemName == NULL)
            fatalError("addToIsoEachRowCbk(): malloc("
                       "strlen(GBLfsCurrentDir) + strlen(itemName) + 2) failed");
        
        strcpy(fullItemName, GBLfsCurrentDir);
        strcat(fullItemName, itemName);
        strcat(fullItemName, "/");
        
        printf("want to add dir: '%s'\n", fullItemName);
        
        free(fullItemName);
    }
    else if(fileType == FILE_TYPE_REGULAR)
    {
        fullItemName = (char*)malloc(strlen(GBLfsCurrentDir) + strlen(itemName) + 1);
        if(fullItemName == NULL)
            fatalError("addToIsoEachRowCbk(): malloc("
                       "strlen(GBLfsCurrentDir) + strlen(itemName) + 2) failed");
        
        strcpy(fullItemName, GBLfsCurrentDir);
        strcat(fullItemName, itemName);
        
        printf("want to add file: '%s'\n", fullItemName);
        
        free(fullItemName);
    }
    else
        printWarning("gui error, adding anything other then files and directories doesn't work");
    
    g_free(itemName);
}

void addToIsoCbk(GtkButton *button, gpointer data)
{
    GtkTreeSelection* selection;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLfsTreeView));

    gtk_tree_selection_selected_foreach(selection, addToIsoEachRowCbk, NULL);
}

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
    g_signal_connect(GBLisoTreeView , "row-activated", (GCallback)isoRowDblClickCbk, NULL);
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

void changeIsoDirectory(char* newDirStr)
{
    int rc;
    Dir* newDir;
    DirLL* nextDir;
    FileLL* nextFile;
    GtkTreeIter listIterator;
    GtkTreeModel* model;
    
    rc = bk_get_dir_from_string(&GBLisoTree, newDirStr, &newDir);
    if(rc <= 0)
    {
        printLibWarning("failed to change directory on image", rc);
        return;
    }
    
    /* for improved performance disconnect the model from tree view before udating it */
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(GBLisoTreeView));
    g_object_ref(model);
    gtk_tree_view_set_model(GTK_TREE_VIEW(GBLisoTreeView), NULL);
    
    gtk_list_store_clear(GBLisoListStore);
    
    /* add all directories to the tree */
    nextDir = newDir->directories;
    while(nextDir != NULL)
    {
        gtk_list_store_append(GBLisoListStore, &listIterator);
        gtk_list_store_set(GBLisoListStore, &listIterator, 
                           COLUMN_ICON, GBLdirPixbuf,
                           COLUMN_FILENAME, nextDir->dir.name, 
                           COLUMN_SIZE, "dir",
                           COLUMN_HIDDEN_TYPE, FILE_TYPE_DIRECTORY,
                           -1);
        
        nextDir = nextDir->next;
    }
    
    /* add all files to the tree */
    nextFile = newDir->files;
    while(nextFile != NULL)
    {
        gtk_list_store_append(GBLisoListStore, &listIterator);
        gtk_list_store_set(GBLisoListStore, &listIterator, 
                           COLUMN_ICON, GBLfilePixbuf,
                           COLUMN_FILENAME, nextFile->file.name, 
                           COLUMN_SIZE, "file",
                           COLUMN_HIDDEN_TYPE, FILE_TYPE_REGULAR,
                           -1);
        
        nextFile = nextFile->next;
    }
    
    /* reconnect the model and view now */
    gtk_tree_view_set_model(GTK_TREE_VIEW(GBLisoTreeView), model);
    g_object_unref(model);
    
    /* set current directory string */
    if(GBLisoCurrentDir != NULL)
        free(GBLisoCurrentDir);
    GBLisoCurrentDir = (char*)malloc(strlen(newDirStr) + 1);
    if(GBLisoCurrentDir == NULL)
        fatalError("changeIsoDirectory(): malloc(strlen(newDirStr) + 1) failed");
    strcpy(GBLisoCurrentDir, newDirStr);
}

void isoGoUpDirTreeCbk(GtkButton *button, gpointer data)
{
    int count;
    bool done;
    char* newCurrentDir;
    
    /* do nothing if already at root */
    if(GBLisoCurrentDir[0] == '/' && GBLisoCurrentDir[1] == '\0')
        return;
    
    /* need to allocate a new string because changeIsoDirectory() uses it 
    * to copy from after freeing GBLisoCurrentDir */
    newCurrentDir = (char*)malloc(strlen(GBLisoCurrentDir) + 1);
    if(GBLisoCurrentDir == NULL)
        fatalError("isoGoUpDirTree(): malloc(strlen(GBLisoCurrentDir) + 1) failed");
    strcpy(newCurrentDir, GBLisoCurrentDir);
    
    /* look for the second last slash */
    done = false;
    for(count = strlen(newCurrentDir) - 1; !done; count--)
    {
        if(newCurrentDir[count - 1] == '/')
        /* truncate the string */
        {
            newCurrentDir[count] = '\0';
            changeIsoDirectory(newCurrentDir);
            done = true;
        }
    }
    
    free(newCurrentDir);
}

void isoRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                       GtkTreeViewColumn* col, gpointer data)
{
    GtkTreeModel* model;
    GtkTreeIter iterator;
    char* name;
    char* newCurrentDir;
    int fileType;
    
    model = gtk_tree_view_get_model(treeview);
    
    if(gtk_tree_model_get_iter(model, &iterator, path) == FALSE)
    {
        printWarning("isoRowDblClicked(): gtk_tree_model_get_iter() failed");
        return;
    }
    
    gtk_tree_model_get(model, &iterator, COLUMN_HIDDEN_TYPE, &fileType, -1);
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        gtk_tree_model_get(model, &iterator, COLUMN_FILENAME, &name, -1);
        
        newCurrentDir = (char*)malloc(strlen(GBLisoCurrentDir) + strlen(name) + 2);
        if(newCurrentDir == NULL)
            fatalError("isoRowDblClicked(): malloc("
                       "strlen(GBLisoCurrentDir) + strlen(name) + 2) failed");
        
        strcpy(newCurrentDir, GBLisoCurrentDir);
        strcat(newCurrentDir, name);
        strcat(newCurrentDir, "/");
        
        changeIsoDirectory(newCurrentDir);
        
        free(newCurrentDir);
        g_free(name);
    }
}

void openIso(char* filename)
{
    VolInfo volInfo;
    int rc;
    
    /* open image file for reading */
    GBLisoForReading = open(filename, O_RDONLY);
    if(GBLisoForReading == -1)
    {
        printWarning("failed to open iso file for reading");
        return;
    }
    
    rc = bk_read_vol_info(GBLisoForReading, &volInfo);
    if(rc <= 0)
    {
        printLibWarning("failed to read volume info", rc);
        return;
    }
    
    /* READ entire directory tree */
    GBLisoTree.directories = NULL;
    GBLisoTree.files = NULL;
    if(volInfo.filenameTypes & FNTYPE_ROCKRIDGE)
    {
        lseek(GBLisoForReading, volInfo.pRootDrOffset, SEEK_SET);
        rc = readDir(GBLisoForReading, &GBLisoTree, FNTYPE_ROCKRIDGE, true);
    }
    else if(volInfo.filenameTypes & FNTYPE_JOLIET)
    {
        lseek(GBLisoForReading, volInfo.sRootDrOffset, SEEK_SET);
        rc = readDir(GBLisoForReading, &GBLisoTree, FNTYPE_JOLIET, true);
    }
    else
    {
        lseek(GBLisoForReading, volInfo.pRootDrOffset, SEEK_SET);
        rc = readDir(GBLisoForReading, &GBLisoTree, FNTYPE_9660, true);
    }
    if(rc <= 0)
    {
        printLibWarning("failed to read directory tree", rc);
        return;
    }  
    /* END READ entire directory tree */
    
    changeIsoDirectory("/");
}

void openIsoCbk(GtkMenuItem* menuItem, gpointer data)
{
    //~ GtkWidget *dialog;
    //~ char* filename;
    
    //~ dialog = gtk_file_chooser_dialog_new("Open File",
                                         //~ NULL,
                                         //~ GTK_FILE_CHOOSER_ACTION_OPEN,
                                         //~ GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         //~ GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         //~ NULL);
    
    //~ if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    //~ {
        //~ filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        //~ openIso(filename);
        
        //~ g_free(filename);
    //~ }
    
    //~ gtk_widget_destroy(dialog);
    openIso("image.iso");
}
