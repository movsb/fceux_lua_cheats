#undef  NOMINMAX
#include <windows.h>
#include <ShlObj.h>
#include <gdiplus.h>
#include <vector>

using namespace Gdiplus;
#include <cstdio>
using namespace std;

#include "../../res/resource.h"
#include "MainWindow.h"
#include "TreeView.h"
#include "RomInfoDlg.h"
#include "CheatInfoDlg.h"
#include "CheatFileInfoDlg.h"
#include "StateInfoDlg.h"

#include "../tables/table.h"
#include "../tinyxml/tinyxml.h"
#include "../Str.h"

#include "Archive.h"
#include "App.h"
#include <shlwapi.h>

AApp* theApp = new AApp;

ARootPage::ARootPage()
{
	m_Table = new ATable;
	m_Rom = new ARom;
	m_Cheat = new ACheat;
	m_State = new AState;

	//CreateImageList();
	m_ImageList = 0;
}
ARootPage::~ARootPage()
{
	delete m_Table;
	delete m_Rom;
	delete m_Cheat;
	delete m_State;
}

void ARootPage::UpdateTreeItemText(HTREEITEM hItem,const char* pszText)
{
	TVITEMEX tvi = {0};
	tvi.mask = TVIF_TEXT;
	tvi.hItem = hItem;
	tvi.pszText = (LPTSTR)pszText;
	m_pTree->SetItem(&tvi);
}

bool ARootPage::DumpCheatsToXml(HTREEITEM hFileTree,void* pdoc)
{
	TiXmlDocument& doc = *(TiXmlDocument*)pdoc;
	ACheatFile* pFile = (ACheatFile*)((ParamStruct*)m_pTree->GetItemLParam(hFileTree))->pv3;

	//ÿ���һ���ڵ��link

	//XML�ļ�����
	TiXmlDeclaration* declaration = new TiXmlDeclaration("1.0","GB2312","");
	doc.LinkEndChild(declaration);

	//NesCheat�ڵ�
	TiXmlElement* nodeNesCheat = new TiXmlElement("NesCheat");
	nodeNesCheat->SetAttribute("version","1.0");

	//Info�ڵ�
	TiXmlElement* nodeInfo = new TiXmlElement("Info");

	TiXmlElement* nodeInfoName = new TiXmlElement("Name");
	TiXmlText* nodeInfoNameText = new TiXmlText(pFile->name.c_str());
	nodeInfoName->LinkEndChild(nodeInfoNameText);
	nodeInfo->LinkEndChild(nodeInfoName);

	if(pFile->md5.size()){
		TiXmlElement* nodeInfoMd5 = new TiXmlElement("Md5");
		TiXmlText* nodeInfoMd5Text = new TiXmlText(pFile->md5.c_str());
		nodeInfoMd5->LinkEndChild(nodeInfoMd5Text);
		nodeInfo->LinkEndChild(nodeInfoMd5);
	}

	if(pFile->crc32.size()){
		TiXmlElement* nodeInfoCrc32 = new TiXmlElement("Crc32");
		TiXmlText* nodeInfoCrc32Text = new TiXmlText(pFile->crc32.c_str());
		nodeInfoCrc32->LinkEndChild(nodeInfoCrc32Text);
		nodeInfo->LinkEndChild(nodeInfoCrc32);
	}

	if(pFile->author.size()){
		TiXmlElement* nodeInfoAuthor = new TiXmlElement("Author");
		TiXmlText* nodeInfoAuthorText = new TiXmlText(pFile->author.c_str());
		nodeInfoAuthor->LinkEndChild(nodeInfoAuthorText);
		nodeInfo->LinkEndChild(nodeInfoAuthor);
	}

	if(pFile->time.size()){
		TiXmlElement* nodeInfoTime = new TiXmlElement("Time");
		TiXmlText* nodeInfoTimeText = new TiXmlText(pFile->time.c_str());
		nodeInfoTime->LinkEndChild(nodeInfoTimeText);
		nodeInfo->LinkEndChild(nodeInfoTime);
	}

	TiXmlElement* nodeInfoDesc = new TiXmlElement("Desc");
	TiXmlText* nodeInfoDescText = new TiXmlText(pFile->desc.c_str());
	nodeInfoDesc->LinkEndChild(nodeInfoDescText);
	nodeInfo->LinkEndChild(nodeInfoDesc);

	nodeNesCheat->LinkEndChild(nodeInfo);

	TiXmlElement* nodeCheats = new TiXmlElement("Cheats");
	DumpCheatsTree(pFile->entry,nodeCheats);

	nodeNesCheat->LinkEndChild(nodeCheats);

	doc.LinkEndChild(nodeNesCheat);

	return true;
}

bool ARootPage::DumpCheatsToXml(HTREEITEM hFileTree,string fname)
{
	TiXmlDocument doc;
	return DumpCheatsToXml(hFileTree,&doc)
		&& doc.SaveFile(fname.c_str());
}

bool ARootPage::DumpCheatsToXml(HTREEITEM hFileTree,string* str)
{
	TiXmlDocument doc;

	if(DumpCheatsToXml(hFileTree,&doc)){
		TiXmlPrinter printer;
		printer.SetIndent("\t");
		if(doc.Accept(&printer)){
			*str = printer.CStr();
			return true;
		}
	}
	return false;
}

void ARootPage::DumpCheatsTree(ACheatEntry* pEntry,void* node)
{
	TiXmlElement* pnode = (TiXmlElement*)node;
	while(pEntry){
		if(pEntry->type == ACheatEntry::TYPE_NODE){
			TiXmlElement* nodeNode = new TiXmlElement("Node");
			nodeNode->SetAttribute("name",pEntry->node.name.c_str());
			DumpCheatsTree(pEntry->child,nodeNode);
			pnode->LinkEndChild(nodeNode);
		}else if(pEntry->type == ACheatEntry::TYPE_CHEAT){
			TiXmlElement* nodeItem = new TiXmlElement("Item");

			TiXmlElement* nodeItemName = new TiXmlElement("Name");
			TiXmlText* nodeItemNameText = new TiXmlText(pEntry->item.name.c_str());
			nodeItemName->LinkEndChild(nodeItemNameText);
			nodeItem->LinkEndChild(nodeItemName);

			TiXmlElement* nodeItemDesc = new TiXmlElement("Desc");
			TiXmlText* nodeItemDescText = new TiXmlText(pEntry->item.desc.c_str());
			nodeItemDesc->LinkEndChild(nodeItemDescText);
			nodeItem->LinkEndChild(nodeItemDesc);

			TiXmlElement* nodeItemCustom = new TiXmlElement("Custom");
			TiXmlText* nodeItemCustomText = new TiXmlText(pEntry->item.custom.c_str());
			nodeItemCustom->LinkEndChild(nodeItemCustomText);
			nodeItem->LinkEndChild(nodeItemCustom);

			//dump ���е�value
			TiXmlElement* nodeItemValues = new TiXmlElement("Values");
			for(list_s* p=pEntry->item.values.next; p!=&pEntry->item.values; p=p->next){
				TiXmlElement* v = new TiXmlElement("Value");
				ACheatEntry::VALUE* pValue = list_data(p,ACheatEntry::VALUE,entry);

				TiXmlElement* vName = new TiXmlElement("Name");
				TiXmlText* vNameText = new TiXmlText(pValue->name.c_str());
				vName->LinkEndChild(vNameText);
				v->LinkEndChild(vName);

				TiXmlElement* vScript = new TiXmlElement("Script");
				TiXmlText* vScriptText = new TiXmlText(pValue->script.c_str());
				vScript->LinkEndChild(vScriptText);
				v->LinkEndChild(vScript);

				nodeItemValues->LinkEndChild(v);
			}
			nodeItem->LinkEndChild(nodeItemValues);
			pnode->LinkEndChild(nodeItem);
		}
		pEntry = pEntry->next;
	}
}

