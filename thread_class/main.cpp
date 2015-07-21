#include <iostream>
#include "thread.h"

using namespace std;

class test:public Thread
{
public:
    int func(void *threadPara);
};

int test::func(void *threadPara)
{
    while(1)
    {
        cout << *(int *)threadPara<<endl;
        Sleep(1000);
    }
}

int main()
{
    int i=1,j=2;
    test test1,test2;

    test1.run((void *)&i);
    test2.run((void *)&j);

    Sleep(100000);

    return 0;
}






