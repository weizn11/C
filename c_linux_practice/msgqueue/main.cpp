#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int test()
{
    char str[10];
    int i;

    for(i=0;i<1000;i++)
        str[i]=1;
    return 0;
}

int main()
{
    char *str="123\n";
    FILE *file;

    file=fopen("1.txt","wt");

    while(1)
    {
        fwrite(str,sizeof(str),1,file);
        fflush(file);
        sleep(1);
    }


    return 0;
}