void ARootPage::GetCheatParentAndSiblingsPointers(HTREEITEM hCurrent,void** parent,bool* isFile,ACheatEntry** pPrev,ACheatEntry** pNext)
{
	//ȡ�ø����ָ��
	HTREEITEM hTvParent = m_pTree->GetNextItem(hCurrent,TVGN_PARENT);
	ParamStruct* psParent = (ParamStruct*)m_pTree->GetItemLParam(hTvParent);
	//����2ѡ1
	ACheatEntry* pParentEntry = (ACheatEntry*)psParent->pv2;
	ACheatFile*  pParentFile = (ACheatFile*)psParent->pv3;
	bool bIsParentFile = psParent->type==ParamStruct::PS_CHEAT;
	assert(psParent->type==ParamStruct::PS_CHEAT || psParent->type==ParamStruct::PS_CHEAT_DIR);

	assert(hTvParent!=NULL && psParent!=NULL && "ARootPage::GetCheatParentAndSiblingsPointers");

	//ȡ��PrevSibling
	HTREEITEM hTvPrevSibling = m_pTree->GetNextItem(hCurrent,TVGN_PREVIOUS);
	ParamStruct* psPrev = 0;
	ACheatEntry* pPrevEntry = 0;
	if(hTvPrevSibling){
		psPrev = (ParamStruct*)m_pTree->GetItemLParam(hTvPrevSibling);
		pPrevEntry = (ACheatEntry*)psPrev->pv2;
	}

	//ȡ��NextSibling
	HTREEITEM hTvNextSibling = m_pTree->GetNextItem(hCurrent,TVGN_NEXT);
	ParamStruct* psNext = 0;
	ACheatEntry* pNextEntry = 0;
	if(hTvNextSibling){
		psNext = (ParamStruct*)m_pTree->GetItemLParam(hTvNextSibling);
		pNextEntry = (ACheatEntry*)psNext->pv2;
	}

	*isFile = bIsParentFile;
	*parent = bIsParentFile?(void*)pParentFile:(void*)pParentEntry;
	*pPrev  = pPrevEntry;
	*pNext  = pNextEntry;
}

void ARootPage::AddCheatEntryPointers(ACheatEntry* pEntry,HTREEITEM hCurrent)
{
	
	bool bIsParentFile;
	ACheatEntry* pParentEntry=0;
	ACheatEntry* pPrevEntry=0;
	ACheatEntry* pNextEntry=0;
	ACheatFile* pParentFile=0;
	void* ppp = 0;

	GetCheatParentAndSiblingsPointers(hCurrent,&ppp,&bIsParentFile,&pPrevEntry,&pNextEntry);
	if(bIsParentFile) pParentFile  = (ACheatFile*)ppp;
	else			  pParentEntry = (ACheatEntry*)ppp;


	//��дparent��child,�����������ļ���Ŀ¼,��Ҫ���ж�
	//pEntry->child = 0;		//δ�����½ڵ�//�½��϶�û�к���(ֻ��Ŀ¼����)
	pEntry->parent = bIsParentFile?0:pParentEntry;

	//��дnext��prev
	//pEntry->prev = bHasPrev?pPrevEntry:0;
	pEntry->prev = pPrevEntry;
	pEntry->next = pNextEntry;

	//////////////////////////////////////////////////////////////////////////

	//���·ǵ�ǰ����ָ����
	if(pPrevEntry){
		pPrevEntry->next = pEntry;
	}
	if(pNextEntry){
		pNextEntry->prev = pEntry;
	}
	if(!pPrevEntry){
		if(bIsParentFile) pParentFile->entry  = pEntry;
		else			  pParentEntry->child = pEntry;
	}
}

void ARootPage::DelCheatEntryPointers(HTREEITEM hCurrent)
{
	bool bIsParentFile;
	ACheatEntry* pParentEntry=0;
	ACheatEntry* pPrevEntry=0;
	ACheatEntry* pNextEntry=0;
	ACheatFile* pParentFile=0;
	void* ppp = 0;

	GetCheatParentAndSiblingsPointers(hCurrent,&ppp,&bIsParentFile,&pPrevEntry,&pNextEntry);
	if(bIsParentFile) pParentFile  = (ACheatFile*)ppp;
	else			  pParentEntry = (ACheatEntry*)ppp;

	 if(pPrevEntry){
		 pPrevEntry->next = pNextEntry;
	 }else{
		if(bIsParentFile)	pParentFile->entry  = pNextEntry;
		else				pParentEntry->child = pNextEntry;
	 }
}

HTREEITEM ARootPage::AddSubTreeItem(HTREEITEM hParent,const char* pszText,LPARAM param)
{
	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT|TVIF_PARAM/*|TVIF_IMAGE*/;
	tvis.itemex.iImage = 0;
	tvis.itemex.pszText = (LPTSTR)pszText;
	tvis.itemex.lParam = param;
	return m_pTree->InsertItem(&tvis);
}

void ARootPage::CreateImageList()
{
	m_ImageList = ImageList_Create(16,16,ILC_COLOR16,2,10);
	HBITMAP hBmp = 0;
	HINSTANCE hInst = GetModuleHandle(0);
	hBmp = ::LoadBitmap(hInst,MAKEINTRESOURCE(IDB_DIR));
	ImageList_Add(m_ImageList,hBmp,NULL);
	hBmp = ::LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CHEAT));
	ImageList_Add(m_ImageList,hBmp,NULL);
}

void ARootPage::BuildRomTree()
{
	CleanRomTree();
	m_Rom->OpenDb(m_Table->GetDb());
	ARomMap* rom_map = m_Rom->GetRomHeaders();
	while(rom_map){
		AddRom(rom_map);
		rom_map = rom_map->next;
	}
}

void ARootPage::CleanRomTree()
{
	assert(0);
	int HTH_ROM=0;
	HTREEITEM hChild = m_pTree->GetChild(m_hTreeItems[HTH_ROM]);
	while(hChild){
		LPARAM param;
		if(m_pTree->GetItemLParam(hChild,&param)){
			assert(param!=0 && "ARootPage::CleanRomTree()");
			ParamStruct* ps = (ParamStruct*)param;
			ARomMap* rom_map = (ARomMap*)ps->pv2;
			delete[] rom_map->phdr;
			delete rom_map;
			delete ps;
		}
		HTREEITEM old = hChild;
		hChild = m_pTree->GetNextChild(hChild);
		m_pTree->DeleteItem(old);
	}
}

void ARootPage::AddRom(ARomMap* rom_map)
{
	assert(0);
	int HTH_ROM=0;
	AddSubTreeItem(m_hTreeItems[HTH_ROM],(LPTSTR)rom_map->phdr->comment,(LPARAM)new ParamStruct(ParamStruct::PS_ROM,this,rom_map));
}

//////////////////////////////////////////////////////////////////////////

void ARootPage::BuildCheatTree()
{
	CleanCheatTree();
	m_Cheat->OpenDb(m_Table->GetDb());
	ACheatMap* cheat_map = m_Cheat->GetCheatHeaders();
	while(cheat_map){
		AddCheat(cheat_map);
		cheat_map = cheat_map->next;
	}
}

void ARootPage::CleanCheatTree()
{
	HTREEITEM hChild = m_pTree->GetChild(m_hTreeItems[HTH_CHEAT]);
	while(hChild){
		LPARAM param;
		if(m_pTree->GetItemLParam(hChild,&param)){

			if(m_bCheatNeedSaving[hChild]){
				assert(0);
			}

			assert(param!=0 && "ARootPage::CleanCheatTree()");
			ParamStruct* ps = (ParamStruct*)param;
			ACheatMap* cheat_map = (ACheatMap*)ps->pv2;
			delete[] cheat_map->phdr;
			delete cheat_map;
			delete ps;
		}
		HTREEITEM old = hChild;
		hChild = m_pTree->GetNextChild(hChild);
		m_pTree->DeleteItem(old);
	}
}

void ARootPage::CheatSetModify(HTREEITEM hItem,bool bModified)
{
	m_bCheatNeedSaving[hItem] = bModified;
}
bool ARootPage::CheatGetModify(HTREEITEM hItem)
{
	return m_bCheatNeedSaving[hItem];
}
void ARootPage::CheatSetModifyCallback(bool bModified)
{
	m_bCheatNeedSaving[m_CurrentCheatEntryTreeItem] = bModified;
}

void ARootPage::FreeCheatEntry(ACheatEntry* pEntry)
{
	if(pEntry->type == ACheatEntry::TYPE_NODE){
		//nothing to do with a node
	}else if(pEntry->type == ACheatEntry::TYPE_CHEAT){
// 		for(list_s* p=pEntry->item.values.next; p!=&pEntry->item.values; p=p->next){
// 			ACheatEntry::VALUE* pValue = list_data(p,ACheatEntry::VALUE,entry);
// 			delete pValue;
// 		}
		while(!list_is_empty(&pEntry->item.values)){
			list_s* p = list_remove_head(&pEntry->item.values);
			ACheatEntry::VALUE* pValue = list_data(p,ACheatEntry::VALUE,entry);
			delete pValue;
		}
	}else{
		assert(0 && "ARootPage::FreeCheatEntry()");
	}
	delete pEntry;
}


void ARootPage::DeleteCheatItem(HTREEITEM hCurrent)
{
	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hCurrent);
	ACheatEntry* pEntry = (ACheatEntry*)ps->pv2;
	assert(pEntry->type == ACheatEntry::TYPE_CHEAT);

	DelCheatEntryPointers(hCurrent);	//��˫���������Ƴ��ڵ�
	m_pTree->DeleteItem(hCurrent);		//ɾ�������ڵ�
	FreeCheatEntry(pEntry);				//�ͷŽڵ��ڴ�
	delete ps;							//�ͷ�ParamStruct
}

