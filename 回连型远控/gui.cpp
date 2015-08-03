#include "gui.h"

GtkWidget *create_window(char *title,int border_width,int length,int width)
{
    GtkWidget *window;

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	EnterCriticalSection(&CRI_ATU);
    gtk_window_set_title(GTK_WINDOW(window),ANSIToUTF8(title));
	LeaveCriticalSection(&CRI_ATU);
    gtk_container_set_border_width(GTK_CONTAINER(window),border_width);
    gtk_widget_set_size_request(GTK_WIDGET(window),length,width);
    gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
    gtk_window_set_position( GTK_WINDOW(window),GTK_WIN_POS_CENTER_ALWAYS);

	g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_widget_destroy),window);

    return window;
}

int update_system_config(GtkWidget *widget,GtkWidget *entry)
{
	FILE *file=NULL;
	char *strbuffer=NULL;

	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(entry));
	LISTEN_PORT=atoi(strbuffer);

	if((file=fopen("config","wb"))==NULL)
	{
		MessageBox(NULL,"Cant't open configuration file!","Error",MB_OK);
		return -1;
	}
	fwrite(&LISTEN_PORT,sizeof(int),1,file);
	fclose(file);
	MessageBox(NULL,"Change the Settings successfully!Please run this program again。","Success",MB_OK);

	return 0;
}

int system_config()
{
	GtkWidget *window;
	GtkWidget *label;
	GtkWidget *port_entry;
	GtkWidget *button;
	GtkWidget *hbox,*vbox;
	char str[20];

	memset(str,NULL,sizeof(str));
	sprintf(str,"%d",LISTEN_PORT);

	window=create_window("System Setup",10,200,100);
	EnterCriticalSection(&CRI_ATU);
	label=gtk_label_new(ANSIToUTF8("Listening port:"));
	LeaveCriticalSection(&CRI_ATU);
	port_entry=gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(port_entry),str);
	EnterCriticalSection(&CRI_ATU);
	button=gtk_button_new_with_label(ANSIToUTF8("Change Setting"));
	LeaveCriticalSection(&CRI_ATU);
	hbox=gtk_hbox_new(FALSE,2);
	vbox=gtk_vbox_new(TRUE,5);

	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),port_entry,TRUE,TRUE,2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_box_pack_start(GTK_BOX(vbox),button,TRUE,TRUE,5);
	gtk_container_add(GTK_CONTAINER(window),vbox);

	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(update_system_config),port_entry);

	gtk_widget_show_all(window);

	return 0;
}

GtkWidget *create_menu_bar()
{
	GtkWidget *menu_bar;
	GtkWidget *file_menu,*choose_menu;
	GtkWidget *menu_item;
	GtkWidget *file_menu_item,*choose_menu_item;

	menu_bar=gtk_menu_bar_new();
	file_menu=gtk_menu_new();
	EnterCriticalSection(&CRI_ATU);
	file_menu_item=gtk_menu_item_new_with_label(ANSIToUTF8("File"));
	menu_item=gtk_menu_item_new_with_label(ANSIToUTF8("Exit"));

	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(gtk_main_quit),NULL);
	gtk_menu_append(GTK_MENU(file_menu),menu_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item),file_menu);
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar),file_menu_item);

	choose_menu=gtk_menu_new();
	choose_menu_item=gtk_menu_item_new_with_label(ANSIToUTF8("Options"));
	menu_item=gtk_menu_item_new_with_label(ANSIToUTF8("System Setup"));

	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(system_config),NULL);
	gtk_menu_append(GTK_MENU(choose_menu),menu_item);

	LeaveCriticalSection(&CRI_ATU);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(choose_menu_item),choose_menu);
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar),choose_menu_item);
	
	return menu_bar;
}

GtkWidget *create_list()
{
    GtkWidget *scrolled_window;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    scrolled_window=gtk_scrolled_window_new(NULL,NULL);    //创建一个滑动窗口
    gtk_widget_set_size_request(GTK_WIDGET(scrolled_window),790,270);
    tree_view=gtk_tree_view_new();      //新建树状列表构建
    main_list_store=gtk_list_store_new(MAX_Column,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    //设置滚动条出现方式
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view),GTK_TREE_MODEL(main_list_store));
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window),tree_view);   //将列表构件放到滚动构件里

    renderer=gtk_cell_renderer_text_new();
	EnterCriticalSection(&CRI_ATU);
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("NO.\t"),renderer,"text",HostID_Column,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),GTK_TREE_VIEW_COLUMN(column));    //追加列名

    renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Host Name\t\t\t"),renderer,"text",HostName_Column,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),GTK_TREE_VIEW_COLUMN(column));    //追加列名

    renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Host IP\t\t\t"),renderer,"text",HostIP_Column,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),GTK_TREE_VIEW_COLUMN(column));    //追加列名

    renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Host OS\t\t\t\t"),renderer,"text",HostOS_Column,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),GTK_TREE_VIEW_COLUMN(column));    //追加列名

    renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Note"),renderer,"text",Notes_Column,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),GTK_TREE_VIEW_COLUMN(column));    //追加列名
	LeaveCriticalSection(&CRI_ATU);

    selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));     //添加选择信号

    return scrolled_window;
}

