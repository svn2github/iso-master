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

#include "isomaster.h"

extern GtkWidget* GBLmainWindow;

#ifdef WINDOWS_BUILD

#include <windows.h>

static GtkWidget* GBLcloseNagBtn;
static bool GBLtimeoutExpired = false;

void closeNagBtnCbk(GtkButton* button, GtkWidget* parentWindow)
{
    gtk_widget_destroy(parentWindow);
}

void enterRegistrationCodeCbk(GtkButton* button, GtkWidget* parentWindow)
{
    GtkWidget* dialog;
    GtkWidget* label;
    GtkWidget* textField;
    int result;
    
    dialog = gtk_dialog_new_with_buttons("Registration code",
                                         GTK_WINDOW(parentWindow),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         NULL);
    
    label = gtk_label_new("Copy-paste or type the registration code here:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, TRUE, 5);
    gtk_widget_show(label);
    
    textField = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(textField), 10);
    g_signal_connect(textField, "activate", (GCallback)acceptDialogCbk, dialog);
    gtk_widget_show(textField);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), textField, TRUE, TRUE, 0);
    
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if(result == GTK_RESPONSE_ACCEPT)
    {
        printf("checking '%s'\n", gtk_entry_get_text(GTK_ENTRY(textField)));fflush(NULL);
    }
    
    gtk_widget_destroy(dialog);
}

void getRegistrationCodeCbk(GtkButton* button, gpointer user_data)
{
    ShellExecute(NULL, "open", 
                 "http://littlesvr.ca/isomaster/buy.php", 
                 NULL, NULL, SW_SHOWNORMAL);
}

gboolean nagCountdown(gpointer data)
{
    static int numSecondsToGo = 1;
    char text[100];
    
    numSecondsToGo--;
    
    sprintf(text, "Continue with demo (%d)", numSecondsToGo);
    gtk_button_set_label(GTK_BUTTON(GBLcloseNagBtn), text);
    
    if(numSecondsToGo == 0)
    {
        gtk_widget_set_sensitive(GBLcloseNagBtn, TRUE);
        GBLtimeoutExpired = true;
        
        return FALSE;
    }
    else
    {
        
        return TRUE;
    }
}

gboolean preventClosingNagWindowCbk(GtkWidget* widget, GdkEvent  *event,
                                    gpointer user_data)
{
    if(GBLtimeoutExpired)
        return FALSE;
    else
        return TRUE;
}

static const char* nagText1 = 
"This is a demo version. The registered version costs 15$.\n"
"Discounts are available for those who have a good reason to ask.";

static const char* nagText2 = 
"Please support the project by paying for your copy.\n";

void showNagScreen(void)
{
    GtkWidget* window;
    GdkPixbuf* appIcon;
    GtkWidget* mainHBox;
    GtkWidget* mainVBox;
    GtkWidget* label;
    GtkWidget* fillerBox;
    GtkWidget* button;
    
    loadAppIcon(&appIcon);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Register ISO Master");
    gtk_window_set_icon(GTK_WINDOW(window), appIcon); /* NULL is ok */
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(GBLmainWindow));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    g_signal_connect(G_OBJECT(window), "delete_event",
                     G_CALLBACK(preventClosingNagWindowCbk), NULL);
    
    mainHBox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), mainHBox);
    gtk_widget_show(mainHBox);
    
    mainVBox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mainHBox), mainVBox, FALSE, TRUE, 5);
    gtk_widget_show(mainVBox);
    
    label = gtk_label_new("ISO Master isn't free!");
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    gtk_box_pack_start(GTK_BOX(mainVBox), label, FALSE, TRUE, 5);
    gtk_widget_show(label);
    
    label = gtk_label_new(nagText1);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    gtk_box_pack_start(GTK_BOX(mainVBox), label, FALSE, TRUE, 5);
    gtk_widget_show(label);
    
    label = gtk_label_new(nagText2);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
    gtk_box_pack_start(GTK_BOX(mainVBox), label, FALSE, TRUE, 5);
    gtk_widget_show(label);
    
    button = gtk_button_new_with_label("Get a registration code");
    gtk_box_pack_start(GTK_BOX(mainVBox), button, FALSE, TRUE, 5);
    gtk_widget_show(button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(getRegistrationCodeCbk), NULL);
    
    button = gtk_button_new_with_label("Enter your registration code");
    gtk_box_pack_start(GTK_BOX(mainVBox), button, FALSE, TRUE, 5);
    gtk_widget_show(button);
    g_signal_connect(G_OBJECT(button), "clicked",
                     G_CALLBACK(enterRegistrationCodeCbk), window);
    
    GBLcloseNagBtn = gtk_button_new_with_label("Continue with demo (6)");
    gtk_box_pack_start(GTK_BOX(mainVBox), GBLcloseNagBtn, FALSE, TRUE, 5);
    gtk_widget_set_sensitive(GBLcloseNagBtn, FALSE);
    gtk_widget_show(GBLcloseNagBtn);
    g_signal_connect(G_OBJECT(GBLcloseNagBtn), "clicked",
                     G_CALLBACK(closeNagBtnCbk), window);
        
    fillerBox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mainVBox), fillerBox, TRUE, TRUE, 5);
    gtk_widget_show(fillerBox);
    
    gtk_widget_show(window);
    
    /* timeout to deal with the close button */
    g_timeout_add(1000, nagCountdown, NULL);
}

#endif
