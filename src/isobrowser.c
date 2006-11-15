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
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include "bk/bk.h"
#include "browser.h"
#include "fsbrowser.h"
#include "isobrowser.h"
#include "error.h"
#include "window.h"
#include "settings.h"

extern GtkWidget* GBLmainWindow;
extern GtkWidget* GBLisoTreeView;
extern GtkListStore* GBLisoListStore;
extern char* GBLisoCurrentDir;
extern GtkWidget* GBLfsTreeView;
extern GtkListStore* GBLfsListStore;
extern char* GBLfsCurrentDir;
extern GtkWidget* GBLisoSizeLbl;
extern GtkWidget* GBLisoCurrentDirField;

/* info about the image being worked on */
VolInfo GBLvolInfo;
/* to know whether am working on an image */
bool GBLisoPaneActive;
/* the size of the iso if it were written right now */
static unsigned long long GBLisoSize = 0;
/* the progress bar from the writing dialog box */
static GtkWidget* GBLWritingProgressBar;
/* the progress bar from the extracting dialog box */
static GtkWidget* GBLextractingProgressBar;

extern GdkPixbuf* GBLdirPixbuf;
extern GdkPixbuf* GBLfilePixbuf;
extern AppSettings GBLappSettings;

void addToIsoCbk(GtkButton *button, gpointer data)
{
    GtkTreeSelection* selection;
    
    if(!GBLisoPaneActive)
    /* no iso open */
        return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLfsTreeView));
    
    gtk_tree_selection_selected_foreach(selection, addToIsoEachRowCbk, NULL);
    
    if(gtk_tree_selection_count_selected_rows(selection) > 0)
    /* reload iso view */
    {
        refreshIsoView();
    }
    
    /* iso size label */
    char sizeStr[20];
    GBLisoSize = 35845;
    //if(GBLvolInfo.filenameTypes & FNTYPE_JOLIET)
        GBLisoSize += 2048;
    GBLisoSize += bk_estimate_iso_size(&GBLvolInfo, FNTYPE_9660 | FNTYPE_JOLIET | FNTYPE_ROCKRIDGE);
    formatSize(GBLisoSize, sizeStr, sizeof(sizeStr));
    gtk_label_set_text(GTK_LABEL(GBLisoSizeLbl), sizeStr);
}

void addToIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* fullItemName; /* with full path */
    int rc;
    GtkWidget* warningDialog;
    
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
        
        rc = bk_add_dir(&GBLvolInfo, fullItemName, GBLisoCurrentDir);
        if(rc <= 0 && rc != BKWARNING_OPER_PARTLY_FAILED)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Failed to add directory %s: '%s'",
                                                   itemName,
                                                   bk_get_error_string(rc));
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
        }
        
        free(fullItemName);
    }
    else if(fileType == FILE_TYPE_REGULAR)
    {
        fullItemName = (char*)malloc(strlen(GBLfsCurrentDir) + strlen(itemName) + 1);
        if(fullItemName == NULL)
            fatalError("addToIsoEachRowCbk(): malloc("
                       "strlen(GBLfsCurrentDir) + strlen(itemName) + 1) failed");
        
        strcpy(fullItemName, GBLfsCurrentDir);
        strcat(fullItemName, itemName);
        
        rc = bk_add_file(&GBLvolInfo, fullItemName, GBLisoCurrentDir);
        if(rc <= 0 && rc != BKWARNING_OPER_PARTLY_FAILED)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Failed to add file %s: '%s'",
                                                   itemName,
                                                   bk_get_error_string(rc));
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
        }
        
        free(fullItemName);
    }
    else
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "GUI error, adding anything other then "
                                               "files and directories doesn't work");
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
    }
    
    g_free(itemName);
}