void ARootPage::DeleteCheatNode(ACheatEntry* pEntry,bool bRecursiveCall)
{
	while(pEntry){
		ACheatEntry* tmp = pEntry->next;

		if(pEntry->type == ACheatEntry::TYPE_NODE){
			DeleteCheatNode(pEntry->child,true);
			delete pEntry;
		}else{
			if(GetCurrentCheatEntry()==pEntry){
				SetCurrentCheatEntry(0);
			}
			FreeCheatEntry(pEntry);
		}
		pEntry = tmp;
	}
}

void ARootPage::DeleteCheatNode(HTREEITEM hCurrent)
{
	//��Ч��ע��
	//����ǵݹ�ɾ��Ŀ¼,��������һ��ѭ��ʱ��Ҫȡ��ǰ�ڵ����һ��,���Բ���ɾ��
	//Ҫ����һ��Ŀ¼ɾ��֮���ɾ��Ŀ¼treeitem
	// 
	//Ҫ�Ƴ�ĳ��Ŀ¼,ֻ��Ҫ�Ѹ�Ŀ¼��prev,next�ڵ���������Ƴ�,child���ù�
	//childֻ��Ҫ�ͷ��ڴ�, ���Կ������ü򵥵�
	//TreeView��DeleteItemʱ,��ɾ����Item���е���Item
	//ͬ��������ɾ���ļ��ڵ�

	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hCurrent);
	ACheatEntry* pEntry = 0;
	bool bIsFile = ps->type == ParamStruct::PS_CHEAT;
	if(bIsFile){//�ļ�
		//pEntry = (ACheatEntry*)ps->pv3;
		pEntry = (ACheatEntry*)((ACheatFile*)ps->pv3)->entry;
		if(!pEntry) return;
	}else{
		pEntry = (ACheatEntry*)ps->pv2;
		assert(pEntry->type == ACheatEntry::TYPE_NODE);
	}

	DeleteCheatNode(pEntry->child);

	if(bIsFile==false){
		DelCheatEntryPointers(hCurrent);
		m_pTree->DeleteItem(hCurrent);//BUG:û��ɾ��PS
		delete pEntry;
	}else{
		ACheatFile* file = (ACheatFile*)ps->pv3;
		file->entry = 0;
	}
}

HTREEITEM ARootPage::ListCheatDir(HTREEITEM hParent,ACheatEntry* node)
{
	if(!node) return NULL;

	ACheatEntry* pEntry = node;

	if(pEntry){
		do{
			if(pEntry->type == ACheatEntry::TYPE_NODE){
				HTREEITEM hDir = AddSubTreeItem(
					hParent,
					pEntry->node.name.c_str(),
					(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_DIR,this,pEntry));
				//�ݹ�������ӽڵ�
				ListCheatDir(hDir,pEntry->child);
			}else if(pEntry->type == ACheatEntry::TYPE_CHEAT){
				AddSubTreeItem(
					hParent,
					pEntry->item.name.c_str(),
					(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_ITEM,this,pEntry));
			}
		}while(pEntry=pEntry->next);
	}
	return NULL;
}

void ARootPage::AddCheat(ACheatMap* cheat_map,ACheatFile* newFile)
{
	TVINSERTSTRUCT tvis = {0};
	ParamStruct* ps = NULL;
	HTREEITEM hItem;

	ps = new ParamStruct(ParamStruct::PS_CHEAT,this,cheat_map,newFile);

	hItem = AddSubTreeItem(m_hTreeItems[HTH_CHEAT],(LPSTR)cheat_map->phdr->comment,(LPARAM)ps);

	unsigned char* data;
	size_t size;
	if(!newFile && m_Cheat->GetCheatData(cheat_map->idx,(void**)&data,&size)){
		if(ATable::SaveToFile(data,size,".cheat.xml")){//TODO:IStream
			ACheatFile* file = NULL;
			m_Cheat->MakeCheatsListFromFile(".cheat.xml",&file);
			DeleteFile(".cheat.xml");
			ListCheatDir(hItem,file->entry);
			ps->pv3 = file;
			UpdateTreeItemText(hItem,file->name.c_str());
		}
	}
	//m_bCheatNeedSaving[hItem] = false;
	m_bCheatNeedSaving[hItem] = newFile!=0;
}

//////////////////////////////////////////////////////////////////////////
void ARootPage::BuildStateTree()
{
	struct FILE_TREE{
		HTREEITEM hItem;
		std::string* file;

		FILE_TREE(HTREEITEM _hItem,std::string* _file)
		{
			hItem = _hItem;
			file = _file;
		}
	};
	std::vector<FILE_TREE> files;

	m_State->OpenDB(m_Table->GetDb());

// 	AStateHeader hdr;
// 	int i=9;
// 	hdr.file = "��Ѫ";
// 	hdr.name = "����";
// 	m_State->AddState(&hdr,&i,4,0);
// 
// 	hdr.file = "��Ѫ";
// 	hdr.name = "����";
// 	m_State->AddState(&hdr,&i,4,0);
// 
// 	hdr.file = "����";
// 	hdr.name = "1";
// 	m_State->AddState(&hdr,&i,4,0);
// 
// 	hdr.file = "����";
// 	hdr.name = "2";
// 	m_State->AddState(&hdr,&i,4,0);
// 
// 	hdr.file = "��Ѫ";
// 	hdr.name = "�Ǻ�";
// 	m_State->AddState(&hdr,&i,4,0);

	AStateMap* smap = m_State->GetStateHeaders();
	while(smap){
		HTREEITEM hItemAdded=0;
		for(std::vector<FILE_TREE>::iterator it=files.begin();
			it!=files.end();
			it++)
		{
			if(*it->file == smap->phdr->file){
				hItemAdded = it->hItem;
				break;
			}
		}
		if(hItemAdded == 0){
			//ParamStruct* ps = new ParamStruct(ParamStruct::PS_STATE_FILE,this,new list_s);
			list_s* entry = new list_s;
			list_init(entry);

			assert(0);
			int HTH_STATE=0;
			hItemAdded=AddSubTreeItem(

				m_hTreeItems[HTH_STATE],
				smap->phdr->file.c_str(),
				(LPARAM)new ParamStruct(ParamStruct::PS_STATE_FILE,this,entry));
			
			//AddSubTreeItem(hItemAdded,smap->phdr->name.c_str(),0);
			list_insert_tail(entry,&smap->entry);

			files.push_back(FILE_TREE(hItemAdded,&smap->phdr->file));
		}else{
			//AddSubTreeItem(hItemAdded,smap->phdr->name.c_str(),0);
			ParamStruct* ps= (ParamStruct*)m_pTree->GetItemLParam(hItemAdded);
			list_s* p = (list_s*)ps->pv2;
			list_insert_tail(p,&smap->entry);
		}
		smap = smap->next;
	}
}

//////////////////////////////////////////////////////////////////////////

const char* ARootPage::GetDBName(const char* fn)
{
	assert(fn!=NULL && "ARootPage::GetDBName()");

	const char* p = strrchr(fn,'\\');
	if(!p) p = strrchr(fn,'/');
	return p?p+1:fn;
}

void ARootPage::InsertRoot(const char* fn)
{
	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT|TVIF_PARAM/*|TVIF_IMAGE*/;
	tvis.itemex.iImage = 0;
	tvis.itemex.pszText = (LPSTR)GetDBName(fn);
	tvis.itemex.lParam = (LPARAM)new ParamStruct(ParamStruct::PS_ROOTPAGE,this);
	m_hThisTree = m_pTree->InsertItem(&tvis);
}

void ARootPage::RemoveRoot()
{
	LPARAM param;
	HTREEITEM hChild = m_pTree->GetRoot();
	if(m_pTree->GetItemLParam(m_hThisTree,&param) && param){
		m_pTree->DeleteItem(m_hThisTree);
		ParamStruct* ps = (ParamStruct*)param;
		ARootPage* page = (ARootPage*)ps->pv1;
		delete ps;
		//delete page;
	}else{
		assert(0 && "ARootPage::RemoveRoot()");
	}
}

