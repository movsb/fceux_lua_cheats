#ifndef __ROM_INFO_DLG_H__
#define __ROM_INFO_DLG_H__

#include "WindowBase.h"
#include "../tables/rom.h"

class ARomInfoDlg:public AWindowBase
{
public:
	ARomInfoDlg(AWindowBase* parent/*,ARomMap* rom_map*/);
	~ARomInfoDlg();

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);


public:
	void ShowRomInfo(ARom* rom,ARomMap* rom_map);
	//void MakeSplashFile(unsigned char* data,int cbdata,int type);

private:
	ARomMap*	m_rommap;
	ARom*		m_rom;
};

#endif//!__ROM_INFO_DLG_H__
