#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

using namespace std;

int traversal_directory(char *_path)
{
    char path[5000];
    DIR *pDir=NULL;
    struct dirent *pStcDir=NULL;

    if((pDir=opendir(_path))==NULL)
        return -1;
    while(pStcDir=readdir(pDir))
    {
        if(!strcmp(pStcDir->d_name,".") || !strcmp(pStcDir->d_name,".."))
            continue;
        cout <<pStcDir->d_name<<endl;
        if(pStcDir->d_type==DT_DIR)
        {
            memset(path,NULL,sizeof(path));
            sprintf(path,"%s/%s",_path,pStcDir->d_name);
            traversal_directory(path);
        }
    }

    return 0;
}

int main()
{
    traversal_directory("/home");
    return 0;
}
