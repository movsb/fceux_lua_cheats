#ifndef __CHEAT_FILE_INFO_DLG_H__
#define __CHEAT_FILE_INFO_DLG_H__

#include "ctl/WindowBase.h"
#include "../tables/cheat.h"

class ACheatFileInfoDlg:public AWindowBase
{
public:
	enum{RET_CLOSE,RET_OK,RET_CANCEL,};
	ACheatFileInfoDlg(AWindowBase* parent,ACheatFile* pFile);
	~ACheatFileInfoDlg();
	ACheatFile* GetFile() const{return m_pFile;}

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnClose();
	INT_PTR OnSize(int width,int height){m_layout.SizeItems();return 0;}


private:
	ACheatFile* m_pFile;
	AEditBox*	m_editName;
	AEditBox*	m_editAuthor;
	AEditBox*	m_editTime;
	AEditBox*	m_editDesc;
	CLayout		m_layout;
};

#endif//!__CHEAT_FILE_INFO_DLG_H__
