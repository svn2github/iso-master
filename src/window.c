#include <gtk/gtk.h>

#include "window.h"
#include "fsbrowser.h"
#include "isobrowser.h"
#include "about.h"

/* the label that holds the value of the iso size */
GtkWidget* GBLisoSizeLbl;

void buildMainToolbar(GtkWidget* boxToPackInto)
{
    GtkWidget* toolbar;
    GtkWidget* icon;
    GtkWidget* button;
    
    toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(boxToPackInto), toolbar, FALSE, FALSE, 0);
    gtk_widget_show(toolbar);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Open", "Open ISO Image", "Private",
                                     icon, G_CALLBACK(openIsoCbk),
                                     NULL);

    icon = gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Save", "Save ISO Image", "Private",
                                     icon, G_CALLBACK(saveIsoCbk),
                                     NULL);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Go back", "Go back up one directory", "Private",
                                     icon, G_CALLBACK(fsGoUpDirTree),
                                     NULL);
}

void buildMenu(GtkWidget* boxToPackInto)
{
    GtkWidget* menuBar;
    GtkWidget* menu;
    GtkWidget* menuItem;
    GtkWidget* icon;
    GtkWidget* rootMenu;
    
    menuBar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(boxToPackInto), menuBar, FALSE, FALSE, 0);
    gtk_widget_show(menuBar);
    
    /* FILE menu */
    menu = gtk_menu_new();
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
    icon = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), icon);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(openIsoCbk), NULL);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, NULL);
    icon = gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), icon);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(saveIsoCbk), NULL);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    icon = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), icon);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(closeMainWindowCbk), NULL);
    
    rootMenu = gtk_menu_item_new_with_mnemonic("_Image");
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), rootMenu);
    gtk_widget_show(rootMenu);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootMenu), menu);
    /* END FILE menu */
    
    /* HELP menu */
    menu = gtk_menu_new();
    
    //menuItem = gtk_menu_item_new_with_mnemonic("_About");
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    //gtk_widget_set_sensitive(menuItem, FALSE);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(showAboutWindowCbk), NULL);
    
    rootMenu = gtk_menu_item_new_with_mnemonic("_Help");
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), rootMenu);
    gtk_widget_show(rootMenu);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootMenu), menu);
    /* END HELP menu */
}

void buildMiddleToolbar(GtkWidget* boxToPackInto)
{
    GtkWidget* toolbar;
    GtkWidget* icon;
    GtkWidget* button;
    GtkWidget* hBox;
    GtkWidget* sizeTitleLabel;
    
    hBox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(boxToPackInto), hBox, FALSE, FALSE, 0);
    gtk_widget_show(hBox);
    
    toolbar = gtk_toolbar_new();
    //gtk_box_pack_start(GTK_BOX(boxToPackInto), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox), toolbar, FALSE, FALSE, 0);
    gtk_widget_show(toolbar);
    
    sizeTitleLabel = gtk_label_new("            Estimated ISO Size: ");
    gtk_box_pack_start(GTK_BOX(hBox), sizeTitleLabel, FALSE, FALSE, 0);
    gtk_widget_show(sizeTitleLabel);
    
    GBLisoSizeLbl = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hBox), GBLisoSizeLbl, FALSE, FALSE, 0);
    gtk_widget_show(GBLisoSizeLbl);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Go back", "Go back up one directory", "Private",
                                     icon, G_CALLBACK(isoGoUpDirTreeCbk),
                                     NULL);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Add", "Add to ISO Image", "Private",
                                     icon, G_CALLBACK(addToIsoCbk),
                                     NULL);

    icon = gtk_image_new_from_stock(GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Extract", "Extract from ISO Image", "Private",
                                     icon, G_CALLBACK(extractFromIsoCbk),
                                     NULL);

    icon = gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     "Remove", "Remove from ISO Image", "Private",
                                     icon, G_CALLBACK(deleteFromIsoCbk),
                                     NULL);
}

gboolean closeMainWindowCbk(GtkWidget *widget, GdkEvent *event)
{
    printf("Quitting\n");
    
    gtk_main_quit();
    
    return FALSE; /* quit */
}