void ARootPage::InitDataTrees()
{
	//const char* table[HTH_NULL] = {"��Ϸ","����ָ","״̬","��ͼ","����","����"};

	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = m_hThisTree;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT|TVIF_PARAM;

	struct{
		int type1;
		int type2;
		const char* str;
	}tmp[] = 
	{
// 		{HTH_ROM,		ParamStruct::PS_PAGE_ROM,		"��Ϸ"},
 		{HTH_CHEAT,		ParamStruct::PS_PAGE_CHEAT,		"����ָ"},
// 		{HTH_STATE,		ParamStruct::PS_PAGE_STATE,		"״̬"},
// 		{HTH_MAP,		ParamStruct::PS_PAGE_MAP,		"��ͼ"},
// 		{HTH_NSF,		ParamStruct::PS_PAGE_NSF,		"����"},
// 		{HTH_ATTACH,	ParamStruct::PS_PAGE_ATTACH,	"����"}
	};

	for(int i=0; i<sizeof(tmp)/sizeof(*tmp); i++){
		tvis.itemex.pszText = (LPSTR)tmp[i].str;
		tvis.itemex.lParam = (LPARAM)new ParamStruct(tmp[i].type2,this);
		m_hTreeItems[tmp[i].type1] = m_pTree->InsertItem(&tvis);
	}
}

void ARootPage::RemoveDataTrees()
{
	for(int i=0; i<HTH_NULL; i++){
		LPARAM param;
		m_pTree->GetItemLParam(m_hTreeItems[i],&param);
		ParamStruct* ps = (ParamStruct*)param;
		assert(ps->pv2 == NULL);
		delete ps;
	}
}

BOOL ARootPage::OpenDatabase(ATreeView* pTree,const char* filename)
{
	strncpy(m_filename,filename,sizeof(m_filename));
	m_pTree = pTree;

	if(!m_Table->Open(filename))
		return FALSE;

	//m_pTree->SendMessage(TVM_SETIMAGELIST,TVSIL_NORMAL,(LPARAM)m_ImageList);

	string name;
	ARom::SplitFileName(filename,0,0,&name,0);

	this->InsertRoot(name.c_str());	//Ϊÿһ����Ϸ���ݿ����һ���µĸ��ڵ�
	this->InitDataTrees();		//Ϊ��Ϸ��ʼ�����ݿ���ڵ�

	//this->BuildRomTree();		//ȡ�ø�ROM����
	this->BuildCheatTree();
	//this->BuildStateTree();


	return TRUE;
}

BOOL ARootPage::CloseDatabase()
{
//	CleanRomTree();
	CleanCheatTree();

	m_Table->Close();

	RemoveDataTrees();

	RemoveRoot();

	return TRUE;
}

AMainWindow::AMainWindow():
	m_CurrentCheatEntry(0),
	m_bDragging(FALSE)
{
	m_TreeView = new ATreeView;
	this->SetParent(NULL);
	::CreateDialog(NULL,MAKEINTRESOURCE(IDD_MAIN_WINDOW),NULL,(DLGPROC)GetWindowThunk());
}
AMainWindow::~AMainWindow()
{

	delete m_TreeView;
}

bool AMainWindow::OpenDatabaseTmp(const char* name)
{
	ARootPage* p = new ARootPage;
	return p->OpenDatabase(m_TreeView,name)!=0;
}

void AMainWindow::MakeRomInfo(ARomMap* rom_map)
{
	ShowPage(ParamStruct::PS_ROM);
	m_RomInfoDlg->ShowRomInfo(m_MenuRootPage->GetRom(),rom_map);
}
void AMainWindow::MakeCheatInfo(ACheatEntry* pEntry,void (__cdecl* SetModify)(bool bModified))
{
	ShowPage(ParamStruct::PS_CHEAT_ITEM);
	m_CheatInfoDlg->ShowCheatInfo(pEntry,SetModify);
}

void AMainWindow::MakeStateInfo(AState* state,list_s* ls)
{
	ShowPage(ParamStruct::PS_STATE_FILE);
	m_StateInfoDlg->ShowStateInfo(state,ls);
}

void AMainWindow::InitTreeViewStyles()
{
	m_TreeView->SetStyle(m_TreeView->GetStyle()|TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT);
	m_TreeView->SetStyle(m_TreeView->GetStyle()|TVS_EDITLABELS);
}


bool AMainWindow::GetFileName(HWND hwnd,char* title, char* filter,char* buffer)
{
		OPENFILENAME ofn = {0};
		*buffer = 0;
		ofn.lStructSize = sizeof(ofn);
		ofn.Flags = OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
		ofn.hwndOwner = hwnd;
		ofn.lpstrFilter = filter;
		ofn.lpstrFile = &buffer[0];
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = title;
		return (bool)(GetOpenFileName(&ofn)>0);
}

bool AMainWindow::GetSaveFile(HWND hwnd,char* title, char* filter,char* buffer,char* defext)
{
	OPENFILENAME ofn = {0};
	*buffer = 0;
	ofn.lStructSize = sizeof(ofn);
	ofn.Flags = OFN_ENABLESIZING|OFN_EXPLORER|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = &buffer[0];
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = title;
	ofn.lpstrDefExt = defext;
	return (bool)(::GetSaveFileName(&ofn)>0);
}

HTREEITEM AMainWindow::GetCheatFileNodeOfCheatOrCheatNode(HTREEITEM hItem)
{
	ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(hItem);
	ACheatFile* file = 0;
	assert(ps->type==ParamStruct::PS_CHEAT_ITEM 
		|| ps->type==ParamStruct::PS_CHEAT_DIR 
		|| ps->type==ParamStruct::PS_CHEAT);

	int type = ps->type;
	ParamStruct* aps = 0;
	while(type != ParamStruct::PS_CHEAT){
		hItem = m_TreeView->GetParent(hItem);
		aps = (ParamStruct*)m_TreeView->GetItemLParam(hItem);
		type = aps->type;
	}
	return hItem;
}

BOOL AMainWindow::MoveCheatBetweenNodes(HTREEITEM hFrom,HTREEITEM hTo)
{
	ParamStruct* psFrom = (ParamStruct*)m_TreeView->GetItemLParam(hFrom);
	ParamStruct* psTo   = (ParamStruct*)m_TreeView->GetItemLParam(hTo);

	if(psFrom->type!=ParamStruct::PS_CHEAT_DIR && psFrom->type!=ParamStruct::PS_CHEAT_ITEM)
		throw "ֻ�� ����ָ/����ָĿ¼ ���ܹ��ƶ�!";

	if(psTo->type!=ParamStruct::PS_CHEAT_DIR && psTo->type!=ParamStruct::PS_CHEAT_ITEM && psTo->type!=ParamStruct::PS_CHEAT)
		throw "ֻ�� ����ָ/����ָĿ¼/����ָ�ļ� �ܹ���Ϊ�ƶ�����Ŀ��!";
	
	if(psFrom == psTo)
		throw "��ͬĿ�겻��Ҫ�����ƶ�!(����ʾ�Ƿ��ȡ��?)";

	HTREEITEM hfileToNode = GetCheatFileNodeOfCheatOrCheatNode(hTo);
	HTREEITEM hfileFromNode = GetCheatFileNodeOfCheatOrCheatNode(hFrom);

	if(hfileFromNode != hfileToNode)
		throw "��Ϊ�Ҳ�֪�����ļ��ƶ�����ָ����ʲô����, ������ȡ����~";

	ARootPage* page = (ARootPage*)psFrom->pv1;
	ACheatEntry* pEntryFrom = (ACheatEntry*)psFrom->pv2;
	ACheatEntry* pEntryTo   = (ACheatEntry*)psTo->pv2;
	int typef = psFrom->type == ParamStruct::PS_CHEAT_DIR;
	int typet = psTo->type   == ParamStruct::PS_CHEAT_DIR || psTo->type==ParamStruct::PS_CHEAT;

	HTREEITEM hInserted = 0;
	const char* pszInertText=0;
	TVINSERTSTRUCT tvi = {0};

	page->DelCheatEntryPointers(hFrom);
	m_TreeView->DeleteItem(hFrom);//ûɾ��ps

	pszInertText = typef?pEntryFrom->node.name.c_str():pEntryFrom->item.name.c_str();

	tvi.itemex.mask = TVIF_IMAGE|TVIF_TEXT|TVIF_PARAM;
	tvi.itemex.iImage = 0;
	tvi.itemex.pszText = (LPTSTR)pszInertText;
	tvi.item.lParam = (LPARAM)psFrom;

	//����&&||��д���ǲ����е���? - ���ļ��ڵ㵱��Ŀ¼
	if(typet && (psTo->type==ParamStruct::PS_CHEAT || MessageBox(
			"��Ҫ�ƶ���Ŀ¼���滹�Ǻ���?\n\n"
			"[��]\t�ƶ�������\n"
			"[��]\t�ƶ�������\n",
			"Ŀ��ΪĿ¼",
			MB_YESNO) == IDYES))
	{
		tvi.hParent = hTo;
		tvi.hInsertAfter = TVI_FIRST;
	}else{
		tvi.hParent = m_TreeView->GetParent(hTo);
		tvi.hInsertAfter = hTo;
	}

	hInserted = m_TreeView->InsertItem(&tvi);

	page->AddCheatEntryPointers(pEntryFrom,hInserted);

	if(typef){
		//ACheatEntry* dirchild = pEntryFrom->child;
		page->ListCheatDir(hInserted,pEntryFrom->child);
	}
	return TRUE;
}

