#ifndef __CHEAT_INFO_DLG_H__
#define __CHEAT_INFO_DLG_H__

#include "WindowBase.h"
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

public:
	void ShowCheatInfo(ACheatEntry* pEntry,void (__cdecl* SetModify)(bool bModified));
	void MoveItemToAnotherPlace(int ifrom,int ito);
	//void ShowScriptIndexof(int index);
	ACheatEntry::VALUE* GetScriptIndexof(int index);

private:
	void InitControls();
	int AddListItem(const char* pszText,LPARAM param);
	int UpdateListItem(int index,const char* pszText,LPARAM param);

private:
	void (__cdecl* m_SetModify)(bool bModified);
	ACheatEntry* m_pEntry;
	
	//bool m_bExpanded;
	bool m_bDragging;
	int m_iDraggingItem;

private:
	void* luastate;
	AEditBox*	m_editName;
	AListView*  m_CheatsList;
// 	AButton*	m_BtnExpand;
// 	AButton*	m_BtnExecute;
// 	AButton*	m_BtnDelete;
	
};

#endif//!__CHEAT_INFO_DLG_H__
