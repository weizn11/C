#ifndef _SHELL_H_
#define _SHELL_H_

#include "global.h"
#include "clientlist.h"
#include "recv.h"
#include "send.h"
#include "gui.h"

void Execute_Command(gpointer Parameter);
int Send_Command(GtkWidget *widget,gpointer pss);

#endif    //_SHELL_H_
