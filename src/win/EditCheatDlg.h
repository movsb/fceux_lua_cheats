#ifndef __EDIT_CHEAT_DLG_H__
#define __EDIT_CHEAT_DLG_H__

#include "ctl/WindowBase.h"

class AEditCheatDlg:public AWindowBase
{
public:
	//如果主窗口被被动Destroy, 那么宿主窗口会先被Destroy,然后才是此窗口
	//此时如果依然返回OK的话, 就错了, 因为主窗口的list已经被移除了
	enum{RET_CLOSE,RET_OK,RET_CANCEL,};
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
	INT_PTR OnSize(int width,int height){m_layout.SizeItems();return 0;}
	INT_PTR OnGetDlgCode(WPARAM vk,MSG* msg);

private:
	AEditBox* m_editDesc;
	AEditBox* m_editScript;

	CLayout m_layout;
	HFONT m_hFont;

private:
	char* m_desc;
	char* m_lua;
};


#endif//!__EDIT_CHEAT_DLG_H__
