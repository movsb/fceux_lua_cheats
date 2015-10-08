#pragma once
#include "WindowBase.h"

class ATreeView:public AWindowBase
{
public:
	ATreeView(void){};
	~ATreeView(void){};

public:
	BOOL DeleteAllItems()
	{
		return TreeView_DeleteAllItems(this->GetHwnd());
	}
	BOOL DeleteItem(HTREEITEM hitem)
	{
		return TreeView_DeleteItem(this->GetHwnd(),hitem);
	}
	UINT GetCount()
	{
		return TreeView_GetCount(this->GetHwnd());
	}
	BOOL GetItem(LPTVITEMEX pitem)
	{
		return TreeView_GetItem(this->GetHwnd(),pitem);
	}
	BOOL GetItemLParam(HTREEITEM hItem,LPARAM* pLParam)
	{
		TVITEMEX tvie = {0};
		tvie.mask = TVIF_PARAM;
		tvie.hItem = hItem;
		if(GetItem(&tvie)){
			*pLParam = tvie.lParam;
			return TRUE;
		}else{
			return FALSE;
		}
	}
	LPARAM GetItemLParam(HTREEITEM hItem,BOOL* bSucceed=0)
	{
		LPARAM param=0;
		BOOL bSucc = GetItemLParam(hItem,&param);
		if(bSucceed) *bSucceed = bSucc;
		return param;
	}
	HTREEITEM GetNextItem(HTREEITEM hitem,UINT flag)
	{
		return TreeView_GetNextItem(this->GetHwnd(),hitem,flag);
	}
	HTREEITEM GetChild(HTREEITEM hitem)
	{
		return TreeView_GetChild(this->GetHwnd(),hitem);
	}
	HTREEITEM GetParent(HTREEITEM hitem)
	{
		return TreeView_GetParent(this->GetHwnd(),hitem);
	}
	HTREEITEM GetRoot()
	{
		return TreeView_GetRoot(this->GetHwnd());
	}
	HTREEITEM GetNextChild(HTREEITEM hitem)//重名了,换一个
	{
		return TreeView_GetNextSibling(this->GetHwnd(),hitem);
	}
	HTREEITEM HitTest(LPTVHITTESTINFO lpht)
	{
		return TreeView_HitTest(this->GetHwnd(),lpht);
	}
	HTREEITEM InsertItem(LPTVINSERTSTRUCT lpis)
	{
		return TreeView_InsertItem(this->GetHwnd(),lpis);
	}
	BOOL SetItem(LPTVITEMEX pitem)
	{
		return TreeView_SetItem(this->GetHwnd(),pitem);
	}
	BOOL SelectItem(HTREEITEM hitem)
	{
		return TreeView_SelectItem(this->GetHwnd(),hitem);
	}
	HRESULT SetExtendedStyle(DWORD dw,UINT mask)
	{
		return TreeView_SetExtendedStyle(this->GetHwnd(),dw,mask);
	}
	HWND EditLabel(HTREEITEM hitem)
	{
		return TreeView_EditLabel(this->GetHwnd(),hitem);
	}
	BOOL Expand(HTREEITEM hItem,UINT flag)
	{
		return TreeView_Expand(this->GetHwnd(),hItem,flag);
	}





	
public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		return CallWindowProc(m_WndProcOrig,this->GetHwnd(),uMsg,wParam,lParam);
	}
};
