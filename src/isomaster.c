#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

#include "window.h"

int main(int argc, char** argv)
{
    GtkWidget* mainWindow;
    GtkWidget* mainVBox;
    GtkWidget* vpaned;
    GtkWidget* frame2box;
    GtkWidget* frame1;
    GtkWidget* frame2;
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
    
    /* 2 FRAMES for the file browsers */
    vpaned = gtk_vpaned_new();
    gtk_box_pack_start(GTK_BOX(mainVBox), vpaned, TRUE, TRUE, 0);
    gtk_widget_show(vpaned);
    
    frame1 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);
    gtk_paned_pack1(GTK_PANED(vpaned), frame1, TRUE, FALSE);
    gtk_widget_show(frame1);
    
    frame2box = gtk_vbox_new(FALSE, 0);
    gtk_paned_pack2(GTK_PANED(vpaned), frame2box, TRUE, FALSE);
    gtk_widget_show(frame2box);
    
    buildMiddleToolbar(frame2box);
    
    frame2 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(frame2box), frame2, TRUE, TRUE, 0);
    gtk_widget_show(frame2);
    /* END 2 FRAMES for the file browsers */
    
    statusBar = gtk_statusbar_new();
    gtk_widget_show(statusBar);
    gtk_box_pack_start(GTK_BOX(mainVBox), statusBar, FALSE, FALSE, 0);
    
    gtk_main();
    
    return 0;
}
