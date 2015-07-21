#include "apihook.h"

APIHook::APIHook()
{
    pModuleName=NULL;
    pAPIName=NULL;
    pAPIAddress=NULL;
    memset(oldByteData,NULL,5);
    memset(newByteData,NULL,5);

    return;
}

APIHook::~APIHook()
{
    if(pModuleName!=NULL)
        delete pModuleName;
    if(pAPIName!=NULL)
        delete pAPIName;
    UnHook();

    return;
}

int APIHook::SetHookAPI(LPSTR _pModuleName,LPSTR _pAPIName,PROC _func)
{
    if(pModuleName!=NULL)
        return -1;
    pModuleName=new char(strlen(_pModuleName)+1);
    pAPIName=new char(strlen(_pAPIName)+1);
    if(pModuleName==NULL || pAPIName==NULL)
        return -2;
    memset(pModuleName,NULL,strlen(_pModuleName)+1);
    memset(pAPIName,NULL,strlen(_pAPIName)+1);

    strcat(pModuleName,_pModuleName);
    strcat(pAPIName,_pAPIName);

    this->func=_func;

    return 0;
}

int APIHook::Hook()
{
    DWORD dwNum=0;

    //获取指定模块中的API地址
    pAPIAddress=(PROC)GetProcAddress(GetModuleHandle(pModuleName),pAPIName);
    if(pAPIAddress==NULL)
        return -1;
    //保存该地址处5个字节的内容
    ReadProcessMemory(GetCurrentProcess(),(LPCVOID)pAPIAddress,oldByteData,5,&dwNum);

    //构造JMP指令
    newByteData[0]=0xE9;  //jmp code
    *(DWORD *)(newByteData+1)=(DWORD)func-(DWORD)pAPIAddress-5;

    //将构造好的JMP地址写入到该处
    WriteProcessMemory(GetCurrentProcess(),(LPVOID)pAPIAddress,newByteData,5,&dwNum);

    return 0;
}

int APIHook::UnHook()
{
    DWORD dwNum=0;

    if(pAPIAddress==NULL)
        return 0;

    WriteProcessMemory(GetCurrentProcess(),(LPVOID)pAPIAddress,oldByteData,5,&dwNum);

    return 0;
}




