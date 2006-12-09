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
#include <gdk/gdkkeysyms.h>
#include <libintl.h>

#include "window.h"
#include "browser.h"
#include "fsbrowser.h"
#include "isobrowser.h"
#include "about.h"
#include "settings.h"
#include "boot.h"
#include "bk/bk.h"

#define _(str) gettext(str)

/* the label that holds the value of the iso size */
GtkWidget* GBLisoSizeLbl;
/* check menu item for 'show hidden files' */
GtkWidget* GBLshowHiddenMenuItem;
/* check menu item for 'sort directories first' */
GtkWidget* GBLsortDirsFirst;
/* icon for 'new directory' for fs browser */
GtkWidget* GBLnewDirIcon;
/* icon for 'new directory' for iso browser */
GtkWidget* GBLnewDirIcon2;
/* 'add to iso' icon */
GtkWidget* GBLaddIcon;
/* 'extract from iso' icon */
GtkWidget* GBLextractIcon;

extern GtkWidget* GBLmainWindow;
extern AppSettings GBLappSettings;
extern bool GBLisoPaneActive;
extern bool GBLisoChangesProbable;

void buildMainToolbar(GtkWidget* boxToPackInto)
{
    GtkWidget* toolbar;
    GtkWidget* icon;
    GtkWidget* button;
    
    toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(boxToPackInto), toolbar, FALSE, FALSE, 0);
    gtk_widget_show(toolbar);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_LARGE_TOOLBAR);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("Go back"), 
                                     _("Go back up one directory on the filesystem"), "Private",
                                     icon, G_CALLBACK(fsGoUpDirTreeCbk),
                                     NULL);
    
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("New Directory"), 
                                     _("Create new directory on the filesystem"), "Private",
                                     GBLnewDirIcon, G_CALLBACK(createDirCbk), (gpointer)1);
}

