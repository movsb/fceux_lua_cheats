#ifndef __CHEAT_FILE_INFO_DLG_H__
#define __CHEAT_FILE_INFO_DLG_H__

#include "WindowBase.h"
#include "../tables/cheat.h"

class ACheatFileInfoDlg:public AWindowBase
{
public:
	enum{RET_OK,RET_CANCEL,RET_CLOSE};
	ACheatFileInfoDlg(AWindowBase* parent,ACheatFile* pFile);
	~ACheatFileInfoDlg();
	ACheatFile* GetFile() const{return m_pFile;}

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnClose();

// private:
// 	string m_name;
// 	string m_author;
// 	string m_time;
// 	string m_md5;
// 	string m_crc32;
// 	string m_desc;

private:
	ACheatFile* m_pFile;
	AEditBox*	m_editName;
	AEditBox*	m_editMd5;
	AEditBox*	m_editCrc32;
	AEditBox*	m_editAuthor;
	AEditBox*	m_editTime;
	AEditBox*	m_editDesc;
	
};

#endif//!__CHEAT_FILE_INFO_DLG_H__
