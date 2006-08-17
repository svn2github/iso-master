#include <gtk/gtk.h>

extern GtkWidget* GBLmainWindow;

static const char* 
GBLprogramName = "ISO Master";

static const char* 
GBLauthors[3] = {"Andrew Smith", 
                 "http://littlesvr.ca/misc/contactandrew.php", 
                 NULL};

static const char* 
GBLcomments = "An application for editing ISO9660 images based on the "
              "bkisofs access library and the GTK2 GUI toolkit.";

static const char* 
GBLcopyright = "Copyright 2005-2006 Andrew Smith";

static const char* 
GBLwebsite = "http://littlesvr.ca/isomaster/";

static const char* 
GBLlicense = "GNU General Public Licence Version 2";

void showAboutWindowCbk(GtkMenuItem* menuItem, gpointer data)
{
    gtk_show_about_dialog(GTK_WINDOW(GBLmainWindow), 
                          "name", GBLprogramName,
                          "authors", GBLauthors,
                          "comments", GBLcomments,
                          "copyright", GBLcopyright,
                          "license", GBLlicense,
                          "website", GBLwebsite,
                          NULL);
}
