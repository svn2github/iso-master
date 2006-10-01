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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

#include "browser.h"
#include "fsbrowser.h"
#include "error.h"
#include "settings.h"

extern AppSettings GBLappSettings;
extern char* GBLuserHomeDir;

extern GtkWidget* GBLmainWindow;

extern GtkWidget* GBLfsCurrentDirField;
extern GtkWidget* GBLfsTreeView;
extern GtkListStore* GBLfsListStore;
extern char* GBLfsCurrentDir;

extern GdkPixbuf* GBLdirPixbuf;
extern GdkPixbuf* GBLfilePixbuf;

extern int errno;

void buildFsBrowser(GtkWidget* boxToPackInto)
{
    GtkWidget* scrolledWindow;
    GtkTreeSelection *selection;
    GtkCellRenderer* renderer;
    GtkTreeViewColumn* column;
    
    GtkIconSet* iconSet;
    GtkIconSize* iconSizes = NULL;
    int numIconSizes;
    GtkIconSize iconSize;
    
    GBLfsListStore = gtk_list_store_new(NUM_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT);
    
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(boxToPackInto), scrolledWindow, TRUE, TRUE, 0);
    gtk_widget_show(scrolledWindow);
    
    /* view widget */
    GBLfsTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(GBLfsListStore));
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(GBLfsTreeView), COLUMN_FILENAME);
    g_object_unref(GBLfsListStore); /* destroy model automatically with view */
    gtk_container_add(GTK_CONTAINER(scrolledWindow), GBLfsTreeView);
    g_signal_connect(GBLfsTreeView, "row-activated", (GCallback)fsRowDblClickCbk, NULL);
    gtk_widget_show(GBLfsTreeView);
    
    /* this won't be enabled until gtk allows me to drag a multiple selection */
    GtkTargetEntry targetEntry;
    targetEntry.target = "text/plain";
    targetEntry.flags = 0;
    targetEntry.info = 0;
    gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(GBLfsTreeView), GDK_BUTTON1_MASK, 
                                           &targetEntry, 1, GDK_ACTION_COPY);
    
    /* enable multi-line selection */
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLfsTreeView));
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
    gtk_tree_view_append_column(GTK_TREE_VIEW(GBLfsTreeView), column);
    
    /* size column */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_set_title(column, "Size");
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", COLUMN_SIZE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, sizeCellDataFunc, NULL, NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_SIZE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(GBLfsTreeView), column);
        
    /* set default sort */
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(GBLfsListStore),
                                         COLUMN_FILENAME, GTK_SORT_ASCENDING);

    /* CREATE pixbuf for directory */
    iconSet = gtk_icon_factory_lookup_default(GTK_STOCK_DIRECTORY);
    if(iconSet == NULL)
        GBLdirPixbuf = NULL;
    else
    {
        gtk_icon_set_get_sizes(iconSet, &iconSizes, &numIconSizes);
        iconSize = iconSizes[0];
        g_free(iconSizes);
        //!! figure out proper size and resisze if necessary, see gtk-demo->stock->create_model()
        GBLdirPixbuf = gtk_widget_render_icon(GBLfsTreeView, GTK_STOCK_DIRECTORY, iconSize, NULL);
    }
    /* END CREATE pixbuf for directory */
    
    /* CREATE pixbuf for file */
    iconSet = gtk_icon_factory_lookup_default(GTK_STOCK_FILE);
    if(iconSet == NULL)
        GBLdirPixbuf = NULL;
    else
    {
        gtk_icon_set_get_sizes(iconSet, &iconSizes, &numIconSizes);
        iconSize = iconSizes[0];
        g_free(iconSizes);
        //!! figure out proper size and resisze if necessary, see gtk-demo->stock->create_model()
        GBLfilePixbuf = gtk_widget_render_icon(GBLfsTreeView, GTK_STOCK_FILE, iconSize, NULL);
    }
    /* END CREATE pixbuf for file */
    
    if(GBLappSettings.fsCurrentDir != NULL)
    {
        bool rc;
        
        rc = changeFsDirectory(GBLappSettings.fsCurrentDir);
        if(rc == false)
            /* GBLuserHomeDir has just been set and tested a second ago in findHomeDir() */
            changeFsDirectory(GBLuserHomeDir);
    }
    else
        changeFsDirectory(GBLuserHomeDir);
}

