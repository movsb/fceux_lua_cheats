#include <string>
#include <windows.h>
#include <cstring>

using namespace std;

#include "App.h"

string __cdecl AApp::error(const char* fmt,...)
{
	char str[2048]={0};
	va_list va;
	va_start(va,fmt);
	_vsnprintf(str,sizeof(str),fmt,va);
	return str;
}
