#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <Tlhelp32.h>

using namespace std;

class Enum_Process
{
public:
    int Enum_Pro();
};

int Enum_Process::Enum_Pro()
{
    PROCESSENTRY32 Pe32;
    HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);

    memset(&Pe32,NULL,sizeof(Pe32));
    Pe32.dwSize=sizeof(Pe32);
    if(Process32First(hSnap,&Pe32)!=true)
    {
        cout<<"Error!\n错误代码:"<<GetLastError();
        system("pause");
        return 0;
    }
	cout<<"映像名称\t\t\t进程ID"<<endl;
    do
    {
        cout<<Pe32.szExeFile<<"\t\t"<<Pe32.th32ProcessID<<endl;
    }
    while(Process32Next(hSnap,&Pe32)==true);
    return 1;
}

int main(int argc,char *argv[])
{
    class Enum_Process func;
    func.Enum_Pro();
	system("pause");
    return 0;
}





