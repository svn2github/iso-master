#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "browser.h"
#include "fsbrowser.h"
#include "error.h"

/* set when the fs browser is constructed */
static char* GBLuserHomeDir;

/* the view used for the contents of the fs browser */
static GtkWidget* GBLfsTreeView;
/* the list store used for the contents of the fs browser */
static GtkListStore* GBLfsListStore;
/* slash-terminated, the dir being displayed in the browser */
static char* GBLfsCurrentDir = NULL;

/* menu-sized pixbufs of a directory and a file */
extern GdkPixbuf* GBLdirPixbuf;
extern GdkPixbuf* GBLfilePixbuf;

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
    
    char* userHomeDir;
    
    GBLfsListStore = gtk_list_store_new(NUM_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    
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
    
    /* STORE the user's home directory */
    userHomeDir = getenv("HOME");
    if(userHomeDir == NULL)
    /* pretend user's home is root */
    {
        printWarning("failed to getenv(\"HOME\"), using \"/\" as home directory");
        GBLuserHomeDir = (char*)malloc(2);
        if(GBLuserHomeDir == NULL)
            fatalError("buildFsBrowser(): malloc(2) failed");
        GBLuserHomeDir[0] = '/';
        GBLuserHomeDir[1] = '\0';
    }
    else
    {
        int pathLen;
        
        /* need the directory ending with a / (bkisofs rule) */
        
        pathLen = strlen(userHomeDir);
        if(userHomeDir[pathLen] == '/')
        {
            GBLuserHomeDir = (char*)malloc(pathLen + 1);
            if(GBLuserHomeDir == NULL)
                fatalError("buildFsBrowser(): malloc(pathLen + 1) failed");
            strcpy(GBLuserHomeDir, userHomeDir);
        }
        else
        {
            GBLuserHomeDir = (char*)malloc(pathLen + 2);
            if(GBLuserHomeDir == NULL)
                fatalError("buildFsBrowser(): malloc(pathLen + 2) failed");
            strcpy(GBLuserHomeDir, userHomeDir);
            strcat(GBLuserHomeDir, "/");
        }
    }
    /* END STORE the user's home directory */
    
    changeFsDirectory(GBLuserHomeDir);
}

void changeFsDirectory(char* newDirStr)
{
    DIR* newDir;
    struct dirent* nextItem; /* for contents of the directory */
    char* nextItemPathAndName; /* for use with stat() */
    struct stat nextItemInfo;
    GtkTreeIter listIterator;
    int rc;
    GtkTreeModel* model;
    
    newDir = opendir(newDirStr);
    if(newDir == NULL)
    {
        printWarning("changeFsDirectory(): failed to opendir(newDirStr)");
        return;
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
        
        /* skip hidden files/dirs */
        if(nextItem->d_name[0] == '.')
            continue;
        
        if(strlen(nextItem->d_name) > 256)
            fatalError("changeFsDirectory(): cannot handle filename longer then 256 chars");
        
        nextItemPathAndName = (char*)malloc(strlen(newDirStr) + 257);
        strcpy(nextItemPathAndName, newDirStr);
        strcat(nextItemPathAndName, nextItem->d_name);
        
        rc = stat(nextItemPathAndName, &nextItemInfo);
        if(rc == -1)
            fatalError("changeFsDirectory(): stat(newSrcPathAndName, &nextItemInfo) failed");
        
        if(nextItemInfo.st_mode & S_IFDIR)
        /* directory */
        {
            gtk_list_store_append(GBLfsListStore, &listIterator);
            gtk_list_store_set(GBLfsListStore, &listIterator, 
                               COLUMN_ICON, GBLdirPixbuf,
                               COLUMN_FILENAME, nextItem->d_name, 
                               COLUMN_SIZE, "dir",
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
                               COLUMN_SIZE, "file",
                               COLUMN_HIDDEN_TYPE, FILE_TYPE_REGULAR,
                               -1);
        }
        else
        /* fancy file, ignore it */
        {
            free(nextItemPathAndName);
            continue;
        }
        
        free(nextItemPathAndName);
    
    } /* while (dir contents) */
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(GBLfsTreeView), model);
    g_object_unref(model);
    
    /* set current directory string */
    if(GBLfsCurrentDir != NULL)
        free(GBLfsCurrentDir);
    GBLfsCurrentDir = (char*)malloc(strlen(newDirStr) + 1);
    if(GBLfsCurrentDir == NULL)
        fatalError("changeFsDirectory(): malloc(strlen(newDirStr)) failed");
    strcpy(GBLfsCurrentDir, newDirStr);
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
                      GtkTreeViewColumn* col, gpointer userdata)
{
    GtkTreeModel* model;
    GtkTreeIter iterator;
    char* name;
    char* newCurrentDir;
    int fileType;
    
    model = gtk_tree_view_get_model(treeview);
    
    if(gtk_tree_model_get_iter(model, &iterator, path) == FALSE)
    {
        printWarning("fsRowDblClicked(): gtk_tree_model_get_iter() failed");
        return;
    }
    
    gtk_tree_model_get(model, &iterator, COLUMN_HIDDEN_TYPE, &fileType, -1);
    if(fileType == FILE_TYPE_DIRECTORY)
    {
        gtk_tree_model_get(model, &iterator, COLUMN_FILENAME, &name, -1);
        
        newCurrentDir = (char*)malloc(strlen(GBLfsCurrentDir) + strlen(name) + 2);
        if(newCurrentDir == NULL)
            fatalError("fsRowDblClicked(): malloc(newCurrentDirlen) failed");
        
        strcpy(newCurrentDir, GBLfsCurrentDir);
        strcat(newCurrentDir, name);
        strcat(newCurrentDir, "/");
        
        changeFsDirectory(newCurrentDir);
        
        free(newCurrentDir);
        g_free(name);
    }
}
