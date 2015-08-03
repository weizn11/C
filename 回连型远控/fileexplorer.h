#ifndef _FILEEXPLORER_
#define _FILEEXPLORER_

#include "global.h"
#include "gui.h"

void file_explorer_tree_selection(GtkWidget *view,GtkTreePath *path,GtkTreeViewColumn *col,gpointer Parameter);
void file_explorer(gpointer Parameter);


#endif _FILEEXPLORER_