bool AMainWindow::GetMenuTreeItem(HTREEITEM* phitem)
{
	TVHITTESTINFO hti = {0};
	POINT pt;

	::GetCursorPos(&hti.pt);
	pt = hti.pt;
	::ScreenToClient(m_TreeView->GetHwnd(),&hti.pt);

	m_TreeView->HitTest(&hti);

	if(hti.flags & TVHT_ONITEM){
		if(phitem) *phitem = hti.hItem;

		LPARAM param;
		if(m_TreeView->GetItemLParam(hti.hItem,&param)){
			if(param){
				ParamStruct* ps = (ParamStruct*)param;
				this->m_MenuPS = ps;
				this->m_MenuhItem = hti.hItem;
				this->m_MenuRootPage = (ARootPage*)ps->pv1;
				return true;
			}
		}
	}else{
		if(phitem) *phitem = 0;
	}
	return false;
}

void AMainWindow::ShowMenuFor(int type)
{
	HMENU hMenu = NULL;
	POINT pt;
	switch(type)
	{
	case ParamStruct::PS_ROOTPAGE:
		hMenu = ::GetSubMenu(::LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_DATABASE)),0);
		break;

	//��Ϸ�˵�
	case ParamStruct::PS_PAGE_ROM:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_ROM)),0);
		break;
	case ParamStruct::PS_ROM:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_ROM)),1);
		break;

	//����ָ�˵�
	case ParamStruct::PS_PAGE_CHEAT:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),0);
		break;
	case ParamStruct::PS_CHEAT:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),1);
		break;
	case ParamStruct::PS_CHEAT_DIR:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),2);
		break;
	case ParamStruct::PS_CHEAT_ITEM:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),3);
		break;
	}
	if(hMenu){
		::GetCursorPos(&pt);
		::TrackPopupMenu(hMenu,TPM_LEFTALIGN,pt.x,pt.y,0,this->GetHwnd(),NULL);
	}
}

void AMainWindow::ShowPage(int type)
{
	struct{
		AWindowBase* win;
		int type;
	}a[] = 
	{
		{(AWindowBase*)m_RomInfoDlg,ParamStruct::PS_ROM},
		{(AMainWindow*)m_CheatInfoDlg,ParamStruct::PS_CHEAT_ITEM},
		{(AMainWindow*)m_StateInfoDlg,ParamStruct::PS_STATE_FILE}
	};

	for(int i=0; i<sizeof(a)/sizeof(*a); i++){
		if(a[i].type == type) a[i].win->ShowWindow(SW_SHOW);
		else a[i].win->ShowWindow(SW_HIDE);
	}
}

// bool AMainWindow::HandleMenuForRootPage(int id,INT_PTR& result)
// {
// 	result = 0;
// 	switch(id)
// 	{
// 	case IDM_DATABASE_CLOSEDATABASE:
// 		{
// 			m_MenuRootPage->CloseDatabase();
// 			return true;
// 		}
// 	}
// 	return false;
// }
// 

bool AMainWindow::CloseAllDatabase()
{
	bool b = true;
	HTREEITEM hRoot = m_TreeView->GetRoot();
	while(hRoot){
		ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(hRoot);
		ARootPage* page = (ARootPage*)ps->pv1;
		//if(!page->CloseDatabase()) b=false;
		for(std::map<HTREEITEM,bool>::iterator it=page->m_bCheatNeedSaving.begin();
			it!=page->m_bCheatNeedSaving.end();
			it++)
		{
			if((*it).second){
				ParamStruct* aps = (ParamStruct*)m_TreeView->GetItemLParam((*it).first);
				ACheatFile* cf = (ACheatFile*)aps->pv3;
				if(MessageBox(cf->name.c_str(),"�����޸�?",MB_YESNO)==IDYES){
					m_MenuPS = aps;
					m_MenuRootPage = (ARootPage*)aps->pv1;
					m_MenuhItem = it->first;
					if(SendMessage(WM_COMMAND,MAKEWPARAM(IDM_CHEATFILE_SAVE_UPDATE,0),0)){
						(*it).second = false;
					}else{
						b=false;
					}
				}else{
					it->second = false;
				}
			}
		}
		if(b) page->CloseDatabase();


		hRoot = m_TreeView->GetNextItem(hRoot,TVGN_NEXT);
	}
	return b;
}
//��Щ�������Ը�����ARootPage. ����, ��ôд����������
bool AMainWindow::HandleMenuForCheat(int id,INT_PTR& result)
{
	//bool handled = false;
	result = 0;
	switch(id)
	{
	case IDM_CHEATROOT_ADDNEW:
		{
			char file[260];
			if(GetFileName(this->GetHwnd(),"ѡ�����ָXML�ļ�","XML(*.xml)\0*.xml",file))
			{
				int row_id;
				ACheatHeader* hdr = NULL;
				if(m_MenuRootPage->GetCheat()->Add(file,&hdr,&row_id)){
					m_MenuRootPage->AddCheat(new ACheatMap(hdr,row_id));
				}
				return true;
			}
			return true;
		}
	case IDM_CHEATROOT_ADDMEMFILE:
		{
			ACheatFileInfoDlg dlg(this,0);
			int code = dlg.GetDlgCode();
			if(code==ACheatFileInfoDlg::RET_CANCEL||code==ACheatFileInfoDlg::RET_CLOSE){
				result = FALSE;
				return true;
			}
			ACheatFile* file = dlg.GetFile();
			ACheatHeader* hdr = 0;
			m_MenuRootPage->GetCheat()->MakeCheatHeader(&hdr,(char*)file->name.c_str(),(char*)file->desc.c_str());
			ACheatMap* pmap = new ACheatMap(hdr,-1);
			m_MenuRootPage->AddCheat(pmap,file);
			return true;
		}
	case IDM_CHEATFILE_DELETE:
		{
			ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(m_MenuhItem);
			ARootPage* pPage = (ARootPage*)ps->pv1;
			ACheatMap* pMap = (ACheatMap*)ps->pv2;
			ACheatFile* pFile = (ACheatFile*)ps->pv3;

			stringstream ss;
			ss<<"ȷ��Ҫɾ������ָ�ļ� "<<pFile->name<<" ?";
			if(MessageBox(string(ss.str()).c_str(),"",MB_YESNO|MB_DEFBUTTON2)==IDNO){
				return false;
			}
			m_MenuRootPage->DeleteCheatNode(m_MenuhItem);

			delete pFile;
			delete pMap->phdr;
			pPage->GetCheat()->Delete(pMap->idx);
			delete pMap;			
			m_TreeView->DeleteItem(m_MenuhItem);
			m_MenuRootPage->CheatSetModify(m_MenuhItem,false);
			return true;
		}
	case IDM_CHEATFILE_DUMPTOXML:
		{
			char file[260];
			BOOL bret = FALSE;
			if(GetSaveFile(this->GetHwnd(),"ѡ�񱣴��ļ�","XML(*.xml)\0*.xml",file,".xml")){
				bret = m_MenuRootPage->DumpCheatsToXml(m_MenuhItem,(string)file);
				MessageBox(bret?"�����ɹ�!":"����ʧ��!");
			}
			result = bret;
			return true;
		}
	case IDM_CHEATFILE_SAVE_UPDATE:
		{
			ACheatMap* cm = (ACheatMap*)m_MenuPS->pv2;
			ACheat* pcheat = m_MenuRootPage->GetCheat();
			string str;
			BOOL bret = m_MenuRootPage->DumpCheatsToXml(m_MenuhItem,&str);
			if(bret == FALSE){
				MessageBox("����ʧ��~");
				result = FALSE;
				return true;
			}

			if(cm->idx == -1){
				int row_id;
				if(pcheat->Add(cm->phdr,str.c_str(),str.size(),&row_id)){
					cm->idx = row_id;
					MessageBox("��ӳɹ�~");
					m_MenuRootPage->CheatSetModify(m_MenuhItem,false);
					result = TRUE;
				}else{
					MessageBox("ʧ��!");
					result = FALSE;
				}
				return true;
			}else{
				if(pcheat->Update(cm->phdr,str.c_str(),str.size(),cm->idx)){
					MessageBox("���³ɹ�~");
					m_MenuRootPage->CheatSetModify(m_MenuhItem,false);
					result = TRUE;
				}else{
					MessageBox("����ʧ��~");
					result = FALSE;
				}
				return true;
			}
		}
	case IDM_CHEATFILE_VIEWFILE:
		{
			ARootPage* page = (ARootPage*)m_MenuPS->pv1;
			ACheatFile* file = (ACheatFile*)m_MenuPS->pv3;
			string oldname = file->name;
			ACheatFileInfoDlg dlg(this,file);
			int code=dlg.GetDlgCode();
			if(code==ACheatFileInfoDlg::RET_OK){
				if(oldname != file->name){
					m_MenuRootPage->UpdateTreeItemText(m_MenuhItem,file->name.c_str());
					// 					ACheatMap* pmap = (ACheatMap*)m_MenuPS->pv2;
					// 					delete pmap->phdr->comment;
					// 					pmap->phdr->comment = new char[file->name.size()+1];
					// 					memcpy(pmap->phdr->comment,file->name.c_str(),file->name.size()+1);
				}
				page->CheatSetModify(m_MenuhItem,true);
			}

			result = dlg.GetDlgCode()==ACheatFileInfoDlg::RET_OK;
			return true;
		}
	case IDM_CHEATFILE_ADDDIR:
	case IDM_CHEATDIR_ADDDIR:
		{
			HWND hEdit = 0;
			HTREEITEM hTVItem = 0;
			ACheatEntry* pEntry = new ACheatEntry;
			const char* new_dir = "��Ŀ¼";

			m_TreeView->SetFocus();	//must

			hTVItem = m_MenuRootPage->AddSubTreeItem(
				m_MenuhItem,(LPTSTR)new_dir,
				(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_DIR,m_MenuRootPage,pEntry));
			m_TreeView->Expand(m_MenuhItem,TVE_EXPAND);
			hEdit = m_TreeView->EditLabel(hTVItem);
			::SendMessage(hEdit,EM_SETSEL,0,-1);

			pEntry->type = ACheatEntry::TYPE_NODE;
			pEntry->node.name = new_dir;

			m_MenuRootPage->AddCheatEntryPointers(pEntry,hTVItem);

			m_MenuRootPage->CheatSetModify(m_MenuhItem,true);
			return true;
		}
	case IDM_CHEATFILE_ADDCHEAT:
	case IDM_CHEATDIR_ADDCHEAT:
		{
			HWND hEdit = 0;
			HTREEITEM hTVItem = 0;
			ACheatEntry* pEntry = new ACheatEntry;
			const char* new_cheat = "�½���ָ";

			m_TreeView->SetFocus();	//must
			hTVItem = m_MenuRootPage->AddSubTreeItem(
				m_MenuhItem,(LPTSTR)new_cheat,
				(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_ITEM,m_MenuRootPage,pEntry));
			m_TreeView->Expand(m_MenuhItem,TVE_EXPAND);
			hEdit = m_TreeView->EditLabel(hTVItem);
			::SendMessage(hEdit,EM_SETSEL,0,-1);

			pEntry->type = ACheatEntry::TYPE_CHEAT;
			pEntry->item.name = new_cheat;

			m_MenuRootPage->AddCheatEntryPointers(pEntry,hTVItem);


			m_MenuRootPage->CheatSetModify(m_MenuhItem,true);
			return true;
		}
	case IDM_CHEATDIR_RENAME:
	case IDM_CHEATITEM_RENAME:
		{
			m_TreeView->SetFocus();
			m_TreeView->EditLabel(m_MenuhItem);
			return true;
		}
	case IDM_CHEATITEM_DELETE:
		{
			ACheatEntry* pEntry = (ACheatEntry*)m_MenuPS->pv2;
			assert(pEntry->type == ACheatEntry::TYPE_CHEAT);

			stringstream ss;
			ss<<"ȷ��Ҫɾ������ָ "<<pEntry->item.name<<" ?";
			if(MessageBox(string(ss.str()).c_str(),"",MB_YESNO|MB_DEFBUTTON2)==IDNO){
				return false;
			}

			if(m_CurrentCheatEntry == pEntry){
				MakeCheatInfo(0);
				m_CurrentCheatEntry=0;
			}


			m_MenuRootPage->CheatSetModify(m_MenuhItem,true);
			m_MenuRootPage->DeleteCheatItem(m_MenuhItem);
			return true;
		}
	case IDM_CHEATDIR_DELETE:
		{
			ACheatEntry* pEntry=(ACheatEntry*)m_MenuPS->pv2;
			assert(pEntry->type == ACheatEntry::TYPE_NODE);

			stringstream ss;
			ss<<"ȷ��Ҫɾ������ָĿ¼(������Ŀ¼��) "<<pEntry->node.name<<" ?";
			if(MessageBox(string(ss.str()).c_str(),"",MB_YESNO|MB_DEFBUTTON2)==IDNO){
				return false;
			}

			m_MenuRootPage->SetCurrentCheatEntry(m_CurrentCheatEntry);
			m_MenuRootPage->CheatSetModify(m_MenuhItem,true);
			m_MenuRootPage->DeleteCheatNode(m_MenuhItem);
			if(m_CurrentCheatEntry!=0 &&
				m_MenuRootPage->GetCurrentCheatEntry()==0
				){
					MakeCheatInfo(0);
					m_CurrentCheatEntry=0;
			}
			return true;
		}
	}
	return false;
}