void buildIsoBrowser(GtkWidget* boxToPackInto)
{
    GtkWidget* scrolledWindow;
    GtkTreeSelection *selection;
    GtkCellRenderer* renderer;
    GtkTreeViewColumn* column;
    
    GBLisoListStore = gtk_list_store_new(NUM_COLUMNS, GDK_TYPE_PIXBUF, 
                                         G_TYPE_STRING, 
                                         G_TYPE_UINT, /* 64-bit sizes not allowed on an iso */
                                         G_TYPE_UINT);
    
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
    //~ gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledWindow), 
                                        //~ GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(boxToPackInto), scrolledWindow, TRUE, TRUE, 0);
    gtk_widget_show(scrolledWindow);
    
    /* view widget */
    GBLisoTreeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(GBLisoListStore));
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(GBLisoTreeView), COLUMN_FILENAME);
    g_object_unref(GBLisoListStore); /* destroy model automatically with view */
    gtk_container_add(GTK_CONTAINER(scrolledWindow), GBLisoTreeView);
    g_signal_connect(GBLisoTreeView , "row-activated", (GCallback)isoRowDblClickCbk, NULL);
    gtk_widget_show(GBLisoTreeView);
    
    /* this won't be enabled until gtk allows me to drag a multiple selection */
    //~ GtkTargetEntry targetEntry;
    //~ targetEntry.target = "text/plain";
    //~ targetEntry.flags = 0;
    //~ targetEntry.info = 0;
    //~ gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(GBLisoTreeView), &targetEntry, 1, GDK_ACTION_COPY);
    
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
    
    gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(GBLisoListStore), COLUMN_FILENAME, 
                                    sortByName, NULL, NULL);
    
    /* size column */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_set_title(column, "Size");
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", COLUMN_SIZE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, sizeCellDataFunc32, NULL, NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_SIZE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(GBLisoTreeView), column);
    
    /* set default sort */
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(GBLisoListStore),
                                         COLUMN_FILENAME, GTK_SORT_ASCENDING);

    gtk_widget_set_sensitive(GBLisoCurrentDirField, FALSE);
    gtk_widget_set_sensitive(GBLisoTreeView, FALSE);
}

void buildIsoLocator(GtkWidget* boxToPackInto)
{
    GBLisoCurrentDirField = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(GBLisoCurrentDirField), FALSE);
    //gtk_widget_set_sensitive(GBLisoCurrentDirField, FALSE);
    gtk_box_pack_start(GTK_BOX(boxToPackInto), GBLisoCurrentDirField, FALSE, FALSE, 0);
    gtk_widget_show(GBLisoCurrentDirField);
}

void changeIsoDirectory(char* newDirStr)
{
    int rc;
    Dir* newDir;
    DirLL* nextDir;
    FileLL* nextFile;
    GtkTreeIter listIterator;
    GtkTreeModel* model;
    GtkWidget* warningDialog;
    
    rc = bk_get_dir_from_string(&GBLvolInfo, newDirStr, &newDir);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to change directory: '%s'",
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
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
                           COLUMN_SIZE, 0,
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
                           COLUMN_SIZE, nextFile->file.size,
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
    
    /* update the field with the path and name */
    gtk_entry_set_text(GTK_ENTRY(GBLisoCurrentDirField), GBLisoCurrentDir);
}

void closeIso(void)
{
    if(!GBLisoPaneActive)
    /* no image open or created, nothing to do */
        return;
    
    bk_destroy_vol_info(&GBLvolInfo);
    
    GBLisoSize = 0;
    gtk_label_set_text(GTK_LABEL(GBLisoSizeLbl), "");
    
    gtk_widget_set_sensitive(GBLisoCurrentDirField, FALSE);
    gtk_widget_set_sensitive(GBLisoTreeView, FALSE);
    
    GBLisoPaneActive = false;
}

void deleteFromIsoCbk(GtkButton *button, gpointer data)
{
    GtkTreeSelection* selection;
    
    if(!GBLisoPaneActive)
    /* no iso open */
        return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLisoTreeView));
    
    gtk_tree_selection_selected_foreach(selection, deleteFromIsoEachRowCbk, NULL);
    
    if(gtk_tree_selection_count_selected_rows(selection) > 0)
    /* reload iso view */
    {
        refreshIsoView();
    }
    
    /* iso size label */
    char sizeStr[20];
    GBLisoSize = 35845;
    //if(GBLvolInfo.filenameTypes & FNTYPE_JOLIET)
        GBLisoSize += 2048;
    GBLisoSize += bk_estimate_iso_size(&GBLvolInfo, FNTYPE_9660 | FNTYPE_JOLIET | FNTYPE_ROCKRIDGE);
    formatSize(GBLisoSize, sizeStr, sizeof(sizeStr));
    gtk_label_set_text(GTK_LABEL(GBLisoSizeLbl), sizeStr);
}

void deleteFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                             GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* fullItemName; /* with full path */
    int rc;
    GtkWidget* warningDialog;
    
    gtk_tree_model_get(model, iterator, COLUMN_HIDDEN_TYPE, &fileType, 
                                        COLUMN_FILENAME, &itemName, -1);
    
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        fullItemName = (char*)malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 2);
        if(fullItemName == NULL)
            fatalError("deleteFromIsoEachRowCbk(): malloc("
                       "strlen(GBLisoCurrentDir) + strlen(itemName) + 2) failed");
        
        strcpy(fullItemName, GBLisoCurrentDir);
        strcat(fullItemName, itemName);
        strcat(fullItemName, "/");
        
        rc = bk_delete_dir(&GBLvolInfo, fullItemName);
        if(rc <= 0)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Failed to delete directory %s: '%s'",
                                                   itemName,
                                                   bk_get_error_string(rc));
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
        }
        
        free(fullItemName);
    }
    else if(fileType == FILE_TYPE_REGULAR)
    {
        fullItemName = (char*)malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1);
        if(fullItemName == NULL)
            fatalError("deleteFromIsoEachRowCbk(): malloc("
                       "strlen(GBLisoCurrentDir) + strlen(itemName) + 1) failed");
        
        strcpy(fullItemName, GBLisoCurrentDir);
        strcat(fullItemName, itemName);
        
        rc = bk_delete_file(&GBLvolInfo, fullItemName);
        if(rc <= 0)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Failed to delete file %s: '%s'",
                                                   itemName,
                                                   bk_get_error_string(rc));
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
        }
        
        free(fullItemName);
    }
    else
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "GUI error, deleting anything other then "
                                               "files and directories doesn't work");
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
    }
    
    g_free(itemName);
}

void extractFromIsoCbk(GtkButton *button, gpointer data)
{
    GtkTreeSelection* selection;
    GtkWidget* progressWindow;
    GtkWidget* descriptionLabel;
    
    if(!GBLisoPaneActive)
    /* no iso open */
        return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GBLisoTreeView));
    
    if(gtk_tree_selection_count_selected_rows(selection) > 0)
    /* reload fs view */
    {
        /* dialog window for the progress bar */
        progressWindow = gtk_dialog_new();
        gtk_dialog_set_has_separator(GTK_DIALOG(progressWindow), FALSE);
        gtk_window_set_modal(GTK_WINDOW(progressWindow), TRUE);
        gtk_window_set_title(GTK_WINDOW(progressWindow), "Progress");
        gtk_widget_show(progressWindow);
        g_signal_connect_swapped(progressWindow, "destroy",
                                 G_CALLBACK(extractingProgressWindowDestroyedCbk), NULL);
        
        /* just some text */
        descriptionLabel = gtk_label_new("Please wait while I'm extracting the selected files...");
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(progressWindow)->vbox), descriptionLabel, TRUE, TRUE, 0);
        gtk_widget_show(descriptionLabel);
        
        /* the progress bar */
        GBLextractingProgressBar = gtk_progress_bar_new();
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(progressWindow)->vbox), GBLextractingProgressBar, TRUE, TRUE, 0);
        gtk_widget_show(GBLextractingProgressBar);
        
        gtk_tree_selection_selected_foreach(selection, extractFromIsoEachRowCbk, NULL);
        
        refreshFsView();
    }
    
    if(GBLextractingProgressBar != NULL)
    /* progress window not closed */
        gtk_widget_destroy(progressWindow);
    GBLextractingProgressBar = NULL;
}

void extractFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                              GtkTreeIter* iterator, gpointer data)
{
    int fileType;
    char* itemName;
    char* fullItemName; /* with full path */
    int rc;
    GtkWidget* warningDialog;
    
    gtk_tree_model_get(model, iterator, COLUMN_HIDDEN_TYPE, &fileType, 
                                        COLUMN_FILENAME, &itemName, -1);
    
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        fullItemName = (char*)malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 2);
        if(fullItemName == NULL)
            fatalError("extractFromIsoEachRowCbk(): malloc("
                       "strlen(GBLisoCurrentDir) + strlen(itemName) + 2) failed (out of memory?)");
        
        strcpy(fullItemName, GBLisoCurrentDir);
        strcat(fullItemName, itemName);
        strcat(fullItemName, "/");
        
        rc = bk_extract_dir(&GBLvolInfo, fullItemName, GBLfsCurrentDir, 
                            false, extractingProgressUpdaterCbk);
        if(rc <= 0 && rc != BKWARNING_OPER_PARTLY_FAILED) /* ignore that warning, already shown to user in the warning callback */
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Failed to extract directory %s: '%s'",
                                                   itemName,
                                                   bk_get_error_string(rc));
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
        }
        
        free(fullItemName);
    }
    else if(fileType == FILE_TYPE_REGULAR)
    {
        fullItemName = (char*)malloc(strlen(GBLisoCurrentDir) + strlen(itemName) + 1);
        if(fullItemName == NULL)
            fatalError("extractFromIsoEachRowCbk(): malloc("
                       "strlen(GBLisoCurrentDir) + strlen(itemName) + 1) failed");
        
        strcpy(fullItemName, GBLisoCurrentDir);
        strcat(fullItemName, itemName);
        
        rc = bk_extract_file(&GBLvolInfo, fullItemName, GBLfsCurrentDir, 
                             false, extractingProgressUpdaterCbk);
        if(rc <= 0)
        {
            warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Failed to extract file %s: '%s'",
                                                   itemName,
                                                   bk_get_error_string(rc));
            gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
            gtk_dialog_run(GTK_DIALOG(warningDialog));
            gtk_widget_destroy(warningDialog);
        }
        
        free(fullItemName);
    }
    else
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "GUI error, extracting anything other then "
                                               "files and directories doesn't work");
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
    }
    
    g_free(itemName);
}

void extractingProgressUpdaterCbk(void)
{
    if(GBLextractingProgressBar != NULL)
    {
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(GBLextractingProgressBar)); 
    
        /* redraw progress bar */
        while(gtk_events_pending())
            gtk_main_iteration();
    }
}

void extractingProgressWindowDestroyedCbk(void)
{
    GBLextractingProgressBar = NULL;
}

void isoGoUpDirTreeCbk(GtkButton *button, gpointer data)
{
    int count;
    bool done;
    char* newCurrentDir;
    GtkWidget* warningDialog;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return;
    
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
    for(count = strlen(newCurrentDir) - 1; !done && count >= 0; count--)
    {
        if(newCurrentDir[count - 1] == '/')
        /* truncate the string */
        {
            newCurrentDir[count] = '\0';
            changeIsoDirectory(newCurrentDir);
            done = true;
        }
    }
    
    if(!done)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "GUI error, GBLisoCurrentDir is not '/' and has "
                                               "only one slash, please report bug.");
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
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
    GtkWidget* warningDialog;
    
    model = gtk_tree_view_get_model(treeview);
    
    if(gtk_tree_model_get_iter(model, &iterator, path) == FALSE)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "GUI error: 'isoRowDblClicked(): "
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
    /* else do nothing (not a directory) */
}

/* This callback is also used by an accelerator so make sure you don't use 
* the parameters, since they may not be the menuitem parameters */
gboolean newIsoCbk(GtkMenuItem* menuItem, gpointer data)
{
    closeIso();
    
    bk_init_vol_info(&GBLvolInfo);
    GBLvolInfo.warningCbk = operationFailed;
    
    GBLappSettings.filenameTypesToWrite = FNTYPE_9660 | FNTYPE_ROCKRIDGE | FNTYPE_JOLIET;
    
    /* iso size label */
    char sizeStr[20];
    GBLisoSize = 35845;
    //if(GBLvolInfo.filenameTypes & FNTYPE_JOLIET)
        GBLisoSize += 2048;
    GBLisoSize += bk_estimate_iso_size(&GBLvolInfo, FNTYPE_9660 | FNTYPE_JOLIET | FNTYPE_ROCKRIDGE);
    formatSize(GBLisoSize, sizeStr, sizeof(sizeStr));
    gtk_label_set_text(GTK_LABEL(GBLisoSizeLbl), sizeStr);
    
    gtk_widget_set_sensitive(GBLisoCurrentDirField, TRUE);
    gtk_widget_set_sensitive(GBLisoTreeView, TRUE);
    
    GBLisoPaneActive = true;
    
    changeIsoDirectory("/");
    
    /* the accelerator callback must return true */
    return TRUE;
}

