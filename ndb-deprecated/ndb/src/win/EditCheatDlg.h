#ifndef __EDIT_CHEAT_DLG_H__
#define __EDIT_CHEAT_DLG_H__

#include "WindowBase.h"

class AEditCheatDlg:public AWindowBase
{
public:
	enum{RET_OK,RET_CANCEL,RET_CLOSE};
public:
	AEditCheatDlg(AWindowBase* parent,char* desc=0,char* lua=0);
	~AEditCheatDlg();

	std::string& GetName(){return m_Name;}
	std::string& GetScript(){return m_Script;}

private:
	std::string m_Name;
	std::string m_Script;

public:
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnClose();
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);

private:
	AEditBox* m_editDesc;
	AEditBox* m_editScript;

private:
	char* m_desc;
	char* m_lua;
};


#endif//!__EDIT_CHEAT_DLG_H__
