#ifndef __CHEAT_CUSTOM_DLG_H__
#define __CHEAT_CUSTOM_DLG_H__

#include "ctl/WindowBase.h"
#include <SdkLayout.h>
#include <string>
using namespace std;


typedef struct lua_State lua_State;

class ACheatCustomDlg:public AWindowBase
{
public:
	ACheatCustomDlg(AWindowBase* parent,HWND* phModal,lua_State* L);
	~ACheatCustomDlg();

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnClose();
	INT_PTR OnSize(int width,int height);
	INT_PTR OnVScroll(WPARAM wParam,HWND hScrollNull){m_layout.ProcessScrollMessage(WM_VSCROLL,wParam, m_lParam);return 0;}
	INT_PTR OnHScroll(WPARAM wParam, HWND hScrollNUll){m_layout.ProcessScrollMessage(WM_HSCROLL,wParam, m_lParam);return 0;}
public:
	int GetValue(){return m_Value;}

private:
	void InitializeFromTable();
	bool CheckValid(int n);
	bool LuaOnOk(int val);

private:
	SdkLayout::CSdkLayout m_layout;
	HWND*		m_phModal;
	lua_State*	m_Lua;
	int			m_Value;
	HWND		m_Static;
	AEditBox*	m_editCustom;

	int			m_minValue;		//我不知道是否需要负数, 我假设不需要,并把负数作为无效的标志
	int			m_maxValue;
};

#endif//!__CHEAT_CUSTOM_DLG_H__
