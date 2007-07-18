void addToTempFilesList(const char* pathAndName);
void destroyTempFilesList(void);
void editSelectedRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data);
void editSelectedBtnCbk(GtkMenuItem *menuitem, gpointer data);
char* makeRandomFilename(const char* sourceName);
void viewSelectedBtnCbk(GtkMenuItem *menuitem, gpointer data);
void viewSelectedRowCbk(GtkTreeModel* model, GtkTreePath* path,
                        GtkTreeIter* iterator, gpointer data);

/******************************************************************************
* FileCreated
* Linked list node, each having the path and name of a temporary file that 
* ISO Master created (for example while editing a file). */
typedef struct TempFileCreated
{
    char* pathAndName;
    
    struct TempFileCreated* next;
    
} TempFileCreated;
