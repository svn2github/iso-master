void buildIsoBrowser(GtkWidget* boxToPackInto);
void isoGoUpDirTree(GtkButton *button, gpointer data);
void isoRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                       GtkTreeViewColumn* col, gpointer data);
void openIso(char* filename);
void openIsoCbk(GtkMenuItem* menuItem, gpointer data);
