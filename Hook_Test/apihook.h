#ifndef APIHOOK_H_INCLUDED
#define APIHOOK_H_INCLUDED

#include <windows.h>

class APIHook
{
public:
    APIHook();
    ~APIHook();

    //Set API Info
    int SetHookAPI(LPSTR _pModuleName,LPSTR _pAPIName,PROC _func);
    //HOOK API
    int Hook();
    //Clear HOOK
    int UnHook();

private:
    LPSTR pModuleName;      //API所在链接库名称
    LPSTR pAPIName;         //API名称
    PROC pAPIAddress;       //API地址
    PROC func;              //回调的函数
    BYTE oldByteData[5];    //原函数入口代码
    BYTE newByteData[5];    //更改后的函数入口代码
};

#endif // APIHOOK_H_INCLUDED
