#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

int main()
{
    intptr_t i=-1;
    void *p;
    p=sbrk(0);
    printf("%d\n",i);
    printf("0x%x\n",p);
    p=sbrk(100);
    printf("0x%x\n",p);
    p=sbrk(100);
    printf("0x%x\n",p);
    printf("OK\n");

    return 0;
}
