void addToIsoCbk(GtkButton *button, gpointer data);
void buildIsoBrowser(GtkWidget* boxToPackInto);
void isoGoUpDirTreeCbk(GtkButton *button, gpointer data);
void isoRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                       GtkTreeViewColumn* col, gpointer data);
void openIso(char* filename);
void openIsoCbk(GtkMenuItem* menuItem, gpointer data);
