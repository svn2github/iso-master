void buildFsBrowser(GtkWidget* boxToPackInto);
void changeFsDirectory(char* newDirStr);
void fsGoUpDirTree(GtkButton *button, gpointer data);
void fsRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                      GtkTreeViewColumn* col, gpointer userdata);