int add_to_list(ClientListNode CLN)
{
    GtkTreeIter iter;

    gtk_list_store_append(main_list_store,&iter);
    gtk_list_store_set(main_list_store,&iter,HostID_Column,TotalList,-1);
	EnterCriticalSection(&CRI_ATU);
    gtk_list_store_set(main_list_store,&iter,HostName_Column,ANSIToUTF8(CLN.name),-1);
    gtk_list_store_set(main_list_store,&iter,HostIP_Column,ANSIToUTF8(CLN.ip),-1);
    gtk_list_store_set(main_list_store,&iter,HostOS_Column,ANSIToUTF8(CLN.OS),-1);
    if(CLN.HTTP_Proxy)
        gtk_list_store_set(main_list_store,&iter,Notes_Column,ANSIToUTF8("Use HTTP Proxy"),-1);

	LeaveCriticalSection(&CRI_ATU);
    TotalList++;

    return 0;
}

int remove_item(int list_index)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

//	printf("移除条目:%d\n",list_index);
    model=gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
    gtk_tree_model_iter_nth_child(model,&iter,NULL,list_index);  //获取对应条目的迭代器
    gtk_list_store_remove(main_list_store, &iter);               //删除列表中对应的条目
    TotalList--;

    gtk_tree_model_iter_nth_child(model,&iter,NULL,list_index);   //获取删除条目的下一条迭代器
    for (list_index++; list_index<TotalList; list_index++)
    {
        gtk_list_store_set(main_list_store,&iter,HostID_Column,list_index,-1);
        gtk_tree_model_iter_nth_child(model,&iter,NULL,list_index);
    }

    return 0;
}

void file_ok_sel(GtkWidget *widget,gpointer Parameter)
{
	Explorer_Struct *es=(Explorer_Struct *)Parameter;
	TranStructParameter *TSP=NULL;
	char remote_file[MAX_SIZE];
	char *ptmp=NULL;
	char *str=NULL;
	GtkTreeModel *model;
    GtkTreeIter iter;

	TSP=(TranStructParameter *)malloc(sizeof(TranStructParameter));
	if(TSP==NULL)
	{
		MessageBox(NULL,"malloc error","Error",MB_OK);
		exit(-1);
	}
	memset(TSP,NULL,sizeof(TranStructParameter));
	memset(remote_file,NULL,sizeof(remote_file));

	EnterCriticalSection(&es->cs);
	EnterCriticalSection(&CRI_UTA);
	strcat(TSP->local_path,UTF8ToANSI(gtk_file_selection_get_filename(GTK_FILE_SELECTION(es->LocalFileSelection))));
	LeaveCriticalSection(&CRI_UTA);
	if(es->tran_file_flag==1)
	{
		for(ptmp=&(TSP->local_path[strlen(TSP->local_path)-1]);*ptmp!='\\';ptmp--);
	    ptmp++;
		strcat(TSP->remote_path,es->path);
	    strcat(TSP->remote_path,ptmp);
	}
	else
	{
		if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(es->file_selection),&model,&iter))
		{
			LeaveCriticalSection(&es->cs);
			return;
		}
	    gtk_tree_model_get(model,&iter,0,&str,-1);
		EnterCriticalSection(&CRI_UTA);
		strcat(remote_file,UTF8ToANSI(str));
		LeaveCriticalSection(&CRI_UTA);
		strcat(TSP->remote_path,es->path);
	    strcat(TSP->remote_path,remote_file);
		g_free(str);
		str=NULL;
	}
	TSP->ClientSocket=es->ClientSocket;
	TSP->flag=2;
	gtk_widget_destroy(GTK_WIDGET(es->LocalFileSelection));
	if(es->tran_file_flag==1)
	    upload_window(NULL,TSP);
	else
		download_window(NULL,TSP);
	LeaveCriticalSection(&es->cs);
	free(TSP);
	return;
}

