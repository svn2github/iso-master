#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

#include "window.h"
#include "fsbrowser.h"
#include "isobrowser.h"

/* menu-sized pixbufs of a directory and a file */
GdkPixbuf* GBLdirPixbuf;
GdkPixbuf* GBLfilePixbuf;

int main(int argc, char** argv)
{
    GtkWidget* mainWindow;
    GtkWidget* mainVBox;
    GtkWidget* vpaned;
    GtkWidget* topFrameBox;
    GtkWidget* topFrame;
    GtkWidget* bottomFrame;
    GtkWidget* bottomFrameBox;
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
    
    vpaned = gtk_vpaned_new();
    gtk_box_pack_start(GTK_BOX(mainVBox), vpaned, TRUE, TRUE, 0);
    gtk_widget_show(vpaned);
    
    /* frame for top file browser */
    topFrame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(topFrame), GTK_SHADOW_IN);
    gtk_paned_pack1(GTK_PANED(vpaned), topFrame, TRUE, FALSE);
    gtk_widget_show(topFrame);
    
    topFrameBox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(topFrame), topFrameBox);
    gtk_widget_show(topFrameBox);
    
    buildFsBrowser(topFrameBox);
    
    /* frame for bottom file browser */
    bottomFrame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(bottomFrame), GTK_SHADOW_IN);
    gtk_paned_pack2(GTK_PANED(vpaned), bottomFrame, TRUE, FALSE);
    gtk_widget_show(bottomFrame);
    
    bottomFrameBox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(bottomFrame), bottomFrameBox);
    gtk_widget_show(bottomFrameBox);
    
    buildMiddleToolbar(bottomFrameBox);
    
    buildIsoBrowser(bottomFrameBox);
    
    statusBar = gtk_statusbar_new();
    gtk_widget_show(statusBar);
    gtk_box_pack_start(GTK_BOX(mainVBox), statusBar, FALSE, FALSE, 0);
    
    gtk_main();
    
    return 0;
}
