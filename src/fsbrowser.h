void buildFsBrowser(GtkWidget* boxToPackInto);
void changeFsDirectory(char* newDirStr);
void fsRowDblClicked(GtkTreeView* treeview, GtkTreePath* path,
                     GtkTreeViewColumn* col, gpointer userdata);
