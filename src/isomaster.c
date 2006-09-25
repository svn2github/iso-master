/******************************* LICENCE **************************************
* Any code in this file may be redistributed or modified under the terms of
* the GNU General Public Licence as published by the Free Software 
* Foundation; version 2 of the licence.
****************************** END LICENCE ***********************************/

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

#include "window.h"
#include "fsbrowser.h"
#include "isobrowser.h"
#include "settings.h"

GtkWidget* GBLmainWindow;
/* to be able to resize the two file browsers */
GtkWidget* GBLbrowserPaned;
/* icon for 'new directory' for fs browser */
GtkWidget* GBLnewDirIcon;
/* icon for 'new directory' for iso browser */
GtkWidget* GBLnewDirIcon2;

extern AppSettings GBLappSettings;

int main(int argc, char** argv)
{
    GdkPixbuf* appIcon;
    GdkPixbuf* newDirPixbuf;
    GtkWidget* mainVBox;
    GtkWidget* mainFrame; /* to put a border around the window contents */
    GtkWidget* topPanedBox; /* to pack the top part of GBLbrowserPaned */
    GtkWidget* bottomPanedBox; /* to pack the bottom part of GBLbrowserPaned */
    GtkWidget* statusBar;
    
    gtk_init(&argc, &argv);
    
    findHomeDir();
    
    loadSettings();
    
    /* ICONPATH is defined in the makefile */
    appIcon = gdk_pixbuf_new_from_file(ICONPATH"/isomaster.svg", NULL);
    
    newDirPixbuf = gdk_pixbuf_new_from_file(ICONPATH"/folder-new.png", NULL);
    if(newDirPixbuf == NULL)
    /* could not load icon but need one so replace it with 'unknown' from stock  */
    {
        GBLnewDirIcon = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_MENU);
        GBLnewDirIcon2 = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_MENU);
    }
    else
    /* resize the icon loaded */
    {
        int menuSize;
        gtk_icon_size_lookup(GTK_ICON_SIZE_MENU, &menuSize, &menuSize);
        newDirPixbuf = gdk_pixbuf_scale_simple(newDirPixbuf, menuSize, menuSize, GDK_INTERP_HYPER);
        GBLnewDirIcon = gtk_image_new_from_pixbuf(newDirPixbuf);
        GBLnewDirIcon2 = gtk_image_new_from_pixbuf(newDirPixbuf);
    }
    
    /* main window */
    GBLmainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(GBLmainWindow), 
                                GBLappSettings.windowWidth, GBLappSettings.windowHeight);
    gtk_window_set_title(GTK_WINDOW(GBLmainWindow), "ISO Master");
    gtk_window_set_icon(GTK_WINDOW(GBLmainWindow), appIcon); /* NULL is ok */
    gtk_widget_show(GBLmainWindow);
    g_signal_connect(G_OBJECT(GBLmainWindow), "delete_event",
                     G_CALLBACK(closeMainWindowCbk), NULL);
    
    mainVBox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(GBLmainWindow), mainVBox);
    gtk_widget_show(mainVBox);
    
    buildMenu(mainVBox);
    
    buildMainToolbar(mainVBox);
    
    buildFsLocator(mainVBox);
    
    mainFrame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(mainFrame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(mainVBox), mainFrame, TRUE, TRUE, 0);
    gtk_widget_show(mainFrame);
    
    GBLbrowserPaned = gtk_vpaned_new();
    gtk_container_add(GTK_CONTAINER(mainFrame), GBLbrowserPaned);
    gtk_widget_show(GBLbrowserPaned);
    gtk_paned_set_position(GTK_PANED(GBLbrowserPaned), GBLappSettings.topPaneHeight);
    
    topPanedBox = gtk_vbox_new(FALSE, 0);
    gtk_paned_pack1(GTK_PANED(GBLbrowserPaned), topPanedBox, TRUE, FALSE);
    gtk_widget_show(topPanedBox);
    
    buildFsBrowser(topPanedBox);
    
    bottomPanedBox = gtk_vbox_new(FALSE, 0);
    gtk_paned_pack2(GTK_PANED(GBLbrowserPaned), bottomPanedBox, TRUE, FALSE);
    gtk_widget_show(bottomPanedBox);
    
    buildMiddleToolbar(bottomPanedBox);
    
    buildIsoLocator(bottomPanedBox);
    
    buildIsoBrowser(bottomPanedBox);
    
    statusBar = gtk_statusbar_new();
    gtk_widget_show(statusBar);
    gtk_box_pack_start(GTK_BOX(mainVBox), statusBar, FALSE, FALSE, 0);
    
    gtk_main();
    
    return 0;
}