bool AMainWindow::HandleMenuForRom(int id,INT_PTR& result)
{
	result = 0;

	switch(id)
	{
	case IDM_ROMROOT_ADDROM:
		{
			char file[260];
			char splash[260];

			if(GetFileName(this->GetHwnd(),
				"ѡ��ROM�ļ�",
				"ROM(*.nes,*.zip,*.7z,*.rar)\0*.nes;*.zip;*.7z;*.rar"
				"\0All Files(*.*)\0*.*",file)
				&& GetFileName(this->GetHwnd(),"ѡ����Ϸ����","PNG(*.png)\0*.png",splash))
			{
				int row_id;
				ARomHeader* hdr = NULL;
				try{
					if(m_MenuRootPage->GetRom()->Add(file,splash,&row_id,&hdr)){
						m_MenuRootPage->AddRom(new ARomMap(hdr,row_id));
					}
				}
				catch(const char* e){
					MessageBox(e,0,MB_ICONEXCLAMATION);
				}
				return true;
			}
			return true;
		}
	case IDM_ROMROOT_SAVEALL:
		{
			char disp[MAX_PATH]={0};
			BROWSEINFO bi = {0};
			PIDLIST_ABSOLUTE pidl=0;
			bi.hwndOwner = this->GetHwnd();
			bi.ulFlags = BIF_USENEWUI|BIF_RETURNONLYFSDIRS;
			bi.pszDisplayName = disp;//NULL��������?
			bi.lpszTitle = "ѡ�񱣴浽���ļ���";
			if((pidl=SHBrowseForFolder(&bi))){
				if(SHGetPathFromIDList(pidl,disp)){
					PathAddBackslash(disp);
					try{
						result = m_MenuRootPage->GetRom()->SaveAllRoms(disp);
						result = TRUE;
						return true;
					}
					catch(string e){
						MessageBox(e.c_str(),0,MB_ICONERROR);
						result = FALSE;
					}
				}
			}
			return true;
		}
	case IDM_ROMFILE_DELETE:
		{
			ARomMap* rom_map = (ARomMap*)m_MenuPS->pv2;
			stringstream ss;
			ss<<"ȷʵҪɾ��ROM�ļ� "<<rom_map->phdr->comment<< " ?";
			if(MessageBox(string(ss.str()).c_str(),"��ʾ",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) == IDNO){
				return false;
			}
			if(m_MenuRootPage->GetRom()->Delete(rom_map->idx)){
				m_TreeView->DeleteItem(m_MenuhItem);
			}
			return true;
		}
	case IDM_ROMFILE_SAVEAS:
		{
			ARomMap* rm = reinterpret_cast<ARomMap*>(m_MenuPS->pv2);
			char file[MAX_PATH] = {0};
			
			struct{
				int type;
				const char* ext;
				const char* filter;
			}exts[] = {
				{ARom::TYPE_NES,".nes","NES �ļ�(*.nes)\0*.nes"},
				{ARom::TYPE_ZIP,".zip","ZIP �ļ�(*.zip)\0*.zip"},
				{ARom::TYPE_7Z, ".7z","7Zip �ļ�(*.7z)\0*.7z"},
				{ARom::TYPE_RAR,".rar","Rar �ļ�(*.rar)\0*.rar"},
				{ARom::TYPE_UNK,"","�����ļ�(*.*)\0*.*"}
			};
			
			const char* defExtention = 0;
			const char* defFilter = 0;
			int i;
			for(i=0; i<sizeof(exts)/sizeof(*exts); i++){
				if(rm->phdr->type == exts[i].type){
					defExtention = exts[i].ext;
					defFilter = exts[i].filter;
					break;
				}
			}
			if(i==sizeof(exts)/sizeof(*exts)){
				defExtention = 0;
				defFilter = 0;
			}
			strncpy(file,rm->phdr->comment,sizeof(file));
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof(ofn);
			ofn.Flags = OFN_OVERWRITEPROMPT|OFN_CREATEPROMPT|OFN_ENABLESIZING|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST;
			ofn.hInstance = GetModuleHandle(0);
			ofn.hwndOwner = this->GetHwnd();
			ofn.lpstrDefExt = defExtention;
			ofn.lpstrFile = file;
			ofn.lpstrFilter = defFilter;
			ofn.lpstrTitle = "�����ѡ���������ļ���";
			ofn.nMaxFile = MAX_PATH;
			if(::GetSaveFileName(&ofn)){
				try{
					if(m_MenuRootPage->GetRom()->SaveRomAsFile(rm->idx,file)){
						MessageBox(file,"����ɹ�!",MB_ICONINFORMATION);
						return true;
					}else{
						MessageBox(file,"����ʧ��!",MB_ICONERROR);
						return false;
					}
				}
				catch(string s){
					MessageBox(s.c_str(),0,MB_ICONERROR);
					return false;
				}
			}

			return false;
		}
	}

	return false;
}