void file_explorer_upload(GtkWidget *widget,gpointer Parameter)
{
	Explorer_Struct *es=(Explorer_Struct *)Parameter;
	GtkWidget *local_file_selection;

	/* 创建一个新的文件选择构件 */
	EnterCriticalSection(&es->cs);
	local_file_selection=gtk_file_selection_new ("File Selection");
	es->LocalFileSelection=GTK_FILE_SELECTION(local_file_selection);
	gtk_widget_set_size_request(GTK_WIDGET(local_file_selection),700,400);
	gtk_window_set_position( GTK_WINDOW(local_file_selection),GTK_WIN_POS_CENTER_ALWAYS);
	es->tran_file_flag=1;
	LeaveCriticalSection(&es->cs);
	g_signal_connect(G_OBJECT (local_file_selection),"destroy",G_CALLBACK (gtk_widget_destroy),local_file_selection);

	g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (local_file_selection)->cancel_button),"clicked",
		G_CALLBACK (gtk_widget_destroy),local_file_selection);

	/* 为ok_button按钮设置回调函数，连接到file_ok_sel function函数 */
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (local_file_selection)->ok_button),"clicked",\
		G_CALLBACK (file_ok_sel),es);

	gtk_widget_show (local_file_selection);
	return;
}

void file_explorer_down(GtkWidget *widget,gpointer Parameter)
{
	Explorer_Struct *es=(Explorer_Struct *)Parameter;
	GtkWidget *local_file_selection;
	char *str=NULL;
	GtkTreeModel *model;
    GtkTreeIter iter;
	
	/* 创建一个新的文件选择构件 */
	EnterCriticalSection(&es->cs);
	local_file_selection=gtk_file_selection_new ("File Selection");
	es->LocalFileSelection=GTK_FILE_SELECTION(local_file_selection);
	gtk_widget_set_size_request(GTK_WIDGET(local_file_selection),700,400);
	gtk_window_set_position( GTK_WINDOW(local_file_selection),GTK_WIN_POS_CENTER_ALWAYS);
	
	if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(es->file_selection),&model,&iter))
	{
		LeaveCriticalSection(&es->cs);
		return;
	}
	gtk_tree_model_get(model,&iter,0,&str,-1);
	gtk_file_selection_set_filename (GTK_FILE_SELECTION(local_file_selection),str);
	g_free(str);
	str=NULL;

	es->tran_file_flag=2;
	LeaveCriticalSection(&es->cs);

	g_signal_connect(G_OBJECT (local_file_selection),"destroy",G_CALLBACK (gtk_widget_destroy),local_file_selection);
	
	g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (local_file_selection)->cancel_button),"clicked",
		G_CALLBACK (gtk_widget_destroy),local_file_selection);
	
	/* 为ok_button按钮设置回调函数，连接到file_ok_sel function函数 */
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (local_file_selection)->ok_button),"clicked",\
		G_CALLBACK (file_ok_sel),es);
	
	gtk_widget_show (local_file_selection);
	return;
}

void file_explorer_refresh(GtkWidget *widget,gpointer Parameter)
{
	Explorer_Struct *es=(Explorer_Struct *)Parameter;
	EnterCriticalSection(&es->cs);
	es->refresh_flag=1;
	LeaveCriticalSection(&es->cs);
	return;
}

void file_explorer_delete(GtkWidget *widget,gpointer Parameter)
{
	Explorer_Struct *es=(Explorer_Struct *)Parameter;
	char *str=NULL;
	GtkTreeModel *model;
    GtkTreeIter iter;

	EnterCriticalSection(&es->cs);
	if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(es->file_selection),&model,&iter))
	{
		LeaveCriticalSection(&es->cs);
		return;
	}
	gtk_tree_model_get(model,&iter,0,&str,-1);     //获取要删除的文件名
	es->filename=str;

	es->delete_flag=1;
	LeaveCriticalSection(&es->cs);

	return;
}

void pop_flie_explorer_menu(GtkWidget *widget,GdkEventButton *event,gpointer Parameter)
{
	Explorer_Struct *es=(Explorer_Struct *)Parameter;
	GtkWidget *menu;
	GtkWidget *item;
	
	if(event->type==GDK_BUTTON_RELEASE && event->button==3)
	{
		menu=gtk_menu_new();
		item=gtk_menu_item_new_with_label("Upload File");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(file_explorer_upload),es);
		item=gtk_menu_item_new_with_label("Download File");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(file_explorer_down),es);
		item=gtk_menu_item_new_with_label("Refresh");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(file_explorer_refresh),es);
		item=gtk_menu_item_new_with_label("Delete");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(file_explorer_delete),es);
		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
	}
	return;
}

int add_explorer_list(GtkListStore *store,char *str)
{
    GtkTreeIter iter;
	
    gtk_list_store_append(store,&iter);
	EnterCriticalSection(&CRI_ATU);
    gtk_list_store_set(store,&iter,0,ANSIToUTF8(str),-1);
	LeaveCriticalSection(&CRI_ATU);
	
    return 0;
}

int remove_explorer_item(GtkWidget *view,GtkListStore *store)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
	
    model=gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    while(gtk_tree_model_iter_nth_child(model,&iter,NULL,0))       //获取对应条目的迭代器
    {
		gtk_list_store_remove(store, &iter);               //删除列表中对应的条目
	}
	
    return 0;
}

