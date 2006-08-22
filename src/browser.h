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

void formatSize(unsigned sizeInt, char* sizeStr, int sizeSize);
void sizeCellDataFunc(GtkTreeViewColumn *col, GtkCellRenderer *renderer,
                      GtkTreeModel *model, GtkTreeIter *iter,
                      gpointer data);
