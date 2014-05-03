#ifndef __CHEAT_INFO_DLG_H__
#define __CHEAT_INFO_DLG_H__

#include "ctl/WindowBase.h"
#include "../tables/cheat.h"


class ACheatInfoDlg:public AWindowBase
{
public:
	ACheatInfoDlg(AWindowBase* parent/*,ARomMap* rom_map*/);
	~ACheatInfoDlg();

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnNotify(LPNMHDR phdr);
	INT_PTR OnMouseMove(int key,int x,int y);
	INT_PTR OnLButtonUp(int key,int x,int y);
	INT_PTR OnSize(int width,int height){m_layout.SizeItems();UpdateListHeaderSize();return 0;}


public:
	void ShowCheatInfo(ACheatFile* file,ACheatEntry* pEntry);
	void MoveItemToAnotherPlace(int ifrom,int ito);

private:
	void InitControls();
	int AddListItem(const char* pszText,LPARAM param);
	int UpdateListItem(int index,const char* pszText,LPARAM param);
	void UpdateListHeaderSize();

private:
	ACheatEntry* m_pEntry;
	
	//bool m_bExpanded;
	bool m_bDragging;
	int m_iDraggingItem;

private:
	void* luastate;
	AEditBox*	m_editName;
	AListView*  m_CheatsList;
	ACheatFile* m_pFile;

	CLayout		m_layout;
	
};

#endif//!__CHEAT_INFO_DLG_H__
