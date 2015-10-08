#ifndef __APP_H__
#define __APP_H__

#include <windows.h>
#include <string>

using namespace std;

class AApp
{
public:
	AApp()
	{
		SetAppPath();
		SetAppInstance();
	}
	~AApp();

public:
	const string GetAppPath() const {return m_AppPath;}
	const HINSTANCE GetAppInstance() const {return m_hInst;}
	string __cdecl error(const char* fmt,...);

private:
	void SetAppPath()
	{
		char path[MAX_PATH]={0};
		::GetModuleFileName(0,path,sizeof(path));
		strrchr(path,'\\')[1] = '\0';
		m_AppPath = path;
	}
	void SetAppInstance()
	{
		m_hInst = GetModuleHandle(0);
	}

private:
	string		m_AppPath;
	HINSTANCE	m_hInst;
};

extern AApp* theApp;
#define mkerr(e)	theApp->error##e

#endif//!__APP_H__