void buildMenu(GtkWidget* boxToPackInto)
{
    GtkWidget* menuBar;
    GtkWidget* menu;
    GtkWidget* menuItem;
    GtkWidget* separator;
    //~ GtkWidget* icon;
    GtkWidget* rootMenu;
    GtkAccelGroup* accelGroup;
    guint accelKey;
    GdkModifierType accelModifier;
    GClosure *closure = NULL;
    
    /* KEYBOARD accelerators */
    accelGroup = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(GBLmainWindow), accelGroup);
    
    gtk_accelerator_parse("<Control>N", &accelKey, &accelModifier);
    closure = g_cclosure_new(G_CALLBACK(newIsoCbk), NULL, NULL);
    gtk_accel_group_connect(accelGroup, accelKey, accelModifier, GTK_ACCEL_VISIBLE, closure);
    gtk_accel_map_add_entry("<ISOMaster>/Image/New", accelKey, accelModifier);
    
    gtk_accelerator_parse("<Control>O", &accelKey, &accelModifier);
    closure = g_cclosure_new(G_CALLBACK(openIsoCbk), NULL, NULL);
    gtk_accel_group_connect(accelGroup, accelKey, accelModifier, GTK_ACCEL_VISIBLE, closure);
    gtk_accel_map_add_entry("<ISOMaster>/Image/Open", accelKey, accelModifier);
    
    gtk_accelerator_parse("<Control>S", &accelKey, &accelModifier);
    closure = g_cclosure_new(G_CALLBACK(saveIsoCbk), NULL, NULL);
    gtk_accel_group_connect(accelGroup, accelKey, accelModifier, GTK_ACCEL_VISIBLE, closure);
    gtk_accel_map_add_entry("<ISOMaster>/Image/Save", accelKey, accelModifier);
    
    gtk_accelerator_parse("<Control>W", &accelKey, &accelModifier);
    closure = g_cclosure_new(G_CALLBACK(closeMainWindowCbk), NULL, NULL);
    gtk_accel_group_connect(accelGroup, accelKey, accelModifier, GTK_ACCEL_VISIBLE, closure);
    
    gtk_accelerator_parse("<Control>Q", &accelKey, &accelModifier);
    closure = g_cclosure_new(G_CALLBACK(closeMainWindowCbk), NULL, NULL);
    gtk_accel_group_connect(accelGroup, accelKey, accelModifier, GTK_ACCEL_VISIBLE, closure);
    gtk_accel_map_add_entry("<ISOMaster>/Image/Quit", accelKey, accelModifier);
    /* END KEYBOARD accelerators */
    
    menuBar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(boxToPackInto), menuBar, FALSE, FALSE, 0);
    gtk_widget_show(menuBar);
    
    /* FILE menu */
    rootMenu = gtk_menu_item_new_with_mnemonic(_("_Image"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), rootMenu);
    gtk_widget_show(rootMenu);
    
    menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootMenu), menu);
    gtk_menu_set_accel_group(GTK_MENU(menu), accelGroup);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
    //~ icon = gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_NEW);
    //~ gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), icon);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(newIsoCbk), NULL);
    gtk_menu_item_set_accel_path(GTK_MENU_ITEM(menuItem), "<ISOMaster>/Image/New");
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(openIsoCbk), NULL);
    gtk_menu_item_set_accel_path(GTK_MENU_ITEM(menuItem), "<ISOMaster>/Image/Open");
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(saveIsoCbk), NULL);
    gtk_menu_item_set_accel_path(GTK_MENU_ITEM(menuItem), "<ISOMaster>/Image/Save");
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PROPERTIES, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(buildImagePropertiesWindow), NULL);
    
    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    gtk_widget_show(separator);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(closeMainWindowCbk), NULL);
    gtk_menu_item_set_accel_path(GTK_MENU_ITEM(menuItem), "<ISOMaster>/Image/Quit");
    /* END FILE menu */
    
    /* VIEW menu */
    rootMenu = gtk_menu_item_new_with_mnemonic(_("_View"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), rootMenu);
    gtk_widget_show(rootMenu);
    
    menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootMenu), menu);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_REFRESH, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(refreshBothViewsCbk), NULL);
    
    GBLshowHiddenMenuItem = gtk_check_menu_item_new_with_mnemonic(_("_Show hidden files"));
    if(GBLappSettings.showHiddenFilesFs)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(GBLshowHiddenMenuItem), TRUE);
    else
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(GBLshowHiddenMenuItem), FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GBLshowHiddenMenuItem);
    gtk_widget_show(GBLshowHiddenMenuItem);
    g_signal_connect(G_OBJECT(GBLshowHiddenMenuItem), "activate",
                     G_CALLBACK(showHiddenCbk), NULL);
    
    GBLsortDirsFirst = gtk_check_menu_item_new_with_mnemonic(_("_Sort directories first"));
    if(GBLappSettings.sortDirectoriesFirst)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(GBLsortDirsFirst), TRUE);
    else
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(GBLsortDirsFirst), FALSE);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GBLsortDirsFirst);
    gtk_widget_show(GBLsortDirsFirst);
    g_signal_connect(G_OBJECT(GBLsortDirsFirst), "activate",
                     G_CALLBACK(sortDirsFirstCbk), NULL);
    /* END VIEW menu */
    
    /* BOOT menu */
    rootMenu = gtk_menu_item_new_with_mnemonic(_("_BootRecord"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), rootMenu);
    gtk_widget_show(rootMenu);
    
    menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootMenu), menu);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PROPERTIES, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(showBootInfoCbk), NULL);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(extractBootRecordCbk), NULL);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(deleteBootRecordCbk), NULL);
    
    GtkWidget* submenu;
    GtkWidget* rootSubmenu;
    
    rootSubmenu = gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), rootSubmenu);
    gtk_widget_show(rootSubmenu);
    
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootSubmenu), submenu);
    
    menuItem = gtk_menu_item_new_with_label(_("Use selected file on image (no emulation)"));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(setFileAsBootRecordCbk), NULL);
    
    menuItem = gtk_menu_item_new_with_label(_("From file: no emulation"));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(addBootRecordFromFileCbk), (gpointer)BOOT_MEDIA_NO_EMULATION);
    
    menuItem = gtk_menu_item_new_with_label(_("From file: 1200KiB floppy"));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(addBootRecordFromFileCbk), (gpointer)BOOT_MEDIA_1_2_FLOPPY);
    
    menuItem = gtk_menu_item_new_with_label(_("From file: 1440KiB floppy"));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(addBootRecordFromFileCbk), (gpointer)BOOT_MEDIA_1_44_FLOPPY);
    
    menuItem = gtk_menu_item_new_with_label(_("From file: 2880KiB floppy"));
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuItem);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(addBootRecordFromFileCbk), (gpointer)BOOT_MEDIA_2_88_FLOPPY);
    /* END BOOT menu */
    
    /* HELP menu */
    rootMenu = gtk_menu_item_new_with_mnemonic(_("_Help"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), rootMenu);
    gtk_widget_show(rootMenu);
    
    menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootMenu), menu);
    
    menuItem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
    //gtk_widget_set_sensitive(menuItem, FALSE);
    gtk_widget_show(menuItem);
    g_signal_connect(G_OBJECT(menuItem), "activate",
                     G_CALLBACK(showAboutWindowCbk), NULL);
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
    gtk_box_pack_start(GTK_BOX(hBox), toolbar, FALSE, FALSE, 0);
    gtk_widget_show(toolbar);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_LARGE_TOOLBAR);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("Go back"), 
                                     _("Go back up one directory on the ISO"), "Private",
                                     icon, G_CALLBACK(isoGoUpDirTreeCbk),
                                     NULL);
    
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("New Directory"), 
                                     _("Create new directory on the ISO"), "Private",
                                     GBLnewDirIcon2, G_CALLBACK(createDirCbk), (gpointer)0);
    
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("Add"), _("Add to the ISO"), "Private",
                                     GBLaddIcon, G_CALLBACK(addToIsoCbk),
                                     NULL);

    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("Extract"), _("Extract from the ISO"), "Private",
                                     GBLextractIcon, G_CALLBACK(extractFromIsoCbk),
                                     NULL);

    icon = gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_LARGE_TOOLBAR);
    button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                     _("Remove"), _("Delete from the ISO"), "Private",
                                     icon, G_CALLBACK(deleteFromIsoCbk),
                                     NULL);
    
    sizeTitleLabel = gtk_label_new(_("      Estimated ISO Size: "));
    gtk_box_pack_start(GTK_BOX(hBox), sizeTitleLabel, FALSE, FALSE, 0);
    gtk_widget_show(sizeTitleLabel);
    
    GBLisoSizeLbl = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(hBox), GBLisoSizeLbl, FALSE, FALSE, 0);
    gtk_widget_show(GBLisoSizeLbl);
}

