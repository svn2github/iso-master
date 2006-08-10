#include <gtk/gtk.h>

/* this file just has the global variables used by the two file browsers */

/* menu-sized pixbufs of a directory and a file */
GdkPixbuf* GBLdirPixbuf;
GdkPixbuf* GBLfilePixbuf;

/* the view used for the contents of the fs browser */
GtkWidget* GBLfsTreeView;
/* the list store used for the contents of the fs browser */
GtkListStore* GBLfsListStore;
/* slash-terminated, the dir being displayed in the fs browser */
char* GBLfsCurrentDir = NULL;

/* the view used for the contents of the fs browser */
GtkWidget* GBLisoTreeView;
/* the list store used for the contents of the fs browser */
GtkListStore* GBLisoListStore;
/* slash-terminated, the dir being displayed in the iso browser */
char* GBLisoCurrentDir = NULL;
