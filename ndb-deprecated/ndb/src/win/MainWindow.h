#include <windows.h>

#include "WindowBase.h"

#include "../tables/table.h"
//#include "../tinyxml/tinyxml.h"

#include <map>

#define __FCEUX__

class ARomInfoDlg;
class ACheatInfoDlg;
class AStateInfoDlg;

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
		PS_ROOTPAGE=0,
		PS_PAGE_ROM,PS_ROM,
		PS_PAGE_CHEAT,PS_CHEAT,PS_CHEAT_ITEM,PS_CHEAT_DIR,
		PS_PAGE_STATE,PS_STATE_FILE,PS_STATE_ITEM,
		PS_PAGE_MAP,PS_MAP,
		PS_PAGE_NSF,PS_NSF,
		PS_PAGE_ATTACH,PS_ATTACH,

		PS_NULL_ENUM
	};
	int type;
	void* pv1;
	void* pv2;
	void* pv3;
};

class ARootPage
{
public:
	ARootPage();
	~ARootPage();

public:
	BOOL OpenDatabase(ATreeView* pTree,const char* filename);
	BOOL CloseDatabase();
	HTREEITEM AddSubTreeItem(HTREEITEM hParent,const char* pszText,LPARAM param);
	void AddCheatEntryPointers(ACheatEntry* pEntry,HTREEITEM hCurrent);
	void DelCheatEntryPointers(HTREEITEM hCurrent);
	bool DumpCheatsToXml(HTREEITEM hFileTree,string fname);
	bool DumpCheatsToXml(HTREEITEM hFileTree,string* str);
	void UpdateTreeItemText(HTREEITEM hItem,const char* pszText);


private:
	const char* GetDBName(const char* fn);
	void InsertRoot(const char* fn);
	void RemoveRoot();
	void InitDataTrees();
	void RemoveDataTrees();
	void DumpCheatsTree(ACheatEntry* pEntry,void* node);
	void GetCheatParentAndSiblingsPointers(HTREEITEM hCurrent,void** parent,bool* isFile,ACheatEntry** pPrev,ACheatEntry** pNext);
	bool DumpCheatsToXml(HTREEITEM hFileTree,void* pdoc);

private:
	char m_filename[MAX_PATH];
	HTREEITEM m_hThisTree;
	enum{/*HTH_ROM=0,*/HTH_CHEAT/*,HTH_STATE,HTH_MAP,HTH_NSF,HTH_ATTACH*/,HTH_NULL};
	HTREEITEM m_hTreeItems[HTH_NULL];

private:
	ATreeView*	m_pTree;
	ATable*		m_Table;
	ARom*		m_Rom;
	ACheat*		m_Cheat;
	AState*		m_State;
	ACheatEntry* m_CurrentCheatEntry;
	HIMAGELIST m_ImageList;

	//bool		m_bCheatsNeedSaving;

public:
	void CreateImageList();
	void BuildRomTree();
	void CleanRomTree();
	void AddRom(ARomMap* rom_map);
	ARom* GetRom(){return m_Rom;}

public:
	void BuildStateTree();
	AState* GetState(){return m_State;}

public:
	std::map<HTREEITEM,bool> m_bCheatNeedSaving;
	AThunk					 m_TheFuckThunk;
public:
	HTREEITEM    m_CurrentCheatEntryTreeItem;
	void BuildCheatTree();
	void CleanCheatTree();
	void CheatSetModify(HTREEITEM hItem,bool bModified);
	bool CheatGetModify(HTREEITEM hItem);
	void __cdecl CheatSetModifyCallback(bool bModified);
	void FreeCheatEntry(ACheatEntry* pEntry);
	void DeleteCheatItem(HTREEITEM hCurrent);
	void DeleteCheatNode(HTREEITEM hCurrent/*,bool recursiveCall=false*/);
	void SetCurrentCheatEntry(ACheatEntry* pEntry){m_CurrentCheatEntry=pEntry;}
	ACheatEntry* GetCurrentCheatEntry(){return m_CurrentCheatEntry;}
	HTREEITEM ListCheatDir(HTREEITEM hParent,ACheatEntry* node);
	void AddCheat(ACheatMap* cheat_map,ACheatFile* newFile=0);
	ACheat* GetCheat(){return m_Cheat;}

private:
	void DeleteCheatNode(ACheatEntry* pEntry,bool bRecursiveCall=false);
};

class AMainWindow:public AWindowBase
{
public:
	AMainWindow();
	~AMainWindow();

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

public:
	bool OpenDatabaseTmp(const char* name);
	bool CloseAllDatabase();

private:
	
	void InitTreeViewStyles();
	bool GetFileName(HWND hwnd,char* title, char* filter,char* buffer);
	bool GetSaveFile(HWND hwnd,char* title, char* filter,char* buffer,char* defext=0);
	void ShowMenuFor(int type);
	//INT_PTR HandleMenuFor(int id,bool* handled);
	bool HandleMenuForRom(int id,INT_PTR& result);
	bool HandleMenuForCheat(int id,INT_PTR& result);
	//bool HandleMenuForRootPage(int id,INT_PTR& result);
	INT_PTR HandleDblClick();
	bool GetMenuTreeItem(HTREEITEM* phitem=NULL);
	BOOL MoveCheatBetweenNodes(HTREEITEM hFrom,HTREEITEM hTo);
	HTREEITEM GetCheatFileNodeOfCheatOrCheatNode(HTREEITEM hItem);
	void ResizePages(int width,int height);
	

private:
	void ShowPage(int type);
	void MakeRomInfo(ARomMap* rom_map);
	void MakeCheatInfo(ACheatEntry* pEntry,void (__cdecl* SetModify)(bool bModified)=0);
	void MakeStateInfo(AState* state,list_s* ls);

private:
	ARomInfoDlg* m_RomInfoDlg;
	ACheatInfoDlg* m_CheatInfoDlg;
	AStateInfoDlg* m_StateInfoDlg;
private:
	ATreeView* m_TreeView;
	ParamStruct* m_MenuPS;
	HTREEITEM m_MenuhItem;
	ARootPage* m_MenuRootPage;
	ACheatEntry* m_CurrentCheatEntry;
	HTREEITEM    m_CurrentCheatEntryTreeItem;
	BOOL m_bDragging;
	HTREEITEM m_hDraggingItem;
};
