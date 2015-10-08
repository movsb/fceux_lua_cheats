#include <stdio.h>
#include <windows.h>

#include "debug.h"

void DebugOut(char* fmt, ...)
{	
	char buf[10240] = {0};
	va_list va;
	int len;
	DWORD dw;
	va_start(va, fmt);
	len=_vsnprintf(buf,sizeof(buf),fmt,va);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),buf,len,&dw,NULL);
}