INT_PTR AMainWindow::HandleDblClick()
{
	switch(m_MenuPS->type)
	{
	case ParamStruct::PS_ROM:
		{
			ARomMap* rom = (ARomMap*)m_MenuPS->pv2;
			MakeRomInfo(rom);
			return SetDlgResult(0);
		}
	case ParamStruct::PS_CHEAT_ITEM:
		{
			ACheatEntry* entry = (ACheatEntry*)m_MenuPS->pv2;
			m_CurrentCheatEntry = entry;
			m_CurrentCheatEntryTreeItem = m_MenuhItem;
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111
			m_MenuRootPage->m_CurrentCheatEntryTreeItem = GetCheatFileNodeOfCheatOrCheatNode(m_MenuhItem);
			MakeCheatInfo(entry,(void (__cdecl*)(bool bModified))m_MenuRootPage->m_TheFuckThunk.Cdeclcall(m_MenuRootPage,&ARootPage::CheatSetModifyCallback));
			return SetDlgResult(0);
		}
	case ParamStruct::PS_STATE_FILE:
		{
			ARootPage* page = m_MenuRootPage;
			list_s* p = (list_s*)m_MenuPS->pv2;
			MakeStateInfo(page->GetState(),p);
			return SetDlgResult(0);
		}
	}
	return 0;
}

void AMainWindow::ResizePages(int width,int height)
{
	m_TreeView->SetWindowPos(5,5,200,height-5-5);
	m_RomInfoDlg->SetWindowPos(210,5,width-210-5,height-5-5);
	m_CheatInfoDlg->SetWindowPos(210,5,width-210-5,height-5-5);
	m_StateInfoDlg->SetWindowPos(210,5,width-210-5,height-5-5);
}

INT_PTR AMainWindow::OnSize(int width,int height)
{
	ResizePages(width,height);
	return 0;
}

INT_PTR AMainWindow::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AMainWindow::OnClose()
{
#ifndef __FCEUX__
	if(!CloseAllDatabase()){
		return SetDlgResult(FALSE);
	}
	this->DestroyWindow();
	::PostQuitMessage(0);
#else

	ShowWindow(SW_HIDE);
#endif
	return SetDlgResult(TRUE);
}

INT_PTR AMainWindow::OnNotify(LPNMHDR phdr)
{
	if(phdr->idFrom == m_TreeView->GetCtrlID()){
		if(phdr->code == NM_DBLCLK){
			if(!GetMenuTreeItem()) return 0;
			return HandleDblClick();
		}else if(phdr->code == NM_RCLICK){
			HTREEITEM hItem;
			if(!GetMenuTreeItem(&hItem)){
				//���û���Ҽ��˵�,��ǰѡ����...������ȥ, �ܲ�ˬ
				if(hItem) m_TreeView->SelectItem(hItem);
				else{
					POINT pt; GetCursorPos(&pt);
					HMENU hMenu = GetSubMenu(LoadMenu(theApp->GetAppInstance(),MAKEINTRESOURCE(IDR_MENU_MAIN)),0);
					TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,this->GetHwnd(),0);
				}
				return 0;
			}
			//ͬ��
			m_TreeView->SelectItem(m_MenuhItem);
			ShowMenuFor(m_MenuPS->type);
			return SetDlgResult(0);
		}else if(phdr->code == TVN_BEGINLABELEDIT){
			LPNMTVDISPINFO pdi = (LPNMTVDISPINFO)phdr;
			LPARAM param = m_TreeView->GetItemLParam(pdi->item.hItem);
			ParamStruct* ps = (ParamStruct*)param;
			switch(ps->type)
			{
			case ParamStruct::PS_CHEAT:
				return SetDlgResult(TRUE);

			case ParamStruct::PS_CHEAT_ITEM:
			case ParamStruct::PS_CHEAT_DIR:
				break;

			default:
				return SetDlgResult(TRUE);
			}
			return SetDlgResult(FALSE);
		}else if(phdr->code == TVN_ENDLABELEDIT){
			LPNMTVDISPINFO pdi = (LPNMTVDISPINFO)phdr;
			ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(pdi->item.hItem);
			switch(ps->type)
			{
			case ParamStruct::PS_CHEAT_DIR:
				{
					ACheatEntry* pEntry = (ACheatEntry*)ps->pv2;
					assert(pEntry->type == ACheatEntry::TYPE_NODE);
					if(pdi->item.pszText != NULL){
						pEntry->node.name = pdi->item.pszText;
						return SetDlgResult(TRUE);
					}
					return 0;
				}
			case ParamStruct::PS_CHEAT_ITEM:
				{
					ACheatEntry* pEntry = (ACheatEntry*)ps->pv2;
					assert(pEntry->type == ACheatEntry::TYPE_CHEAT);
					if(pdi->item.pszText != NULL){
						pEntry->item.name = pdi->item.pszText;

						//�����ǰ�Ľ���ָ���ڱ��鿴,�����
						if(m_CurrentCheatEntry == pEntry){
							m_CheatInfoDlg->ShowCheatInfo(m_CurrentCheatEntry,(void (__cdecl*)(bool bModified))m_MenuRootPage->m_TheFuckThunk.Cdeclcall(m_MenuRootPage,&ARootPage::CheatSetModifyCallback));
						}

						return SetDlgResult(TRUE);
					}
					return 0;
				}
			}//switch
			return 0;
		}else if(phdr->code == TVN_BEGINDRAG){
			HIMAGELIST hImg = 0;
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)phdr;
			HTREEITEM hItem = pnmtv->itemNew.hItem;
			m_TreeView->SelectItem(hItem);
			ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(hItem);
			if(ps->type!=ParamStruct::PS_CHEAT_DIR && ps->type!=ParamStruct::PS_CHEAT_ITEM){
				return 0;
			}
			hImg = TreeView_CreateDragImage(m_TreeView->GetHwnd(),pnmtv->itemNew.hItem);
			ImageList_BeginDrag(hImg,0,0,0);
			ImageList_DragEnter(m_TreeView->GetHwnd(),pnmtv->ptDrag.x,pnmtv->ptDrag.y);
			//ShowCursor(FALSE);
			SetCapture(this->GetHwnd());
			m_TreeView->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,0);
			m_hDraggingItem = pnmtv->itemNew.hItem;
			m_bDragging = TRUE;
			return SetDlgResult(0);
		}else if(phdr->code == NM_CUSTOMDRAW){
			NMTVCUSTOMDRAW* ptvcd = (NMTVCUSTOMDRAW*)phdr;
			switch(ptvcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				return SetDlgResult(CDRF_NOTIFYITEMDRAW);
			case CDDS_ITEMPREPAINT:
				{
					ParamStruct* ps = (ParamStruct*)ptvcd->nmcd.lItemlParam;
					COLORREF c1,c2;
					if(!ps) return SetDlgResult(CDRF_DODEFAULT);
					switch(ps->type)
					{
					case ParamStruct::PS_CHEAT:
						{
							ARootPage* page = (ARootPage*)ps->pv1;
							if(page->m_bCheatNeedSaving[(HTREEITEM)ptvcd->nmcd.dwItemSpec]){
								c1 = RGB(255,0,0);
								c2 = RGB(255,255,255);
								//return SetDlgResult(CDRF_NEWFONT);
								break;
							}else{
								return SetDlgResult(CDRF_DODEFAULT);
							}
						}
//						return SetDlgResult(CDRF_DODEFAULT);
// 						c1 = RGB(255,128,64);
// 						c2 = RGB(255,255,255);
// 						break;
 						
					case ParamStruct::PS_CHEAT_DIR:
						c1 = RGB(255,255,255);
						c2 = RGB(0,0,255);
						break;
					case  ParamStruct::PS_CHEAT_ITEM:
						return SetDlgResult(CDRF_DODEFAULT);
// 						c1 = RGB(0,255,255);
// 						c2 = RGB(255,255,255);
// 						break;
					default:
						return SetDlgResult(CDRF_DODEFAULT);
					}
					ptvcd->clrText   = c1;
					ptvcd->clrTextBk = c2;

					return SetDlgResult(CDRF_NEWFONT);
				}
			}
			return SetDlgResult(CDRF_DODEFAULT);
		}
	}
	/*return SetDlgResult(0);*/
	return 0;
}

