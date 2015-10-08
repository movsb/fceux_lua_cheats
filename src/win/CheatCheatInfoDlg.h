#ifndef __CHEAT_CHEAT_INFO_DLG_H__
#define __CHEAT_CHEAT_INFO_DLG_H__

#include "ctl/WindowBase.h"
#include "../tables/cheat.h"
#include <SdkLayout.h>

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
	INT_PTR OnSize(int width,int height);
	INT_PTR OnVScroll(WPARAM wParam,HWND hScrollNull){m_layout.ProcessScrollMessage(WM_VSCROLL,wParam, m_lParam);return 0;}
	INT_PTR OnHScroll(WPARAM wParam, HWND hScrollNUll){m_layout.ProcessScrollMessage(WM_HSCROLL,wParam, m_lParam);return 0;}

private:
	SdkLayout::CSdkLayout m_layout;

	bool		m_bCurrentDesc;
	string		m_desc;
	string		m_custom;

	ACheatEntry* m_pEntry;
	AEditBox*	m_editName;
	AEditBox*	m_editEdit;

};

#endif//!__CHEAT_CHEAT_INFO_DLG_H__
