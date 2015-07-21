#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

int func(char *buffer)
{
    int auth=0;
    char str[10];

    memset(str,NULL,sizeof(str));
    memcpy(str,buffer,1000);

    if(auth!=0)
        printf("overflow!\n");
    else
        printf("normal\n");

    getch();

    return 0;
}

int main()
{
    printf("fuck");
    char str[1000];
    FILE *file=NULL;

    memset(str,NULL,sizeof(str));

    file=fopen("1.txt","rb");
    if(file==NULL)
    {
        printf("文件打开失败\n");
        getch();
        return -1;
    }
    fread(str,1,sizeof(str),file);
    func(str);

    return 0;
}