void openIso(char* filename)
{
    int rc;
    GtkWidget* warningDialog;
    
    closeIso();
    
    bk_init_vol_info(&GBLvolInfo);
    GBLvolInfo.warningCbk = operationFailed;
    
    GBLappSettings.filenameTypesToWrite = FNTYPE_9660 | FNTYPE_ROCKRIDGE | FNTYPE_JOLIET;
    
    rc = bk_open_image(&GBLvolInfo, filename);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to open iso file for reading: %s",
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        return;
    }
    
    rc = bk_read_vol_info(&GBLvolInfo);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to read volume info: '%s'",
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        closeIso();
        return;
    }
    
    /* READ entire directory tree */
    if(GBLvolInfo.filenameTypes & FNTYPE_ROCKRIDGE)
        rc = bk_read_dir_tree(&GBLvolInfo, FNTYPE_ROCKRIDGE, true);
    else if(GBLvolInfo.filenameTypes & FNTYPE_JOLIET)
        rc = bk_read_dir_tree(&GBLvolInfo, FNTYPE_JOLIET, false);
    else
        rc = bk_read_dir_tree(&GBLvolInfo, FNTYPE_9660, false);
    if(rc <= 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to read directory tree: '%s'",
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        closeIso();
        return;
    }
    /* END READ entire directory tree */
    
    /* iso size label */
    char sizeStr[20];
    GBLisoSize = 35845;
    //if(GBLvolInfo.filenameTypes & FNTYPE_JOLIET)
        GBLisoSize += 2048;
    GBLisoSize += bk_estimate_iso_size(&GBLvolInfo, FNTYPE_9660 | FNTYPE_JOLIET | FNTYPE_ROCKRIDGE);
    formatSize(GBLisoSize, sizeStr, sizeof(sizeStr));
    gtk_label_set_text(GTK_LABEL(GBLisoSizeLbl), sizeStr);
    
    gtk_widget_set_sensitive(GBLisoCurrentDirField, TRUE);
    gtk_widget_set_sensitive(GBLisoTreeView, TRUE);
    
    changeIsoDirectory("/");
    
    GBLisoPaneActive = true;
}

/* This callback is also used by an accelerator so make sure you don't use 
* the parameters, since they may not be the menuitem parameters */
gboolean openIsoCbk(GtkMenuItem* menuItem, gpointer data)
{
    GtkWidget *dialog;
    char* filename;
    GtkFileFilter* nameFilter;
    
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    nameFilter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(nameFilter), "*.[iI][sS][oO]");
    gtk_file_filter_set_name(GTK_FILE_FILTER(nameFilter), "ISO Images");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(nameFilter));
    
    nameFilter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(nameFilter), "*");
    gtk_file_filter_set_name(GTK_FILE_FILTER(nameFilter), "All files");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(nameFilter));
    
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        openIso(filename);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
    //~ openIso("image.iso");
    
    /* the accelerator callback must return true */
    return TRUE;
}

bool operationFailed(const char* msg)
{
    GtkWidget* warningDialog;
    gint response;
    
    warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_WARNING,
                                           GTK_BUTTONS_YES_NO,
                                           "%s\n\nDo you wish to continue?",
                                           msg);
    gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
    response = gtk_dialog_run(GTK_DIALOG(warningDialog));
    gtk_widget_destroy(warningDialog);
    
    if(response == GTK_RESPONSE_YES)
        return true;
    else
        return false;
}

void refreshIsoView(void)
{
    char* isoCurrentDir; /* for changeIsoDirectory() */
    
    isoCurrentDir = malloc(strlen(GBLisoCurrentDir) + 1);
    if(isoCurrentDir == NULL)
        fatalError("addToIsoCbk(): malloc("
                   "strlen(GBLisoCurrentDir) + 1) failed");
    strcpy(isoCurrentDir, GBLisoCurrentDir);
    
    changeIsoDirectory(isoCurrentDir);
    
    free(isoCurrentDir);
}

