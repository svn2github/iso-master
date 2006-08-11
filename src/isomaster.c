#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

#include "window.h"
#include "fsbrowser.h"
#include "isobrowser.h"

int main(int argc, char** argv)
{
    GtkWidget* mainWindow;
    GtkWidget* mainVBox;
    GtkWidget* mainFrame; /* to put a border around the window contents */
    GtkWidget* vpaned; /* to be able to resize the two file browsers */
    GtkWidget* topPanedBox; /* to pack the top part of vpaned */
    GtkWidget* bottomPanedBox; /* to pack the bottom part of vpaned */
    GtkWidget* statusBar;
    
    gtk_init(&argc, &argv);
    
    /* main window */
    mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(mainWindow), 640, 480);
    gtk_window_set_title(GTK_WINDOW(mainWindow), "ISO Master");
    gtk_widget_show(mainWindow);
    g_signal_connect(G_OBJECT(mainWindow), "delete_event",
                     G_CALLBACK(closeMainWindowCbk), NULL);
    
    mainVBox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(mainWindow), mainVBox);
    gtk_widget_show(mainVBox);
    
    buildMenu(mainVBox);
    buildMainToolbar(mainVBox);
    
    mainFrame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(mainFrame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(mainVBox), mainFrame, TRUE, TRUE, 0);
    gtk_widget_show(mainFrame);
    
    vpaned = gtk_vpaned_new();
    gtk_container_add(GTK_CONTAINER(mainFrame), vpaned);
    gtk_widget_show(vpaned);
    
    topPanedBox = gtk_vbox_new(FALSE, 0);
    gtk_paned_pack1(GTK_PANED(vpaned), topPanedBox, TRUE, FALSE);
    gtk_widget_show(topPanedBox);
    
    buildFsBrowser(topPanedBox);
    
    bottomPanedBox = gtk_vbox_new(FALSE, 0);
    gtk_paned_pack2(GTK_PANED(vpaned), bottomPanedBox, TRUE, FALSE);
    gtk_widget_show(bottomPanedBox);
    
    buildMiddleToolbar(bottomPanedBox);
    
    buildIsoBrowser(bottomPanedBox);
    
    statusBar = gtk_statusbar_new();
    gtk_widget_show(statusBar);
    gtk_box_pack_start(GTK_BOX(mainVBox), statusBar, FALSE, FALSE, 0);
    
    gtk_main();
    
    return 0;
}
