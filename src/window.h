void buildMainToolbar(GtkWidget* boxToPackInto);
void buildMenu(GtkWidget* boxToPackInto);
void buildMiddleToolbar(GtkWidget* boxToPackInto);
gboolean closeMainWindowCbk(GtkWidget *widget, GdkEvent *event);
void closeWindowCbk(GtkWidget *widget, GdkEvent *event);
void loadIcons(void);
void loadIcon(GtkWidget** destIcon, const char* srcFile, int size);
void sortDirsFirstCbk(GtkButton *button, gpointer data);
