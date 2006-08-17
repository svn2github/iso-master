void addToIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data);
void addToIsoCbk(GtkButton *button, gpointer data);
void buildIsoBrowser(GtkWidget* boxToPackInto);
void changeIsoDirectory(char* newDirStr);
void deleteFromIsoCbk(GtkButton *button, gpointer data);
void extractFromIsoCbk(GtkButton *button, gpointer data);
void extractFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                              GtkTreeIter* iterator, gpointer data);
void isoGoUpDirTreeCbk(GtkButton *button, gpointer data);
void isoRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                       GtkTreeViewColumn* col, gpointer data);
void openIso(char* filename);
void openIsoCbk(GtkMenuItem* menuItem, gpointer data);
void saveIso(char* filename);
void saveIsoCbk(GtkWidget *widget, GdkEvent *event);
