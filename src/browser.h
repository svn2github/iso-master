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

void createDirCbk(GtkButton *button, gpointer onFs);
void formatSize(unsigned long long sizeInt, char* sizeStr, int sizeStrLen);
void sizeCellDataFunc32(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                        GtkTreeModel *model, GtkTreeIter *iter,
                        gpointer data);
void sizeCellDataFunc64(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                        GtkTreeModel *model, GtkTreeIter *iter,
                        gpointer data);
