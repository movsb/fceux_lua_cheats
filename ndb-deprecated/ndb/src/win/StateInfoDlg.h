#ifndef __STATE_INFO_DLG_H__
#define __STATE_INFO_DLG_H__

#include "WindowBase.h"
#include "../tables/state.h"

class AStateInfoDlg:public AWindowBase
{
public:
	AStateInfoDlg(AWindowBase* parent/*,ARomMap* rom_map*/);
	~AStateInfoDlg();

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);


public:
	void ShowStateInfo(AState* state,list_s* ls);
	AStateMap* GetScriptIndexof(int index);

private:
	AState*		m_state;
	list_s*		m_ls;

};

#endif//!__STATE_INFO_DLG_H__
