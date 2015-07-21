#include <iostream>
#include "apihook.h"

using namespace std;

APIHook *pHook=NULL;

int test(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType)
{
    cout <<lpText<<endl;
    pHook->UnHook();
    MessageBoxA(hWnd,lpText,lpCaption,uType);

    return 0;
}

int main()
{
    pHook=new APIHook();
    pHook->SetHookAPI("User32.dll","MessageBoxA",(PROC)test);
    pHook->Hook();

    Sleep(3000);
    //pHook->UnHook();

    MessageBoxA(NULL,"123123","123123",MB_OK);
    MessageBoxA(NULL,"1111111","11111111",MB_OK);

    return 0;
}