void saveIso(char* filename)
{
    int rc;
    GtkWidget* progressWindow;
    GtkWidget* descriptionLabel;
    GtkWidget* okButton;
    GtkWidget* warningDialog;
    
    /* dialog window for the progress bar */
    progressWindow = gtk_dialog_new();
    gtk_dialog_set_has_separator(GTK_DIALOG(progressWindow), FALSE);
    gtk_window_set_modal(GTK_WINDOW(progressWindow), TRUE);
    gtk_window_set_title(GTK_WINDOW(progressWindow), "Progress");
    gtk_widget_show(progressWindow);
    g_signal_connect_swapped(progressWindow, "response",
                             G_CALLBACK(gtk_widget_destroy), progressWindow);
    g_signal_connect_swapped(progressWindow, "destroy",
                             G_CALLBACK(writingProgressWindowDestroyedCbk), NULL);
    
    /* just some text */
    descriptionLabel = gtk_label_new("Please wait while I'm saving the new image to disk...");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(progressWindow)->vbox), descriptionLabel, TRUE, TRUE, 0);
    gtk_widget_show(descriptionLabel);
    
    /* the progress bar */
    GBLWritingProgressBar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(progressWindow)->vbox), GBLWritingProgressBar, TRUE, TRUE, 0);
    gtk_widget_show(GBLWritingProgressBar);
    
    /* button to close the dialog (disabled until writing finished) */
    okButton = gtk_dialog_add_button(GTK_DIALOG(progressWindow), GTK_STOCK_OK, GTK_RESPONSE_NONE);
    gtk_widget_set_sensitive(okButton, FALSE);
    
    /* write new image */
    rc = bk_write_image(filename, &GBLvolInfo, time(NULL), GBLappSettings.filenameTypesToWrite, 
                        writingProgressUpdaterCbk);
    if(rc < 0)
    {
        warningDialog = gtk_message_dialog_new(GTK_WINDOW(GBLmainWindow),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "Failed to write image to '%s': '%s'",
                                               filename,
                                               bk_get_error_string(rc));
        gtk_window_set_modal(GTK_WINDOW(warningDialog), TRUE);
        gtk_dialog_run(GTK_DIALOG(warningDialog));
        gtk_widget_destroy(warningDialog);
        /* enable the ok button so the user can close the progress window */
        gtk_widget_set_sensitive(okButton, TRUE);
    }
    
    if(GBLWritingProgressBar != NULL)
    /* progress window hasn't been destroyed */
    {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(GBLWritingProgressBar), 1.0);
        gtk_widget_set_sensitive(okButton, TRUE);
    }
}

/* This callback is also used by an accelerator so make sure you don't use 
* the parameters, since they may not be the menuitem parameters */
gboolean saveIsoCbk(GtkWidget *widget, GdkEvent *event)
{
    GtkWidget *dialog;
    char* filename;
    int dialogResponse;
    GtkFileFilter* nameFilter;
    
    /* do nothing if no image open */
    if(!GBLisoPaneActive)
        return TRUE;
    
    dialog = gtk_file_chooser_dialog_new("Save File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    nameFilter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(nameFilter), "*.[iI][sS][oO]");
    gtk_file_filter_set_name(GTK_FILE_FILTER(nameFilter), "ISO Images");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(nameFilter));
    
    nameFilter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(GTK_FILE_FILTER(nameFilter), "*");
    gtk_file_filter_set_name(GTK_FILE_FILTER(nameFilter), "All files");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(nameFilter));
    
    dialogResponse = gtk_dialog_run(GTK_DIALOG(dialog));
    if(dialogResponse == GTK_RESPONSE_ACCEPT)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    
    gtk_widget_destroy(dialog);
    
    if(dialogResponse == GTK_RESPONSE_ACCEPT)
    {
        saveIso(filename);
        
        g_free(filename);
    }
    
    //~ saveIso("out.iso");

    /* the accelerator callback must return true */
    return TRUE;
}

void writingProgressUpdaterCbk(void)
{
    if(GBLWritingProgressBar != NULL)
    {
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(GBLWritingProgressBar)); 
        
        /* redraw progress bar */
        while(gtk_events_pending())
            gtk_main_iteration();
    }
}

void writingProgressWindowDestroyedCbk(void)
{
    GBLWritingProgressBar = NULL;
}