void buildFsLocator(GtkWidget* boxToPackInto)
{
    GBLfsCurrentDirField = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(boxToPackInto), GBLfsCurrentDirField, FALSE, FALSE, 0);
    gtk_entry_set_editable(GTK_ENTRY(GBLfsCurrentDirField), FALSE);
    gtk_widget_show(GBLfsCurrentDirField);
}

bool changeFsDirectory(char* newDirStr)
{
    DIR* newDir;
    struct dirent* nextItem; /* for contents of the directory */
    char* nextItemPathAndName; /* for use with stat() */
    struct stat nextItemInfo;
    GtkTreeIter listIterator;
    int rc;
    GtkTreeModel* model;
    GtkWidget* warningDialog;
    
    newDir = opendir(newDirStr);
    if(newDir == NULL)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to open directory '%s', error %d",
                                               newDirStr,
                                               errno);
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        
        return false;
    }
    
    /* for improved performance disconnect the model from tree view before udating it */
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(GBLfsTreeView));
    g_object_ref(model);
    gtk_tree_view_set_model(GTK_TREE_VIEW(GBLfsTreeView), NULL);
    
    gtk_list_store_clear(GBLfsListStore);
    
    /* it may be possible but in any case very unlikely that readdir() will fail
    * if it does, it returns NULL (same as end of dir) */
    while( (nextItem = readdir(newDir)) != NULL)
    {
        /* skip current and parent directory */
        if(strcmp(nextItem->d_name, ".") == 0 || strcmp(nextItem->d_name, "..") == 0)
            continue;
        
        if(nextItem->d_name[0] == '.' && !GBLappSettings.showHiddenFilesFs)
        /* skip hidden files/dirs */
            continue;
        
        if(strlen(nextItem->d_name) > 256)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "changeFsDirectory(): skiping directory entry because "
                                                   "cannot handle filename longer than 256 chars");
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
            continue;
        }
        
        nextItemPathAndName = (char*)malloc(strlen(newDirStr) + 257);
        if(nextItemPathAndName == NULL)
            fatalError("changeFsDirectory(): malloc(strlen(newDirStr) + 257) failed");
        
        strcpy(nextItemPathAndName, newDirStr);
        strcat(nextItemPathAndName, nextItem->d_name);
        
        rc = stat(nextItemPathAndName, &nextItemInfo);
        if(rc == -1)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "changeFsDirectory(): skiping directory entry because "
                                                   "stat(%s) failed with %d",
                                                   nextItemPathAndName,
                                                   errno);
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
            free(nextItemPathAndName);
            continue;
        }
        
        if(nextItemInfo.st_mode & S_IFDIR)
        /* directory */
        {
            gtk_list_store_append(GBLfsListStore, &listIterator);
            gtk_list_store_set(GBLfsListStore, &listIterator, 
                               COLUMN_ICON, GBLdirPixbuf,
                               COLUMN_FILENAME, nextItem->d_name, 
                               COLUMN_SIZE, 0,
                               COLUMN_HIDDEN_TYPE, FILE_TYPE_DIRECTORY,
                               -1);
        }
        else if(nextItemInfo.st_mode & S_IFREG)
        /* regular file */
        {
            gtk_list_store_append(GBLfsListStore, &listIterator);
            gtk_list_store_set(GBLfsListStore, &listIterator, 
                               COLUMN_ICON, GBLfilePixbuf,
                               COLUMN_FILENAME, nextItem->d_name, 
                               COLUMN_SIZE, nextItemInfo.st_size,
                               COLUMN_HIDDEN_TYPE, FILE_TYPE_REGULAR,
                               -1);
        }
        /* else fancy file, ignore it */
        
        free(nextItemPathAndName);
    
    } /* while (dir contents) */
    
    closedir(newDir);
    
    /* reconnect the model and view now */
    gtk_tree_view_set_model(GTK_TREE_VIEW(GBLfsTreeView), model);
    g_object_unref(model);
    
    /* set current directory string */
    if(GBLfsCurrentDir != NULL)
        free(GBLfsCurrentDir);
    GBLfsCurrentDir = (char*)malloc(strlen(newDirStr) + 1);
    if(GBLfsCurrentDir == NULL)
        fatalError("changeFsDirectory(): malloc(strlen(newDirStr) + 1) failed");
    strcpy(GBLfsCurrentDir, newDirStr);
    
    /* update the field with the path and name */
    gtk_entry_set_text(GTK_ENTRY(GBLfsCurrentDirField), GBLfsCurrentDir);
    
    return true;
}