INT_PTR AMainWindow::OnMouseMove(int key,int x,int y)
{
	if(m_bDragging){
		TVHITTESTINFO tvht = {0};
		HTREEITEM hItem = 0;
		POINT pt = {x,y};
		::ClientToScreen(this->GetHwnd(),&pt);
		::ScreenToClient(m_TreeView->GetHwnd(),&pt);
		x=pt.x;y=pt.y;
		ImageList_DragMove(x,y);
		ImageList_DragShowNolock(FALSE);
		tvht.pt.x = x;
		tvht.pt.y = y;
		if((hItem=m_TreeView->HitTest(&tvht)) && tvht.flags&TVHT_ONITEM){
			m_TreeView->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,(LPARAM)hItem);
			ImageList_DragShowNolock(TRUE);
		}else{
			m_TreeView->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,0);
		}
		return 0;
	}
	return 0;
}

INT_PTR AMainWindow::OnLButtonUp(int key,int x,int y)
{
	if(m_bDragging){
		HTREEITEM hItem = 0;
		ImageList_DragLeave(m_TreeView->GetHwnd());
		ImageList_EndDrag();
		hItem = (HTREEITEM)m_TreeView->SendMessage(TVM_GETNEXTITEM,TVGN_DROPHILITE,0);
		m_TreeView->SendMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)hItem);
		m_TreeView->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,0);
		ReleaseCapture();
		ShowCursor(TRUE);

		if(hItem){
			try{
				MoveCheatBetweenNodes(m_hDraggingItem,hItem);
				//m_SetModify(true);
				HTREEITEM hFile = GetCheatFileNodeOfCheatOrCheatNode(m_hDraggingItem);//���ڵ�ǰ�ļ�,Ŀǰ��֧��
				ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(hFile);
				ARootPage* page = (ARootPage*)ps->pv1;
				page->m_bCheatNeedSaving[hFile] = true;
			}
			catch(const char* err){
				MessageBox(err,0,MB_ICONINFORMATION);
			}
		}

		m_bDragging = FALSE;
	}
	return 0;
}

INT_PTR AMainWindow::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(!codeNotify && !hWndCtrl){//Menus
		INT_PTR re;
		if(HandleMenuForCheat(ctrlID,re))		return SetDlgResult(re);
		if(HandleMenuForRom(ctrlID,re))			return SetDlgResult(re);
		//if(HandleMenuForRootPage(ctrlID,re))	return SetDlgResult(re);

		switch(ctrlID)
		{
		case IDM_MAIN_OPEN_DB:
			{
				const int bufsize = 1<<20;
				char* files = new char[bufsize];
				memset(files,0,bufsize);
				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				ofn.Flags = OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER|
					OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR|OFN_NONETWORKBUTTON|OFN_PATHMUSTEXIST;
				ofn.hInstance = theApp->GetAppInstance();
				ofn.hwndOwner = this->GetHwnd();
				ofn.lpstrFile = files;
				ofn.nMaxFile = bufsize;
				ofn.lpstrFilter = "NES���ݿ��ļ�(*.ndb)\0*.ndb\0�����ļ�(*.*)\0*.*";

				if(::GetOpenFileName(&ofn)){
					std::vector<string> filelist;
					GetFileListFromOFN(&filelist,files);

					for(std::vector<string>::iterator it=filelist.begin();
						it != filelist.end();
						it ++)
					{
						ARootPage* page = new ARootPage;
						page->OpenDatabase(m_TreeView,it->c_str());
					}
				}

				delete[] files;
				return SetDlgResult(0);
			}
		case IDM_MAIN_NEW_DB:
			{
				char file[MAX_PATH]={0};

				if(GetSaveFile(this->GetHwnd(),
					"ѡ�����ݿⱣ���ļ���",
					"Nes DataBase File(*.ndb)\0*.ndb",
					file,
					".ndb"))
				{
					ARootPage* page = new ARootPage;
					page->OpenDatabase(m_TreeView,file);
					return SetDlgResult(0);
				}
				return 0;
			}
		case IDM_DATABASE_CLOSEDATABASE:
			{
				m_MenuRootPage->CloseDatabase();
				delete m_MenuRootPage;
				return 0;
			}
		}
	}
	return 0;
}

INT_PTR AMainWindow::OnNull(LPARAM lParam)
{
	ControlMessage* pcm = (ControlMessage*)lParam;
	if(!pcm) return 0;
	if(pcm->self = m_CheatInfoDlg){
		if(pcm->uMsg == WM_SETTEXT){
			ParamStruct* ps = (ParamStruct*)m_TreeView->GetItemLParam(m_CurrentCheatEntryTreeItem);
			ARootPage* page = (ARootPage*)ps->pv1;
			page->UpdateTreeItemText(m_CurrentCheatEntryTreeItem,(const char*)pcm->wParam);
			return SetDlgResult(TRUE);
		}
	}
	return 0;
}

INT_PTR AMainWindow::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;
	ShowWindow(SW_HIDE);
	m_TreeView->AttachCtrl(this,IDC_MAIN_TREE);
	m_RomInfoDlg = new ARomInfoDlg(this);
	m_CheatInfoDlg = new ACheatInfoDlg(this);
	m_StateInfoDlg = new AStateInfoDlg(this);

// 	m_RomInfoDlg->SetStyle(m_RomInfoDlg->GetStyle()&~WS_POPUP|WS_CHILD);
// 	m_CheatInfoDlg->SetStyle(m_CheatInfoDlg->GetStyle()&~WS_POPUP|WS_CHILD);
// 	::SetParent(m_RomInfoDlg->GetHwnd(),this->GetHwnd());
// 	::SetParent(m_CheatInfoDlg->GetHwnd(),this->GetHwnd());
	

	//::SetMenu(hWnd,::LoadMenu(::GetModuleHandle(0),MAKEINTRESOURCE(IDR_MENU_MAIN)));
	
	ShowPage(-1);

	this->CenterWindow(NULL);
	this->ShowWindow(SW_SHOWNORMAL);
	InitTreeViewStyles();

// 	ARootPage* page = new ARootPage;
// 	page->OpenDatabase(m_TreeView,"ndb.db");

	return TRUE;
}

// void submain()
// {
// 	theApp = new AApp;
// 	bool initArchiveSystem();
// 	initArchiveSystem();
// 
// 	ULONG_PTR gdiplusToken;
// 	Gdiplus::GdiplusStartupInput gdipInput;
// 	GdiplusStartup(&gdiplusToken,&gdipInput,NULL);
// 
// }
// 
// class XXXXXXXXXXXxx{
// public:
// 	XXXXXXXXXXXxx(){submain();}
// 	~XXXXXXXXXXXxx(){/*Gdiplus::GdiplusShutdown(gdiplusToken);*/}
// };
// static XXXXXXXXXXXxx ______XXXX;
// 
#ifndef __FCEUX__
int __stdcall WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
{
	theApp = new AApp;
	bool initArchiveSystem();
	initArchiveSystem();
// 	void InitSharedMemory();
// 	InitSharedMemory();

#ifdef _DEBUG
	AllocConsole();
	debug_out(("Program Starting...\n"));
	debug_out(("Starting Directory:%s\n",theApp->GetAppPath().c_str()));
#endif

	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdipInput;
	GdiplusStartup(&gdiplusToken,&gdipInput,NULL);


	AMainWindow main;

	MSG msg;
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return int(msg.wParam);
}
#endif

// #include "Archive.h"
// //using namespace NArchive;
// int __stdcall WinMain3( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
// {
// 	initArchiveSystem();
// 	try{
// 		MEM_FILE* mf = GetFirstNesFileFromArchive("C:\\Users\\YangTao\\Desktop\\NESTMP\\nestmp.7z");
// 		mf=mf;
// 	}
// 	catch(const char* e){
// 		e=e;
// 	}
// 	return 0;
// }

// __declspec(dllexport) void ycfkdj(void)
// {
// }
