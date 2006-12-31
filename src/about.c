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

static const char* 
GBLprogramName = "ISO Master 0.6";

static const char* 
GBLauthors[2] = {
"Many thanks to all the following people:\n"
"\n"
"Andrew Smith\n"
"http://littlesvr.ca/misc/contactandrew.php\n"
"Summer 2005 - Fall 2006\n"
"- author and maintainer\n"
"\n"
"Barb Czegel\n"
"http://cs.senecac.on.ca/~barb.czegel/\n"
"Summer-fall 2005\n"
"- one of my teachers from Seneca College who has been kind enough to let me\n"
"  work on ISO Master as my systems project for two semesters\n"
"\n"
"Nicolas Devillard\n"
"http://ndevilla.free.fr/iniparser/\n"
"August 2006\n"
"- the excellent iniparser, for storing and reading config files\n"
"\n"
"The Samba Project\n"
"http://samba.org/\n"
"December 2005\n"
"- most of the filename mangling code I copied from samba\n"
"\n"
"Steffen Winterfeldt\n"
"September 2006\n"
"- helped me figure out how to work with isolinux boot records\n"
"\n"
"Roman Hubatsch (kearone)\n"
"http://kearone.deviantart.com/\n"
"December 2006\n"
"- the excellent 'add' and 'extract' icons and the new Isomaster icon\n"
"\n"
"Tango Desktop Project\n"
"http://tango.freedesktop.org/Tango_Icon_Gallery\n"
"August 2006\n"
"- the pretty 'new folder' icon\n"
"\n"
"David Johnson\n"
"http://www.david-web.co.uk/\n"
"September 2006\n"
"- a patch to allow associating ISO Master with ISO files in file managers\n"
,
NULL};

static const char* 
GBLtranslators = 
"bg (Bulgarian) - Dessislav Petrov\n"
"\n"
"de (German) - Toni Graffy\n"
"\n"
"fr (French) - Leif Thande\n"
"\n"
"pl (Polish) - Marciej Libuda\n"
"\n"
"ru (Russian) - Anton Obidin\n"
"\n"
"sq (Albanian) - Besnik Bleta\n"
"\n"
"zh_TW (Chineese/Taiwan) - Cheng-Wei Chien\n"
;
static const char* 
GBLcomments = "An application for editing ISO9660 images based on the "
              "bkisofs access library and the GTK2 GUI toolkit.";

static const char* 
GBLcopyright = "Copyright 2005-2006 Andrew Smith";

static const char* 
GBLwebsite = "http://littlesvr.ca/isomaster/";

static const char* 
GBLlicense = 
"ISO Master and bkisofs are distributed under the GNU General Public Licence\n"
"version 2, please see LICENCE.TXT for the complete text\n";

void showAboutWindowCbk(GtkMenuItem* menuItem, gpointer data)
{
    gtk_show_about_dialog(GTK_WINDOW(GBLmainWindow), 
                          "name", GBLprogramName,
                          "authors", GBLauthors,
                          "translator-credits", GBLtranslators,
                          "comments", GBLcomments,
                          "copyright", GBLcopyright,
                          "license", GBLlicense,
                          "website", GBLwebsite,
                          NULL);
}

void showHelpOverviewCbk(GtkMenuItem* menuItem, gpointer data)
{
    gtk_show_about_dialog(GTK_WINDOW(GBLmainWindow), 
                          "name", GBLprogramName,
                          "authors", GBLauthors,
                          "translator-credits", GBLtranslators,
                          "comments", GBLcomments,
                          "copyright", GBLcopyright,
                          "license", GBLlicense,
                          "website", GBLwebsite,
                          NULL);
}
