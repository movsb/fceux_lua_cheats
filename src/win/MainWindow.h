#include "ctl/WindowBase.h"

#include "../tables/cheat.h"

class ACheatInfoDlg;
class AMainWindow;
class TiXmlNode;


struct ParamStruct
{
	ParamStruct(int _type,void* _pv1,void* _pv2=NULL,void* _pv3=NULL)
	{
		type = _type;
		pv1 = _pv1;
		pv2 = _pv2;
		pv3 = _pv3;
	}
	enum{
		PS_CHEAT,PS_CHEAT_ITEM,PS_CHEAT_DIR,
		PS_NULL_ENUM
	};
	int type;
	void* pv1;
	void* pv2;
	void* pv3;
};

class AMainWindow:public AWindowBase
{
public:
	AMainWindow();
	~AMainWindow();

	HTREEITEM AddSubTreeItem(HTREEITEM hParent,const char* pszText,LPARAM param);
	void UpdateTreeItemText(HTREEITEM hItem,const char* pszText);

public:
	INT_PTR DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam);
	INT_PTR OnClose();
	INT_PTR OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam);
	INT_PTR OnNotify(LPNMHDR phdr);
	INT_PTR OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl);
	INT_PTR OnSize(int width,int height);
	INT_PTR OnMouseMove(int key,int x,int y);
	INT_PTR OnLButtonUp(int key,int x,int y);
	INT_PTR OnNull(LPARAM lParam);
	INT_PTR OnDropFiles(HDROP hDrop);


private:
	
	void InitTreeViewStyles();
	bool GetFileName(HWND hwnd,char* title, char* filter,char* buffer);
	bool GetSaveFile(HWND hwnd,const char* title, char* filter,char* buffer,char* defext=0);
	bool ShowMenuFor(int type);
	INT_PTR HandleDblClick();
	bool GetMenuTreeItem(HTREEITEM* phitem=NULL);


	
private:
	void GetFileListFromOFN(vector<string>* files,const char* fs);

private:
	ACheatInfoDlg* m_CheatInfoDlg;
	CLayout		   m_layout;
private:
	ATreeView* m_pTree;
	ParamStruct* m_MenuPS;
	ACheatFile*  m_MenuFile;
	ACheatEntry* m_pCheatEntryCurrent;
	ACheatFile*  m_pCheatFileCurrent;
	HTREEITEM    m_hCheatEntryCurrent;
	HTREEITEM m_MenuhItem;
	BOOL m_bDragging;
	HTREEITEM m_hDraggingItem;
	bool m_bSaving;	//表示当前是否正在保存所有xml文档,是就退出函数

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 

public:
	bool OpenCheatXml(const char* file);
	bool OpenCheatXml(ACheatFile* file);
	bool CloseCheatXml();
	bool SaveAllCheatFiles();
	bool SaveCheatFile(HTREEITEM hItem);

public:
	HTREEITEM InsertRoot(const char* name,ACheatFile* pfile);
	void AddCheatEntryPointers(ACheatEntry* pEntry,HTREEITEM hCurrent);
	void DelCheatEntryPointers(HTREEITEM hCurrent);
	bool DumpCheatsToXml(HTREEITEM hFileTree,string fname);
	bool DumpCheatsToXml(HTREEITEM hFileTree,string* str);



private:
	void DumpCheatsTree(ACheatEntry* pEntry,void* node);
	void GetCheatParentAndSiblingsPointers(HTREEITEM hCurrent,void** parent,bool* isFile,ACheatEntry** pPrev,ACheatEntry** pNext);
	bool DumpCheatsToXml(HTREEITEM hFileTree,void* pdoc);

	BOOL MoveCheatBetweenNodes(HTREEITEM hFrom,HTREEITEM hTo);
	HTREEITEM GetCheatFileNodeOfCheatOrCheatNode(HTREEITEM hItem);

public:
	bool MakeCheatsListFromFile(const char* fn,ACheatFile** ppFile);

private:
	void TravelCheatNode(ACheatEntry* cheat,TiXmlNode* node);

public:
	void FreeAllCheatFile(HTREEITEM hFile);
	void FreeCheatEntry(ACheatEntry* pEntry);
	void DeleteCheatItem(HTREEITEM hCurrent);
	void DeleteCheatNode(HTREEITEM hCurrent);
	HTREEITEM ListCheatDir(HTREEITEM hParent,ACheatFile* pfile,ACheatEntry* node);

private:
	void DeleteCheatNode(ACheatEntry* pEntry,bool bRecursiveCall=false);
};
