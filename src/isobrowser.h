void addToIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data);
void addToIsoCbk(GtkButton *button, gpointer data);
void buildIsoBrowser(GtkWidget* boxToPackInto);
void buildIsoLocator(GtkWidget* boxToPackInto);
void cancelOper(GtkDialog* dialog, gint arg1, gpointer user_data);
void changeIsoDirectory(char* newDirStr);
void closeIso(void);
bool confirmCloseIso(void);
void deleteFromIsoCbk(GtkButton *button, gpointer data);
void deleteFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                             GtkTreeIter* iterator, gpointer data);
void extractFromIsoCbk(GtkButton *button, gpointer data);
void extractFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                              GtkTreeIter* iterator, gpointer data);
void extractingProgressUpdaterCbk(void);
void extractingProgressWindowDestroyedCbk(void);
void isoGoUpDirTreeCbk(GtkButton *button, gpointer data);
void isoRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                       GtkTreeViewColumn* col, gpointer data);
gboolean newIsoCbk(GtkMenuItem* menuItem, gpointer data);
void openIso(char* filename);
gboolean openIsoCbk(GtkMenuItem* menuItem, gpointer data);
bool operationFailed(const char* msg);
void refreshIsoView(void);
void saveIso(char* filename);
gboolean saveIsoCbk(GtkWidget *widget, GdkEvent *event);
void writingProgressUpdaterCbk(void);
void writingProgressWindowDestroyedCbk(void);