void destroy_explorer(GtkWidget *widget,gpointer Parameter)
{
	Explorer_Struct *ES=(Explorer_Struct *)Parameter;
	EnterCriticalSection(&ES->cs);
	closesocket(ES->soc);
	ES->exit_flag=1;
	LeaveCriticalSection(&ES->cs);
	return;
}

void file_explorer_window()
{
	GtkWidget *explorer_window;
	GtkWidget *explorer_hbox,*explorer_vbox;
	GtkWidget *drives_scrolled_window;
	GtkWidget *folders_scrolled_window;
	GtkWidget *files_scrolled_window;
	GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
	GtkTreeModel *model;
    GtkTreeIter iter;
	int index,hostID;
	char strbuffer[MAX_SIZE],*hostIP,*hostname;
	ControlClient temp;
	Explorer_Struct *es=NULL;

	memset(strbuffer,NULL,sizeof(strbuffer));
	memset(&temp,NULL,sizeof(ControlClient));

	if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model,&iter))
        return;

	if((es=(Explorer_Struct *)malloc(sizeof(Explorer_Struct)))==NULL)
	{
		MessageBox(NULL,"malloc error!","Error",MB_OK);
		exit(-1);
	}
	memset(es,NULL,sizeof(Explorer_Struct));
	
	gtk_tree_model_get(model,&iter,HostID_Column,&hostID,-1);
    gtk_tree_model_get(model,&iter,HostName_Column,&hostname,-1);
    gtk_tree_model_get(model,&iter,HostIP_Column,&hostIP,-1);
    sprintf(strbuffer,"File Explorer (%s/%s)",hostname,hostIP);
	explorer_window=create_window(strbuffer,10,770,400);
    g_free(hostIP);
    g_free(hostname);

	
	explorer_hbox=gtk_hbox_new(FALSE,1);
	explorer_vbox=gtk_vbox_new(FALSE,1);

	es->path_label=gtk_label_new_with_mnemonic("");
	
	drives_scrolled_window=gtk_scrolled_window_new(NULL,NULL);
	folders_scrolled_window=gtk_scrolled_window_new(NULL,NULL);
	files_scrolled_window=gtk_scrolled_window_new(NULL,NULL);
	
	es->drives_list_store=gtk_list_store_new(1,G_TYPE_STRING);
	es->folders_list_store=gtk_list_store_new(1,G_TYPE_STRING);
	es->files_list_store=gtk_list_store_new(1,G_TYPE_STRING);
	
	es->drives_tree_view=gtk_tree_view_new();
	es->folders_tree_view=gtk_tree_view_new();
	es->files_tree_view=gtk_tree_view_new();
	
	gtk_widget_set_size_request(GTK_WIDGET(drives_scrolled_window),50,350);
	gtk_widget_set_size_request(GTK_WIDGET(folders_scrolled_window),260,350);
	gtk_widget_set_size_request(GTK_WIDGET(files_scrolled_window),260,350);
	//设置滚动条出现的方式
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(drives_scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(folders_scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(files_scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(es->drives_tree_view),GTK_TREE_MODEL(es->drives_list_store));
	gtk_tree_view_set_model(GTK_TREE_VIEW(es->folders_tree_view),GTK_TREE_MODEL(es->folders_list_store));
	gtk_tree_view_set_model(GTK_TREE_VIEW(es->files_tree_view),GTK_TREE_MODEL(es->files_list_store));
	//将列表构件放到滚动构件里
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(drives_scrolled_window),es->drives_tree_view);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(folders_scrolled_window),es->folders_tree_view);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(files_scrolled_window),es->files_tree_view);
	
	renderer=gtk_cell_renderer_text_new();
	EnterCriticalSection(&CRI_ATU);
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Drives"),renderer,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(es->drives_tree_view),GTK_TREE_VIEW_COLUMN(column));
	
	renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Folders"),renderer,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(es->folders_tree_view),GTK_TREE_VIEW_COLUMN(column));
	
	renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes(ANSIToUTF8("Files"),renderer,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(es->files_tree_view),GTK_TREE_VIEW_COLUMN(column));
	LeaveCriticalSection(&CRI_ATU);

	gtk_box_pack_start(GTK_BOX(explorer_hbox),drives_scrolled_window,FALSE,FALSE,3);
	gtk_box_pack_start(GTK_BOX(explorer_hbox),folders_scrolled_window,TRUE,TRUE,3);
	gtk_box_pack_start(GTK_BOX(explorer_hbox),files_scrolled_window,TRUE,TRUE,3);

	gtk_box_pack_start(GTK_BOX(explorer_vbox),es->path_label,TRUE,TRUE,1);
	gtk_box_pack_start(GTK_BOX(explorer_vbox),explorer_hbox,TRUE,TRUE,2);

	temp.soc=GetClientSocket(hostID);
    temp.order=5;
    index=GetIndexBySocket(temp.soc);
    temp.HTTP_Proxy=IODataArray[index]->Client.HTTP_Proxy;
    CC=temp;
	es->ClientSocket=temp.soc;
	es->index=index;

	//添加选择信号
	es->drive_selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(es->drives_tree_view));
	es->folder_selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(es->folders_tree_view));
	es->file_selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(es->files_tree_view));
	InitializeCriticalSection(&es->cs);   //初始化临界区

	//添加鼠标双击信号
    g_signal_connect(G_OBJECT(es->drives_tree_view),"row-activated",G_CALLBACK(file_explorer_tree_selection),es);
    g_signal_connect(G_OBJECT(es->folders_tree_view),"row-activated",G_CALLBACK(file_explorer_tree_selection),es);
	
	gtk_container_add(GTK_CONTAINER(explorer_window),explorer_vbox);
	
	g_signal_connect(G_OBJECT(es->files_tree_view),"button-release-event",G_CALLBACK(pop_flie_explorer_menu),es);
	g_signal_connect(G_OBJECT(explorer_window),"destroy",G_CALLBACK(destroy_explorer),es);

	g_thread_create((GThreadFunc)file_explorer,es,TRUE,NULL);

	gtk_widget_show_all(explorer_window);

	return;
}