gboolean closeMainWindowCbk(GtkWidget *widget, GdkEvent *event)
{
    if(GBLisoChangesProbable && !confirmCloseIso())
        return TRUE;
    
    writeSettings();
    
    printf("Quitting\n");
    
    gtk_main_quit();
    
    /* the accelerator callback must return true */
    return TRUE;
}

void closeWindowCbk(GtkWidget *widget, GdkEvent *event)
{
    gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_REJECT);
}

void loadIcon(GtkWidget** destIcon, const char* srcFile, int size)
{
    GdkPixbuf* pixbuf;
    
    pixbuf = gdk_pixbuf_new_from_file(srcFile, NULL);
    if(pixbuf == NULL)
    /* could not load icon but need one so replace it with 'unknown' from stock  */
    {
        *destIcon = gtk_image_new_from_stock(GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_MENU);
    }
    else
    /* resize the icon loaded */
    {
        pixbuf = gdk_pixbuf_scale_simple(pixbuf, size, size, GDK_INTERP_HYPER);
        *destIcon = gtk_image_new_from_pixbuf(pixbuf);
    }
}

void loadIcons(void)
{
    int size;
    
    gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR, &size, &size);
    
    loadIcon(&GBLnewDirIcon, ICONPATH"/folder-new-tango.png", size);
    loadIcon(&GBLnewDirIcon2, ICONPATH"/folder-new-tango.png", size);
    loadIcon(&GBLaddIcon, ICONPATH"/add2-kearone.png", size);
    loadIcon(&GBLextractIcon, ICONPATH"/extract-kearone.png", size);
}

void sortDirsFirstCbk(GtkButton *button, gpointer data)
{
    GBLappSettings.sortDirectoriesFirst = !GBLappSettings.sortDirectoriesFirst;
    
    refreshFsView();
    if(GBLisoPaneActive)
        refreshIsoView();
}
