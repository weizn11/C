#ifndef _UNICODE_H_
#define _UNICODE_H_

#include <stdio.h>
#include <windows.h>
#include <locale.h>

char *TCHARToChar(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen(lpwszStrIn);
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		pszOut = new char[nOutputStrLen];

		if (pszOut)
		{
			memset(pszOut, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}
	return pszOut;
}

TCHAR *CharToTCHAR(const char *str)
{
	TCHAR T[100];
	memset(T, NULL, sizeof(T));
	MultiByteToWideChar(CP_ACP, 0, str, -1, T, sizeof(T));
	return T;
}

#endif     //_UNICODE_H_
