#ifndef __CHEAT_CUSTOM_DLG_H__
#define __CHEAT_CUSTOM_DLG_H__

#include "WindowBase.h"
#include <string>
using namespace std;
//#include "../tables/cheat.h"

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

public:
	int GetValue(){return m_Value;}

private:
	void InitializeFromTable();
	bool CheckValid(int n);
	bool LuaOnOk(int val);

private:
	HWND*		m_phModal;
	lua_State*	m_Lua;
	int			m_Value;
	HWND		m_Static;
	AEditBox*	m_editCustom;

	int			m_minValue;		//�Ҳ�֪���Ƿ���Ҫ����, �Ҽ��費��Ҫ,���Ѹ�����Ϊ��Ч�ı�־
	int			m_maxValue;
};

#endif//!__CHEAT_CUSTOM_DLG_H__
