#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <stdio.h>
#include <windows.h>
#include <locale.h>

wchar_t * ANSIToUnicode( const char* str )
{
    int textlen ;
    static wchar_t * result=NULL;
    textlen = MultiByteToWideChar( CP_ACP, 0, str,-1, NULL,0 );
    if(result==NULL)
        result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
    else
        result = (wchar_t *)realloc(result,(textlen+1)*sizeof(wchar_t));
    memset(result,0,(textlen+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0,str,-1,(LPWSTR)result,textlen );
    return result;
}

char * UnicodeToANSI( const wchar_t* str )
{
    static char* result=NULL;
    int textlen;
    textlen = WideCharToMultiByte( CP_ACP, 0, str, -1, NULL, 0, NULL, NULL );
    if(result==NULL)
        result = (char *)malloc((textlen+1)*sizeof(wchar_t));
    else
        result = (char *)realloc(result,(textlen+1)*sizeof(wchar_t));
    memset( result, 0, sizeof(char) * ( textlen + 1 ) );
    WideCharToMultiByte( CP_ACP, 0, str, -1, result, textlen, NULL, NULL );
    return result;
}

wchar_t * UTF8ToUnicode( const char* str )
{
    int textlen ;
    static wchar_t * result=NULL;
    textlen = MultiByteToWideChar( CP_UTF8, 0, str,-1, NULL,0 );
    if(result==NULL)
        result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
    else
        result = (wchar_t *)realloc(result,(textlen+1)*sizeof(wchar_t));
    memset(result,0,(textlen+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0,str,-1,(LPWSTR)result,textlen );
    return result;
}

char * UnicodeToUTF8( const wchar_t* str )
{
    static char* result=NULL;
    int textlen;
    textlen = WideCharToMultiByte( CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL );
    if(result==NULL)
        result = (char *)malloc((textlen+1)*sizeof(wchar_t));
    else
        result = (char *)realloc(result,(textlen+1)*sizeof(wchar_t));
    memset(result, 0, sizeof(char) * ( textlen + 1 ) );
    WideCharToMultiByte( CP_UTF8, 0, str, -1, result, textlen, NULL, NULL );
    return result;
}


char* ANSIToUTF8(const char* str)
{
    return UnicodeToUTF8(ANSIToUnicode(str));
}
char* UTF8ToANSI(const char* str)
{
    return UnicodeToANSI(UTF8ToUnicode(str));
}

#endif    //_ENCODING_H_