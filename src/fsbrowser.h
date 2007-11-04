#include <stdbool.h>

void acceptFsPathCbk(GtkEntry *entry, gpointer user_data);
void buildFsBrowser(GtkWidget* boxToPackInto);
void buildFsLocator(GtkWidget* boxToPackInto);
bool changeFsDirectory(const char* newDirStr);
void fsGoUpDirTreeCbk(GtkButton *button, gpointer data);
void fsRowDblClickCbk(GtkTreeView* treeview, GtkTreePath* path,
                      GtkTreeViewColumn* col, gpointer data);
void refreshFsView(void);
void showHiddenCbk(GtkButton *button, gpointer data);
