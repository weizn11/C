#include <stdio.h>
#include <windows.h>
#include <locale.h>

char * UnicodeToANSI( const wchar_t* str )
{
    char* result;
    int textlen;
    textlen = WideCharToMultiByte( CP_ACP, 0, str, -1, NULL, 0, NULL, NULL );
    result =(char *)malloc((textlen+1)*sizeof(char));
    memset( result, 0, sizeof(char) * ( textlen + 1 ) );
    WideCharToMultiByte( CP_ACP, 0, str, -1, result, textlen, NULL, NULL );
    return result;
}

wchar_t * UTF8ToUnicode( const char* str )
{
    int textlen ;
    wchar_t * result;
    textlen = MultiByteToWideChar( CP_UTF8, 0, str,-1, NULL,0 );
    result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
    memset(result,0,(textlen+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0,str,-1,(LPWSTR)result,textlen );
    return result;
}

char* UTF8ToANSI(const char* str)
{
    return UnicodeToANSI(UTF8ToUnicode(str));
}