GtkWidget *create_choose_button()
{
    GtkWidget *frame;
    GtkWidget *button_box;
    GtkWidget *button;
	TranStructParameter *TSP;

	TSP=(TranStructParameter *)malloc(sizeof(TranStructParameter));
	if(TSP==NULL)
	{
		MessageBox(NULL,"malloc error","Error",MB_OK);
		exit(-1);
	}
	memset(TSP,NULL,sizeof(TranStructParameter));

	TSP->ClientSocket=INVALID_SOCKET;
	TSP->flag=1;
	TSP->selection=selection;

    button_box=gtk_hbutton_box_new();
	EnterCriticalSection(&CRI_ATU);
    frame=gtk_frame_new(ANSIToUTF8("Control Options"));

    gtk_container_set_border_width(GTK_CONTAINER(button_box),10);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box),GTK_BUTTONBOX_SPREAD);    //设置子控件排列方式
    gtk_box_set_spacing(GTK_BOX(button_box),10);        //设置子控件之间的间隙
    gtk_container_add(GTK_CONTAINER(frame),button_box);

    button=gtk_button_new_with_label(ANSIToUTF8("Command"));
    gtk_container_add(GTK_CONTAINER(button_box),button);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(shell_window),selection);

	button=gtk_button_new_with_label(ANSIToUTF8("File Explorer"));
	gtk_container_add(GTK_CONTAINER(button_box),button);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(file_explorer_window),NULL);

    button=gtk_button_new_with_label(ANSIToUTF8("Upload"));
    gtk_container_add(GTK_CONTAINER(button_box),button);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(upload_window),TSP);

    button=gtk_button_new_with_label(ANSIToUTF8("Download"));
    gtk_container_add(GTK_CONTAINER(button_box),button);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(download_window),TSP);

    button=gtk_button_new_with_label(ANSIToUTF8("Disconnect"));
    gtk_container_add(GTK_CONTAINER(button_box),button);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(disconnect),selection);
	LeaveCriticalSection(&CRI_ATU);

    return frame;
}

void destroy_shell(GtkWidget *widget,gpointer pss)
{
    shell_struct *ss=(shell_struct *)pss;
	closesocket(ss->soc);
	gtk_widget_destroy(widget);
    return;
}

int insert_text(shell_struct ss,char *_text)
{

    GtkTextIter iter;
    GtkTextBuffer *buffer;
    char text[MAX_SIZE*2];

    memset(text,NULL,sizeof(text));
	EnterCriticalSection(&CRI_ATU);
    strcat(text,ANSIToUTF8(_text));
	LeaveCriticalSection(&CRI_ATU);

    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ss.text_view));   //获取对应的buffer
    gtk_text_buffer_get_end_iter(buffer,&iter);
    gtk_text_buffer_insert(buffer,&iter,text,strlen(text));         //在结束点处追加字符

    gtk_text_buffer_get_end_iter(buffer,&iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(ss.text_view),&iter,0.0,FALSE,0.0,0.0);
    while (gtk_events_pending ()) gtk_main_iteration();

    return 0;
}