void fsGoUpDirTree(GtkButton *button, gpointer data)
{
    int count;
    bool done;
    char* newCurrentDir;
    
    /* do nothing if already at root */
    if(GBLfsCurrentDir[0] == '/' && GBLfsCurrentDir[1] == '\0')
        return;
    
    /* need to allocate a new string because changeFsDirectory() uses it 
    * to copy from after freeing GBLfsCurrentDir */
    newCurrentDir = (char*)malloc(strlen(GBLfsCurrentDir) + 1);
    if(newCurrentDir == NULL)
        fatalError("fsGoUpDirTree(): malloc(strlen(GBLfsCurrentDir) + 1) failed");
    strcpy(newCurrentDir, GBLfsCurrentDir);
    
    /* look for the second last slash */
    done = false;
    for(count = strlen(newCurrentDir) - 1; !done; count--)
    {
        if(newCurrentDir[count - 1] == '/')
        /* truncate the string */
        {
            newCurrentDir[count] = '\0';
            changeFsDirectory(newCurrentDir);
            done = true;
        }
    }
    
    free(newCurrentDir);
}

void fsRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                      GtkTreeViewColumn* col, gpointer data)
{
    GtkTreeModel* model;
    GtkTreeIter iterator;
    char* name;
    char* newCurrentDir;
    int fileType;
    GtkWidget* warningDialog;
    
    model = gtk_tree_view_get_model(treeview);
    
    if(gtk_tree_model_get_iter(model, &iterator, path) == FALSE)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "GUI Error: 'fsRowDblClicked(): "
                                               "gtk_tree_model_get_iter() failed'");
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        return;
    }
    
    gtk_tree_model_get(model, &iterator, COLUMN_HIDDEN_TYPE, &fileType, -1);
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        gtk_tree_model_get(model, &iterator, COLUMN_FILENAME, &name, -1);
        
        newCurrentDir = (char*)malloc(strlen(GBLfsCurrentDir) + strlen(name) + 2);
        if(newCurrentDir == NULL)
            fatalError("fsRowDblClicked(): malloc(strlen(GBLfsCurrentDir) + strlen(name) + 2) failed");
        
        strcpy(newCurrentDir, GBLfsCurrentDir);
        strcat(newCurrentDir, name);
        strcat(newCurrentDir, "/");
        
        changeFsDirectory(newCurrentDir);
        
        free(newCurrentDir);
        g_free(name);
    }
}

void refreshFsView(void)
{
    char* fsCurrentDir; /* for changeFsDirectory() */
    
    fsCurrentDir = malloc(strlen(GBLfsCurrentDir) + 1);
    if(fsCurrentDir == NULL)
        fatalError("refreshFsView(): malloc(strlen(GBLfsCurrentDir) + 1) failed");
    strcpy(fsCurrentDir, GBLfsCurrentDir);
    
    changeFsDirectory(fsCurrentDir);
    
    free(fsCurrentDir);
}

void showHiddenCbk(GtkButton *button, gpointer data)
{
    GBLappSettings.showHiddenFilesFs = !GBLappSettings.showHiddenFilesFs;
    
    /* refresh fs view */
    refreshFsView();
}
