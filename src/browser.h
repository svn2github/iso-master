enum
{
    COLUMN_ICON = 0,
    COLUMN_FILENAME,
    COLUMN_SIZE,
    COLUMN_HIDDEN_TYPE,
    NUM_COLUMNS
};

enum
{
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY
};

void acceptDialogCbk(GtkEntry *entry, GtkDialog* dialog);
void createDirCbk(GtkButton *button, gpointer onFs);
void formatSize(unsigned long long sizeInt, char* sizeStr, int sizeStrLen);
void refreshBothViewsCbk(GtkWidget *widget, GdkEvent *event);
void sizeCellDataFunc32(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                        GtkTreeModel *model, GtkTreeIter *iter,
                        gpointer data);
void sizeCellDataFunc64(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                        GtkTreeModel *model, GtkTreeIter *iter,
                        gpointer data);
gint sortByName(GtkTreeModel *model, GtkTreeIter *a, 
                GtkTreeIter *b, gpointer userdata);