void shell_window(GtkWidget *widget,gpointer selection)
{
    GtkWidget *shell_window,*results_scrolled,*button;
    GtkWidget *entry,*results_text_view,*shell_vbox,*entry_hbox;
    GtkTreeModel *model;
    GtkTreeIter iter;
	
    char strbuffer[MAX_SIZE],*hostIP,*hostname;
    int hostID,index;
    ControlClient temp;
    shell_struct *ss;

	if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model,&iter))
        return;

    if((ss=(shell_struct *)malloc(sizeof(shell_struct)))==NULL)
	{
		puts("malloc error");
		exit(-1);
	}
    memset(strbuffer,NULL,sizeof(strbuffer));
    memset(&temp,NULL,sizeof(ControlClient));
    memset(ss,NULL,sizeof(shell_struct));
	
    gtk_tree_model_get(model,&iter,HostID_Column,&hostID,-1);
    gtk_tree_model_get(model,&iter,HostName_Column,&hostname,-1);
    gtk_tree_model_get(model,&iter,HostIP_Column,&hostIP,-1);
    sprintf(strbuffer,"CMD_SHELL (%s/%s)",hostname,hostIP);
    shell_window=create_window(strbuffer,2,700,430);
    g_free(hostIP);
    g_free(hostname);
	
    results_text_view=gtk_text_view_new();              //新建一个文本区域
    results_scrolled=gtk_scrolled_window_new(NULL,NULL);
    shell_vbox=gtk_vbox_new(FALSE,5);
    entry_hbox=gtk_hbox_new(FALSE,5);
    entry=gtk_entry_new();
	EnterCriticalSection(&CRI_ATU);
    button=gtk_button_new_with_label(ANSIToUTF8("Exec"));
	LeaveCriticalSection(&CRI_ATU);
    gtk_widget_set_size_request(GTK_WIDGET(results_scrolled),690,360);
    gtk_widget_set_size_request(GTK_WIDGET(entry),620,25);
	
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(results_scrolled),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(results_scrolled),results_text_view);    //将文本区域加入到滚动窗口中
	
    gtk_box_pack_start(GTK_BOX(entry_hbox),entry,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(entry_hbox),button,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(shell_vbox),results_scrolled,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(shell_vbox),entry_hbox,FALSE,FALSE,5);
	
    gtk_container_add(GTK_CONTAINER(shell_window),shell_vbox);
	
    temp.soc=GetClientSocket(hostID);
    temp.order=1;
    index=GetIndexBySocket(temp.soc);
    temp.HTTP_Proxy=IODataArray[index]->Client.HTTP_Proxy;
    CC=temp;
	
    ss->index=index;
    ss->text_view=results_text_view;
    ss->entry=entry;
	
	g_signal_connect(G_OBJECT(shell_window),"destroy",G_CALLBACK(destroy_shell),ss);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(Send_Command),ss);
	//g_signal_connect(G_OBJECT(entry),"key-press-event",G_CALLBACK(listen_keyboard),ss);
	g_signal_connect(G_OBJECT(entry),"activate",G_CALLBACK(Send_Command),ss);
	
    /*创建线程*/ 
	g_thread_create((GThreadFunc)Execute_Command,ss,TRUE,NULL);
	
    gtk_widget_show_all(shell_window);
}

void update_progressbar(GtkWidget *pbar,unsigned long total,unsigned long acc,int flag)
{
	//第三个参数,1:上传文件,2:下载文件
	gdouble fraction;
	char text[MAX_SIZE];

	memset(text,NULL,sizeof(text));

	fraction=((gdouble)acc)/((gdouble)total);
	if(flag==1)
		sprintf(text,"%0.2f%% (%ld/%ld)",fraction*100,acc,total);
	else
		sprintf(text,"%0.2f%% (%ld/%ld)",fraction*100,acc,total);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar),fraction);
	EnterCriticalSection(&CRI_ATU);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar),ANSIToUTF8(text));
	LeaveCriticalSection(&CRI_ATU);
	return;
}

void achieve_entry(GtkWidget *widget,TranStruct *ts)
{
	g_signal_handler_disconnect(ts->button,ts->handler);      //解除button绑定的信号
	char *strbuffer=NULL;
	ControlClient temp;
	int index;

	memset(&temp,NULL,sizeof(ControlClient));

	index=GetIndexBySocket(ts->soc);
	temp.soc=ts->soc;
	temp.HTTP_Proxy=IODataArray[index]->Client.HTTP_Proxy;
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(ts->local_path_entry));
	EnterCriticalSection(&CRI_UTA);
	strcat(temp.localpath,UTF8ToANSI(strbuffer));
	strbuffer=(char *)gtk_entry_get_text(GTK_ENTRY(ts->remote_path_entry));
	strcat(temp.remotepath,UTF8ToANSI(strbuffer));
	LeaveCriticalSection(&CRI_UTA);
	temp.order=ts->order;
	temp.pbar=ts->pbar;
	CC=temp;
	return;
}

void destroy_upload_window_TS(GtkWidget *widget,gpointer ts)
{
	free(ts);
	return;
}

