#include "thread.h"

Thread::Thread()
{
    threadPara=NULL;
    runFlag=0;
}

int Thread::run(void *Parameter)
{
    HANDLE hThread;

    if(runFlag) return -1;
    this->threadPara=Parameter;
    this->runFlag=1;
    hThread=CreateThread(NULL,0,start_thread,(LPVOID)this,0,NULL);

    CloseHandle(hThread);
    return 0;
}

DWORD WINAPI Thread::start_thread(LPVOID Parameter)
{
    Thread *_this=(Thread *)Parameter;

    return _this->func(_this->threadPara);
}
