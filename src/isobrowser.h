void activityProgressUpdaterCbk(VolInfo* volInfo);
gboolean activityProgressWindowDeleteCbk(GtkWidget* widget, GdkEvent* event,
                                         gpointer user_data);
void addToIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data);
void addToIsoCbk(GtkButton *button, gpointer data);
void buildIsoBrowser(GtkWidget* boxToPackInto);
void buildIsoLocator(GtkWidget* boxToPackInto);
void cancelOperation(GtkDialog* dialog, gint arg1, gpointer user_data);
void changeIsoDirectory(char* newDirStr);
void closeIso(void);
bool confirmCloseIso(void);
void deleteFromIsoCbk(GtkButton *button, gpointer data);
void deleteFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                             GtkTreeIter* iterator, gpointer data);
void extractFromIsoCbk(GtkButton *button, gpointer data);
void extractFromIsoEachRowCbk(GtkTreeModel* model, GtkTreePath* path,
                              GtkTreeIter* iterator, gpointer data);
//~ gboolean isoButtonPressedCbk(GtkWidget* widget, GdkEventButton* event, gpointer user_data);
void isoGoUpDirTreeCbk(GtkButton *button, gpointer data);
gboolean isoKeyPressedCbk(GtkWidget* widget, GdkEventKey* event, gpointer user_data);
void isoRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                       GtkTreeViewColumn* col, gpointer data);
gboolean newIsoCbk(GtkMenuItem* menuItem, gpointer data);
void openIso(char* filename);
gboolean openIsoCbk(GtkMenuItem* menuItem, gpointer data);
bool operationFailed(const char* msg);
void refreshIsoView(void);
void renameSelected(void);
void renameSelectedCbk(GtkTreeModel* model, GtkTreePath* path,
                           GtkTreeIter* iterator, gpointer data);
void saveIso(char* filename);
gboolean saveIsoCbk(GtkWidget *widget, GdkEvent *event);
//~ void showIsoContextMenu(GtkWidget* isoView, GdkEventButton* event);
void writingProgressResponse(GtkDialog* dialog, gint arg1, gpointer user_data);
void writingProgressUpdaterCbk(VolInfo* volInfo, double percentComplete);
gboolean writingProgressWindowDeleteCbk(GtkWidget* widget, GdkEvent* event,
                                        gpointer user_data);
void writingProgressWindowDestroyedCbk(void);