void upload_window(GtkWidget *widget,gpointer Parameter)
{
	TranStructParameter *TSP=(TranStructParameter *)Parameter;
    GtkWidget *upload_window;
	GtkWidget *vbox;
	GtkWidget *label;
    GtkTreeModel *model;
    GtkTreeIter iter;
	int hostID;
    char strbuffer[MAX_SIZE],*hostIP=NULL,*hostname=NULL;
	TranStruct *TS=NULL;

    memset(strbuffer,NULL,sizeof(strbuffer));

	if((TS=(TranStruct *)malloc(sizeof(TranStruct)))==NULL)
	{
		MessageBox(NULL,"malloc error","Error",MB_OK);
		exit(-1);
	}
	memset(TS,NULL,sizeof(TranStruct));

	if(TSP->flag==1)
    {
		if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(TSP->selection),&model,&iter))
            return;
		gtk_tree_model_get(model,&iter,HostID_Column,&hostID,-1);
		gtk_tree_model_get(model,&iter,HostName_Column,&hostname,-1);
        gtk_tree_model_get(model,&iter,HostIP_Column,&hostIP,-1);
		TS->soc=GetClientSocket(hostID);
	}
	else if(TSP->flag==2)
	{
		TS->soc=TSP->ClientSocket;
		hostID=GetIndexBySocket(TSP->ClientSocket);
		hostname=(char *)malloc(strlen(IODataArray[hostID]->Client.name));
		hostIP=(char *)malloc(strlen(IODataArray[hostID]->Client.ip));
		if(hostname==NULL || hostIP==NULL)
		{
			MessageBox(NULL,"malloc error","Error",MB_OK);
			exit(-1);
		}
		memset(hostname,NULL,strlen(IODataArray[hostID]->Client.name));
		memset(hostIP,NULL,strlen(IODataArray[hostID]->Client.ip));
		strcat(hostname,IODataArray[hostID]->Client.name);
		strcat(hostIP,IODataArray[hostID]->Client.ip);
	}
	else
		return;

    sprintf(strbuffer,"Upload File (%s/%s)",hostname,hostIP);
    upload_window=create_window(strbuffer,10,500,200);
	if(TSP->flag==1)
    {
		g_free(hostIP);
        g_free(hostname);
	}
	else
	{
		free(hostIP);
		free(hostname);
	}

	TS->order=2;         //上传文件命令

	vbox=gtk_vbox_new(FALSE,5);
	TS->pbar=gtk_progress_bar_new();
	gtk_widget_set_size_request(GTK_WIDGET(TS->pbar),410,40);
	TS->local_path_entry=gtk_entry_new();
	TS->remote_path_entry=gtk_entry_new();
	EnterCriticalSection(&CRI_ATU);
	if(TSP->flag==1)
	    TS->button=gtk_button_new_with_label(ANSIToUTF8("Start"));

	if(TSP->flag==2)
	{
		gtk_entry_set_text(GTK_ENTRY(TS->local_path_entry),ANSIToUTF8(TSP->local_path));
	    gtk_entry_set_text(GTK_ENTRY(TS->remote_path_entry),ANSIToUTF8(TSP->remote_path));
	}

	gtk_box_pack_start(GTK_BOX(vbox),TS->pbar,FALSE,FALSE,5);
	label=gtk_label_new(ANSIToUTF8("Local Path:"));
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);      //设置标签为左对齐
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,1);
	gtk_box_pack_start(GTK_BOX(vbox),TS->local_path_entry,FALSE,FALSE,1);
	label=gtk_label_new(ANSIToUTF8("Remote Path:"));
	LeaveCriticalSection(&CRI_ATU);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,1);
	gtk_box_pack_start(GTK_BOX(vbox),TS->remote_path_entry,FALSE,FALSE,1);
	if(TSP->flag==1)
	    gtk_box_pack_start(GTK_BOX(vbox),TS->button,FALSE,FALSE,5);
	gtk_container_add(GTK_CONTAINER(upload_window),vbox);

	if(TSP->flag==1)
	    TS->handler=g_signal_connect(G_OBJECT(TS->button),"clicked",G_CALLBACK(achieve_entry),TS);
	else
		achieve_entry(NULL,TS);
	g_signal_connect(G_OBJECT(upload_window),"destroy",G_CALLBACK(destroy_upload_window_TS),TS);

    gtk_widget_show_all(upload_window);

    return;
}

void destroy_dowmload_window(GtkWidget *widget,gpointer ts)
{
	free(ts);
	return;
}

