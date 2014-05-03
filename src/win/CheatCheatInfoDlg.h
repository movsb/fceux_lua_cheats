#ifndef __CHEAT_CHEAT_INFO_DLG_H__
#define __CHEAT_CHEAT_INFO_DLG_H__

#include "ctl/WindowBase.h"
#include "../tables/cheat.h"

#include <string>
using namespace std;

class ACheatCheatInfoDlg:public AWindowBase
{
public:
	ACheatCheatInfoDlg(AWindowBase* parent,ACheatEntry* pEntry);
	~ACheatCheatInfoDlg();

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnClose();
	INT_PTR OnSize(int width,int height){m_layout.SizeItems();return 0;}


private:
	bool		m_bCurrentDesc;
	string		m_desc;
	string		m_custom;

	ACheatEntry* m_pEntry;
	AEditBox*	m_editName;
	AEditBox*	m_editEdit;

	CLayout		m_layout;
};

#endif//!__CHEAT_CHEAT_INFO_DLG_H__