void download_window(GtkWidget *widget,gpointer Parameter)
{
	TranStructParameter *TSP=(TranStructParameter *)Parameter;
    GtkWidget *upload_window;
	GtkWidget *vbox;
	GtkWidget *label;
    GtkTreeModel *model;
    GtkTreeIter iter;
	int hostID;
    char strbuffer[MAX_SIZE],*hostIP,*hostname;
	TranStruct *TS=NULL;

    memset(strbuffer,NULL,sizeof(strbuffer));

	if((TS=(TranStruct *)malloc(sizeof(TranStruct)))==NULL)
	{
		printf("malloc error\n");
		exit(-1);
	}
	memset(TS,NULL,sizeof(TranStruct));

	if(TSP->flag==1)
    {
		if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(TSP->selection),&model,&iter))
		{
			free(TS);
			return;
		}
		gtk_tree_model_get(model,&iter,HostID_Column,&hostID,-1);
		gtk_tree_model_get(model,&iter,HostName_Column,&hostname,-1);
        gtk_tree_model_get(model,&iter,HostIP_Column,&hostIP,-1);
		TS->soc=GetClientSocket(hostID);
	}
	else if(TSP->flag==2)
	{
		TS->soc=TSP->ClientSocket;
		hostID=GetIndexBySocket(TSP->ClientSocket);
		hostname=(char *)malloc(strlen(IODataArray[hostID]->Client.name));
		hostIP=(char *)malloc(strlen(IODataArray[hostID]->Client.ip));
		if(hostname==NULL || hostIP==NULL)
		{
			MessageBox(NULL,"malloc error","Error",MB_OK);
			exit(-1);
		}
		memset(hostname,NULL,strlen(IODataArray[hostID]->Client.name));
		memset(hostIP,NULL,strlen(IODataArray[hostID]->Client.ip));
		strcat(hostname,IODataArray[hostID]->Client.name);
		strcat(hostIP,IODataArray[hostID]->Client.ip);
	}
	else
	{
		free(TS);
		return;
	}

    sprintf(strbuffer,"Download File (%s/%s)",hostname,hostIP);
    upload_window=create_window(strbuffer,10,500,200);
	if(TSP->flag==1)
    {
		g_free(hostIP);
        g_free(hostname);
	}
	else
	{
		free(hostIP);
		free(hostname);
	}

	TS->order=3;         //下载文件命令
	
	vbox=gtk_vbox_new(FALSE,5);
	TS->pbar=gtk_progress_bar_new();
	gtk_widget_set_size_request(GTK_WIDGET(TS->pbar),410,40);
	TS->local_path_entry=gtk_entry_new();
	TS->remote_path_entry=gtk_entry_new();
	EnterCriticalSection(&CRI_ATU);
	if(TSP->flag==1)
	    TS->button=gtk_button_new_with_label(ANSIToUTF8("Start"));

	if(TSP->flag==2)
	{
		gtk_entry_set_text(GTK_ENTRY(TS->local_path_entry),ANSIToUTF8(TSP->local_path));
		gtk_entry_set_text(GTK_ENTRY(TS->remote_path_entry),ANSIToUTF8(TSP->remote_path));
	}
	
	gtk_box_pack_start(GTK_BOX(vbox),TS->pbar,FALSE,FALSE,5);
	label=gtk_label_new(ANSIToUTF8("Remote Path:"));
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,1);
	gtk_box_pack_start(GTK_BOX(vbox),TS->remote_path_entry,FALSE,FALSE,1);
	label=gtk_label_new(ANSIToUTF8("Local Path:"));
	LeaveCriticalSection(&CRI_ATU);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);      //设置标签为左对齐
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,1);
	gtk_box_pack_start(GTK_BOX(vbox),TS->local_path_entry,FALSE,FALSE,1);
	if(TSP->flag==1)
	    gtk_box_pack_start(GTK_BOX(vbox),TS->button,FALSE,FALSE,5);
	gtk_container_add(GTK_CONTAINER(upload_window),vbox);
	
	if(TSP->flag==1)
	    TS->handler=g_signal_connect(G_OBJECT(TS->button),"clicked",G_CALLBACK(achieve_entry),TS);
	else
		achieve_entry(NULL,TS);
	g_signal_connect(G_OBJECT(upload_window),"destroy",G_CALLBACK(destroy_dowmload_window),TS);
	
    gtk_widget_show_all(upload_window);
	
    return;
}

void disconnect(GtkWidget *widget,gpointer selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	ControlClient temp;
	int hostID,index;

	memset(&temp,NULL,sizeof(ControlClient));

	if(!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model,&iter))
        return;
	gtk_tree_model_get(model,&iter,HostID_Column,&hostID,-1);
// 	gdk_threads_enter();
// 	remove_item(hostID);
// 	gdk_threads_leave();
	temp.soc=GetClientSocket(hostID);
	index=GetIndexBySocket(temp.soc);
	temp.HTTP_Proxy=IODataArray[index]->Client.HTTP_Proxy;
	temp.order=4;
	CC=temp;
	return;
}

DWORD WINAPI _MessageBox(LPVOID Parameter)
{
	MessageStruct MS=*(MessageStruct *)Parameter;
	MessageBox(NULL,MS.content,MS.title,MB_OK);
	return 0;
}