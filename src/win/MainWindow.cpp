#include <windows.h>
#include <ShlObj.h>
#include <shlwapi.h>
#include <vector>
#include <sstream>
#include <cstdio>
using namespace std;

#include "../../res/resource.h"
#include "ctl/TreeView.h"
#include "CheatInfoDlg.h"
#include "CheatFileInfoDlg.h"
#include "../tinyxml/tinyxml.h"
#include "ctl/App.h"
#include "MainWindow.h"


AApp* theApp = new AApp;

bool AMainWindow::MakeCheatsListFromFile(const char* fn,ACheatFile** ppFile)
// reads xml from file fn
// throws const char*
{
	TiXmlDocument doc(fn);
	doc.SetCondenseWhiteSpace(false);
	if(!doc.LoadFile()) {
		throw "解析xml文件失败!";
	}

	TiXmlNode* nodeNesCheat = doc.FirstChild("NesCheat");
	if(nodeNesCheat == NULL){
		//no node NesCheat found
		throw "未找到NesCheat结点, 可能不是正确的作弊码数据库文件!";
	}

	ACheatFile* cheat = new ACheatFile;

	TiXmlNode* nodeInfo = nodeNesCheat->FirstChild("Info");
	if(nodeInfo != NULL){
		TiXmlHandle h(nodeInfo);
		TiXmlText* name = h.FirstChild("Name").FirstChild().ToText();
		if(name) cheat->name = name->Value();

		TiXmlText* author = h.FirstChild("Author").FirstChild().ToText();
		if(author) cheat->author = author->Value();

		TiXmlText* time = h.FirstChild("Time").FirstChild().ToText();
		if(time) cheat->time = time->Value();

		TiXmlText* desc = h.FirstChild("Desc").FirstChild().ToText();
		if(desc) cheat->desc = desc->Value();
	}

	TiXmlNode* nodeCheats = nodeNesCheat->FirstChild("Cheats");
	if(nodeCheats != NULL){
		ACheatEntry tmpentry;
		TravelCheatNode(&tmpentry,nodeCheats->FirstChild());
		//tmpentry只是个假的父节点
		ACheatEntry* pt = tmpentry.child;
		while(pt){
			pt->parent = 0;
			pt = pt->next;
		}
		cheat->entry = tmpentry.child;
	}

	*ppFile = cheat;
	return true;
}

void AMainWindow::TravelCheatNode(ACheatEntry* cheat,TiXmlNode* node)
// Travel TiXmlNode node, and add all its child node and the child node's siblings as the children of cheat
{
	ACheatEntry* lastChild = new ACheatEntry;
	ACheatEntry* oldlastChild=lastChild;

	if(node != NULL){
		do{
			if(strcmp(node->Value(),"Node")==0){		//---Directory
				ACheatEntry* dir = new ACheatEntry;
				dir->type = ACheatEntry::TYPE_NODE;
				dir->parent = cheat;

				const char* name = node->ToElement()->Attribute("name");

				name = name?name:"目录";
				dir->node.name = name;

				lastChild->next = dir;
				dir->prev = lastChild;
				lastChild = dir;

				TravelCheatNode(dir,node->FirstChild());
			}
			else if(strcmp(node->Value(),"Item")==0){	//---Item
				//is currently the CheatItem
				ACheatEntry* entry = new ACheatEntry;
				entry->type = ACheatEntry::TYPE_CHEAT;
				entry->parent = cheat;

				TiXmlHandle h(node);

				TiXmlText* name = h.FirstChild("Name").FirstChild().ToText();
				if(name) entry->item.name = name->Value();

				TiXmlText* desc = h.FirstChild("Desc").FirstChild().ToText();
				if(desc) entry->item.desc = desc->Value();

				TiXmlText* custom = h.FirstChild("Custom").FirstChild().ToText();
				if(custom) entry->item.custom = custom->Value();

				// Get all values
				TiXmlNode* values = h.FirstChild("Values").ToNode();
				if(values != NULL){
					TiXmlNode* vvv = values->FirstChild("Value");
					if(vvv != NULL){
						do{
							TiXmlHandle vhv(vvv);
							ACheatEntry::VALUE* onevalue = new ACheatEntry::VALUE;

							TiXmlText* vname = vhv.FirstChild("Name").FirstChild().ToText();
							if(vname) onevalue->name = vname->Value();

							TiXmlText* vlua = vhv.FirstChild("Script").FirstChild().ToText();
							if(vlua) onevalue->script = vlua->Value();

							entry->item.values.push_back(onevalue);
						}while(vvv=vvv->NextSibling("Value"));
					}
				}

				lastChild->next = entry;
				entry->prev = lastChild;
				lastChild = entry;
			}
		}while(node = node->NextSibling());

		lastChild->next = 0;

		if(!cheat->child){
			cheat->child = oldlastChild->next;
			oldlastChild->next->prev = 0;
		}else{
			cheat->child->next = oldlastChild->next;
			oldlastChild->next->prev = cheat->child;
		}
	}
	delete oldlastChild;
}


HTREEITEM AMainWindow::InsertRoot(const char* name,ACheatFile* pfile)
{
	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT|TVIF_PARAM;
	tvis.itemex.pszText = (LPSTR)name;
	tvis.itemex.lParam = (LPARAM)new ParamStruct(ParamStruct::PS_CHEAT,pfile);
	return m_pTree->InsertItem(&tvis);
}

bool AMainWindow::SaveAllCheatFiles()
{
	if(m_bSaving) return true;
	m_bSaving = true;
	extern HWND GetMainHWND();
	::EnableWindow(GetMainHWND(),FALSE);
	HTREEITEM hItem=0;
	hItem = m_pTree->GetRoot();
	while(hItem){
		ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hItem);
		ACheatFile* pfile = (ACheatFile*)ps->pv1;
		if(pfile->bNeedSaving){
			stringstream ss;
			ss<<"是否保存修改: " << pfile->name << " ?";
			if(MessageBox(string(ss.str()).c_str(),"保存修改",MB_YESNO|MB_DEFBUTTON1)==IDYES){
				bool loop = true;
				do{
					if(SaveCheatFile(hItem)) loop=false;continue;
					if(MessageBox("取消保存将丢失所作的修改, 你确定不保存?",
						"保存失败",
						MB_OKCANCEL|MB_DEFBUTTON2
						) == IDOK
					)
					{
						loop = false;
					}
				}while(loop);
			}
		}
		hItem = m_pTree->GetNextItem(hItem,TVGN_NEXT);
	}
	m_bSaving = false;
	::EnableWindow(GetMainHWND(),TRUE);
	return true;
}

bool AMainWindow::SaveCheatFile(HTREEITEM hItem)
{
	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hItem);
	ACheatFile* pfile = (ACheatFile*)ps->pv1;
	string prompt("选择保存文件: ");
	prompt += "名字:" + pfile->name + "  作者:" + pfile->author;
	if(pfile->path.size()==0){
		char path[260]={0};
		strncpy(path,pfile->name.c_str(),sizeof(path));
		
		OPENFILENAME ofn={0};
		ofn.lStructSize = sizeof(ofn);
		ofn.Flags = OFN_DONTADDTORECENT|OFN_ENABLESIZING|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|
			OFN_NONETWORKBUTTON|OFN_NOREADONLYRETURN|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
		ofn.hInstance = theApp->GetAppInstance();
		ofn.hwndOwner = this->GetHwnd();
		ofn.lpstrDefExt = ".xml";
		ofn.lpstrFile = &path[0];
		ofn.lpstrFilter = "XML(*.xml)\0*.xml";
		ofn.lpstrTitle = prompt.c_str();
		ofn.nMaxFile = 260;

		if(!::GetSaveFileName(&ofn))
			return false;

		pfile->path = path;
	}
	if(!pfile->bNeedSaving){
		//只可能是关闭文件
		return true;
	}else{
		if(!DumpCheatsToXml(hItem,pfile->path)){
			MessageBox("保存失败!","内部错误",MB_ICONERROR);
			return false;
		}
		pfile->bNeedSaving=false;
		return true;
	}
}

bool AMainWindow::OpenCheatXml(const char* file)
{
	ACheatFile* pFile;
	HTREEITEM hItem;
	MakeCheatsListFromFile(file,&pFile);
	hItem = InsertRoot(pFile->name.c_str(),pFile);
	ListCheatDir(hItem,pFile,pFile->entry);
	pFile->path = file;
	return true;
}

bool AMainWindow::OpenCheatXml(ACheatFile* file)
{
	InsertRoot(file->name.c_str(),file);
	return true;
}

HTREEITEM AMainWindow::ListCheatDir(HTREEITEM hParent,ACheatFile* pfile,ACheatEntry* node)
{
	if(!node) return NULL;

	assert(hParent != NULL);

	ACheatEntry* pEntry = node;

	if(pEntry){
		do{
			if(pEntry->type == ACheatEntry::TYPE_NODE){
				HTREEITEM hDir = AddSubTreeItem(
					hParent,
					pEntry->node.name.c_str(),
					(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_DIR,pfile,pEntry));
				//递归遍历孩子节点
				ListCheatDir(hDir,pfile,pEntry->child);
			}else if(pEntry->type == ACheatEntry::TYPE_CHEAT){
				AddSubTreeItem(
					hParent,
					pEntry->item.name.c_str(),
					(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_ITEM,pfile,pEntry));
			}else{
				assert(0);
			}
		}while((pEntry=pEntry->next));
	}
	return NULL;
}

bool AMainWindow::DumpCheatsToXml(HTREEITEM hFileTree,void* pdoc)
{
	TiXmlDocument& doc = *(TiXmlDocument*)pdoc;
	ACheatFile* pFile = (ACheatFile*)((ParamStruct*)m_pTree->GetItemLParam(hFileTree))->pv1;

	//每完成一个节点才link

	//XML declaration
	TiXmlDeclaration* declaration = new TiXmlDeclaration("1.0","GB2312","");
	doc.LinkEndChild(declaration);

	//NesCheat节点
	TiXmlElement* nodeNesCheat = new TiXmlElement("NesCheat");
	nodeNesCheat->SetAttribute("version","1.0");

	//Info节点
	TiXmlElement* nodeInfo = new TiXmlElement("Info");

	TiXmlElement* nodeInfoName = new TiXmlElement("Name");
	TiXmlText* nodeInfoNameText = new TiXmlText(pFile->name.c_str());
	nodeInfoName->LinkEndChild(nodeInfoNameText);
	nodeInfo->LinkEndChild(nodeInfoName);

	//if(pFile->author.size()){
		TiXmlElement* nodeInfoAuthor = new TiXmlElement("Author");
		TiXmlText* nodeInfoAuthorText = new TiXmlText(pFile->author.c_str());
		nodeInfoAuthor->LinkEndChild(nodeInfoAuthorText);
		nodeInfo->LinkEndChild(nodeInfoAuthor);
	//}

	TiXmlElement* nodeInfoTime = new TiXmlElement("Time");
	TiXmlText* nodeInfoTimeText = new TiXmlText(pFile->time.c_str());
	nodeInfoTime->LinkEndChild(nodeInfoTimeText);
	nodeInfo->LinkEndChild(nodeInfoTime);

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

bool AMainWindow::DumpCheatsToXml(HTREEITEM hFileTree,string fname)
{
	TiXmlDocument doc;
	return DumpCheatsToXml(hFileTree,&doc)
		&& doc.SaveFile(fname.c_str());
}

bool AMainWindow::DumpCheatsToXml(HTREEITEM hFileTree,string* str)
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

void AMainWindow::DumpCheatsTree(ACheatEntry* pEntry,void* node)
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

			//dump 所有的value
			TiXmlElement* nodeItemValues = new TiXmlElement("Values");
			//for(list_s* p=pEntry->item.values.next; p!=&pEntry->item.values; p=p->next){
			for(auto s = pEntry->item.values.begin(); s != pEntry->item.values.end(); ++s){
				TiXmlElement* v = new TiXmlElement("Value");
				ACheatEntry::VALUE* pValue = *s;

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

void AMainWindow::GetCheatParentAndSiblingsPointers(HTREEITEM hCurrent,void** parent,bool* isFile,ACheatEntry** pPrev,ACheatEntry** pNext)
{
	//取得父结点指针
	HTREEITEM hTvParent = m_pTree->GetNextItem(hCurrent,TVGN_PARENT);
	ParamStruct* psParent = (ParamStruct*)m_pTree->GetItemLParam(hTvParent);
	assert(hTvParent!=NULL && psParent!=NULL && "AMainWindow::GetCheatParentAndSiblingsPointers");
	assert(psParent->type==ParamStruct::PS_CHEAT || psParent->type==ParamStruct::PS_CHEAT_DIR);

	//pv1-> CheatFile, pv2->CheatEntry
	ACheatEntry* pParentEntry = (ACheatEntry*)psParent->pv2;
	ACheatFile*  pParentFile = (ACheatFile*)psParent->pv1;
	bool bIsParentFile = !pParentEntry;

	//取得PrevSibling
	HTREEITEM hTvPrevSibling = m_pTree->GetNextItem(hCurrent,TVGN_PREVIOUS);
	ParamStruct* psPrev = 0;
	ACheatEntry* pPrevEntry = 0;
	if(hTvPrevSibling){
		psPrev = (ParamStruct*)m_pTree->GetItemLParam(hTvPrevSibling);
		pPrevEntry = (ACheatEntry*)psPrev->pv2;
	}

	//取得NextSibling
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

void AMainWindow::AddCheatEntryPointers(ACheatEntry* pEntry,HTREEITEM hCurrent)
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


	//填写parent和child,父结点可能是文件或目录,需要作判断
	//pEntry->child = 0;		//未必是新节点//新结点肯定没有孩子(只有目录才有)
	pEntry->parent = bIsParentFile?0:pParentEntry;

	//填写next和prev
	//pEntry->prev = bHasPrev?pPrevEntry:0;
	pEntry->prev = pPrevEntry;
	pEntry->next = pNextEntry;

	//////////////////////////////////////////////////////////////////////////

	//更新非当前结点的指针域
	if(pPrevEntry){
		pPrevEntry->next = pEntry;
	}
	if(pNextEntry){
		pNextEntry->prev = pEntry;
	}
	if(!pPrevEntry){ // 前一个entry是相对于当前节点层次来说的
		if(bIsParentFile) pParentFile->entry  = pEntry;
		else			  pParentEntry->child = pEntry;
	}
}

void AMainWindow::DelCheatEntryPointers(HTREEITEM hCurrent)
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

void AMainWindow::FreeAllCheatFile(HTREEITEM hFile)
{
	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hFile);
	ACheatFile* pfile = (ACheatFile*)ps->pv1;
	DeleteCheatNode(pfile->entry);
	delete pfile;
	m_pTree->DeleteItem(hFile);
}

void AMainWindow::FreeCheatEntry(ACheatEntry* pEntry)
{
	if(pEntry->type == ACheatEntry::TYPE_NODE){
		//nothing else to do with a node
	}else if(pEntry->type == ACheatEntry::TYPE_CHEAT){
		auto& vs = pEntry->item.values;
		for(auto s=vs.begin(),e=vs.end(); s!=e; ++s){
			delete *s;
		}
		vs.clear();
	}else{
		assert(0 && "AMainWindow::FreeCheatEntry()");
	}
	delete pEntry;
}

//删除金手指项,包括树结点
void AMainWindow::DeleteCheatItem(HTREEITEM hCurrent)
{
	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hCurrent);
	ACheatEntry* pEntry = (ACheatEntry*)ps->pv2;
	assert(pEntry->type == ACheatEntry::TYPE_CHEAT);

	if(m_pCheatEntryCurrent == pEntry){
		m_pCheatEntryCurrent = 0;
		m_CheatInfoDlg->ShowCheatInfo(0,0);
	}

	DelCheatEntryPointers(hCurrent);	//从双向链表中移除节点
	m_pTree->DeleteItem(hCurrent);		//删除该树节点
	FreeCheatEntry(pEntry);				//释放节点内存
	delete ps;							//释放ParamStruct
}

void AMainWindow::DeleteCheatNode(ACheatEntry* pEntry,bool bRecursiveCall)
{
	while(pEntry){
		ACheatEntry* tmp = pEntry->next;

		if(pEntry->type == ACheatEntry::TYPE_NODE){
			DeleteCheatNode(pEntry->child,true);
			delete pEntry;
		}else{
			if(m_pCheatEntryCurrent == pEntry){
				m_pCheatEntryCurrent = 0;
				m_CheatInfoDlg->ShowCheatInfo(0,0);
			}
			FreeCheatEntry(pEntry);
		}
		pEntry = tmp;
	}
}

void AMainWindow::DeleteCheatNode(HTREEITEM hCurrent)
{
	//无效的注释
	//如果是递归删除目录,由于在下一次循环时还要取当前节点的下一个,所以不能删除
	//要在下一级目录删除之后才删除目录treeitem
	// 
	//要移除某个目录,只需要把该目录从prev,next节点从链表中移除,child不用管
	//child只需要释放内存, 所以可以做得简单点
	//TreeView在DeleteItem时,会删除该Item所有的子Item
	//同样适用于删除文件节点

	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hCurrent);
	ACheatEntry* pEntry = 0;
	bool bIsFile = ps->type == ParamStruct::PS_CHEAT;
	if(bIsFile){//文件
		//pEntry = (ACheatEntry*)ps->pv3;
		pEntry = (ACheatEntry*)((ACheatFile*)ps->pv3)->entry;
		if(!pEntry) return;
	}else{
		pEntry = (ACheatEntry*)ps->pv1;
		assert(pEntry->type == ACheatEntry::TYPE_NODE);
	}

	DeleteCheatNode(pEntry->child);

	if(bIsFile==false){
		DelCheatEntryPointers(hCurrent);
		m_pTree->DeleteItem(hCurrent);//BUG:没能删除PS
		delete pEntry;
	}else{
		ACheatFile* file = (ACheatFile*)ps->pv3;
		file->entry = 0;
	}
}

HTREEITEM AMainWindow::GetCheatFileNodeOfCheatOrCheatNode(HTREEITEM hItem)
{
	ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hItem);
	assert(ps->type==ParamStruct::PS_CHEAT_ITEM 
		|| ps->type==ParamStruct::PS_CHEAT_DIR 
		|| ps->type==ParamStruct::PS_CHEAT);

	int type = ps->type;
	ParamStruct* aps = 0;
	while(type != ParamStruct::PS_CHEAT){
		hItem = m_pTree->GetParent(hItem);
		aps = (ParamStruct*)m_pTree->GetItemLParam(hItem);
		type = aps->type;
	}
	return hItem;
}

BOOL AMainWindow::MoveCheatBetweenNodes(HTREEITEM hFrom,HTREEITEM hTo)
{
	ParamStruct* psFrom = (ParamStruct*)m_pTree->GetItemLParam(hFrom);
	ParamStruct* psTo   = (ParamStruct*)m_pTree->GetItemLParam(hTo);

	if(psFrom->type!=ParamStruct::PS_CHEAT_DIR && psFrom->type!=ParamStruct::PS_CHEAT_ITEM)
		throw "只有 金手指/金手指目录 才能够移动!";

	if(psTo->type!=ParamStruct::PS_CHEAT_DIR && psTo->type!=ParamStruct::PS_CHEAT_ITEM && psTo->type!=ParamStruct::PS_CHEAT)
		throw "只有 金手指/金手指目录/金手指文件 能够作为移动到的目标!";

	if(psFrom == psTo)
		throw "相同目标不需要作此移动!(该提示是否该取消?)";

	HTREEITEM hfileToNode = GetCheatFileNodeOfCheatOrCheatNode(hTo);
	HTREEITEM hfileFromNode = GetCheatFileNodeOfCheatOrCheatNode(hFrom);

	if(hfileFromNode != hfileToNode)
		throw "因为我不知道跨文件移动金手指会有什么意义, 所以我取消了~";

	ACheatEntry* pEntryFrom = (ACheatEntry*)psFrom->pv2;
	ACheatEntry* pEntryTo   = (ACheatEntry*)psTo->pv2;
	int typef = psFrom->type == ParamStruct::PS_CHEAT_DIR;
	int typet = psTo->type   == ParamStruct::PS_CHEAT_DIR || psTo->type==ParamStruct::PS_CHEAT;

	HTREEITEM hInserted = 0;
	const char* pszInsertText=0;
	TVINSERTSTRUCT tvi = {0};

	DelCheatEntryPointers(hFrom);
	m_pTree->DeleteItem(hFrom);//fixme:没删除ps

	pszInsertText = typef?pEntryFrom->node.name.c_str():pEntryFrom->item.name.c_str();

	tvi.itemex.mask = TVIF_TEXT|TVIF_PARAM;
	tvi.itemex.pszText = (LPTSTR)pszInsertText;
	tvi.item.lParam = (LPARAM)psFrom;

	//这种&&||的写法是不是有点绕? - 把文件节点当作目录
	if(typet && (psTo->type==ParamStruct::PS_CHEAT || MessageBox(
		"是要移动到目录里面还是后面?\n\n"
		"[是]\t移动到里面\n"
		"[否]\t移动到后面\n",
		"目标为目录",
		MB_YESNO) == IDYES))
	{
		tvi.hParent = hTo;
		tvi.hInsertAfter = TVI_FIRST;
	}else{
		tvi.hParent = m_pTree->GetParent(hTo);
		tvi.hInsertAfter = hTo;
	}

	hInserted = m_pTree->InsertItem(&tvi);

	AddCheatEntryPointers(pEntryFrom,hInserted);

	if(typef){
		ACheatFile* file = (ACheatFile*)psFrom->pv1;
		ListCheatDir(hInserted,file,pEntryFrom->child);
	}
	return TRUE;
}

AMainWindow::AMainWindow():
	m_bDragging(FALSE),
	m_bSaving(false)
{
	m_pTree = new ATreeView;
	this->SetParent(NULL);
	::CreateDialog(NULL,MAKEINTRESOURCE(IDD_MAIN_WINDOW),NULL,(DLGPROC)GetWindowThunk());
}
AMainWindow::~AMainWindow()
{

	delete m_pTree;
}

void AMainWindow::InitTreeViewStyles()
{
	m_pTree->SetStyle(m_pTree->GetStyle()|TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT);
	m_pTree->SetStyle(m_pTree->GetStyle()|TVS_EDITLABELS);
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

bool AMainWindow::GetSaveFile(HWND hwnd,const char* title, char* filter,char* buffer,char* defext)
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

bool AMainWindow::GetMenuTreeItem(HTREEITEM* phitem)
{
	TVHITTESTINFO hti = {0};
	POINT pt;

	::GetCursorPos(&hti.pt);
	pt = hti.pt;
	::ScreenToClient(m_pTree->GetHwnd(),&hti.pt);

	m_pTree->HitTest(&hti);

	if(hti.flags & TVHT_ONITEM){
		if(phitem) *phitem = hti.hItem;

		LPARAM param;
		if(m_pTree->GetItemLParam(hti.hItem,&param)){
			if(param){
				ParamStruct* ps = (ParamStruct*)param;
				this->m_MenuPS = ps;
				this->m_MenuhItem = hti.hItem;
				this->m_MenuFile = (ACheatFile*)ps->pv1;
				return true;
			}
		}
	}else{
		if(phitem) *phitem = 0;
	}
	return false;
}

bool AMainWindow::ShowMenuFor(int type)
{
	HMENU hMenu = NULL;
	POINT pt;
	switch(type)
	{
	//金手指菜单
	case ParamStruct::PS_CHEAT:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),0);
		break;
	case ParamStruct::PS_CHEAT_DIR:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),1);
		break;
	case ParamStruct::PS_CHEAT_ITEM:
		hMenu = GetSubMenu(LoadMenu(NULL,MAKEINTRESOURCE(IDR_MENU_CHEAT)),2);
		break;
	}

	if(!hMenu) return false;
	::GetCursorPos(&pt);
	int id = (int)::TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_RETURNCMD|TPM_NONOTIFY,pt.x,pt.y,0,this->GetHwnd(),NULL);
	if(!id) return false;

	switch(id)
	{
	case IDM_CHEATFILE_DUMPTOXML:
		{
			char file[260];
			BOOL bret = FALSE;
			if(GetSaveFile(
				GetHwnd(),"选择保存文件","XML(*.xml)\0*.xml",file,".xml")){
					bret = DumpCheatsToXml(m_MenuhItem,(string)file);
					MessageBox(bret?"导出成功!":"导出失败!");
				return true;
			}
			return false;
		}
	case IDM_CHEATFILE_CLOSE:
	case IDM_CHEATFILE_SAVE_UPDATE:
		{
			if(SaveCheatFile(m_MenuhItem)){
				if(id==IDM_CHEATFILE_CLOSE){
					FreeAllCheatFile(m_MenuhItem);
				}
				return true;
			}
			return false;
		}
	case IDM_CHEATFILE_VIEWFILE:
		{
			ACheatFile* file = (ACheatFile*)m_MenuPS->pv1;
			string oldname = file->name;
			ACheatFileInfoDlg dlg(this,file);
			int code=dlg.GetDlgCode();
			if(code==ACheatFileInfoDlg::RET_OK){
				if(oldname != file->name){
					UpdateTreeItemText(m_MenuhItem,file->name.c_str());
				}
				file->bNeedSaving=true;
			}

			return dlg.GetDlgCode()==ACheatFileInfoDlg::RET_OK;
		}
	case IDM_CHEATFILE_ADDDIR:
	case IDM_CHEATDIR_ADDDIR:
		{
			HWND hEdit = 0;
			HTREEITEM hTVItem = 0;
			ACheatEntry* pEntry = new ACheatEntry;
			const char* new_dir = "新目录";

			m_pTree->SetFocus();	//must

			hTVItem = AddSubTreeItem(
				m_MenuhItem,(LPTSTR)new_dir,
				(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_DIR,m_MenuFile,pEntry));
			m_pTree->Expand(m_MenuhItem,TVE_EXPAND);
			hEdit = m_pTree->EditLabel(hTVItem);
			::SendMessage(hEdit,EM_SETSEL,0,-1);

			pEntry->type = ACheatEntry::TYPE_NODE;
			pEntry->node.name = new_dir;

			AddCheatEntryPointers(pEntry,hTVItem);

			m_MenuFile->bNeedSaving=true;

			return true;
		}
	case IDM_CHEATFILE_ADDCHEAT:
	case IDM_CHEATDIR_ADDCHEAT:
		{
			HWND hEdit = 0;
			HTREEITEM hTVItem = 0;
			ACheatEntry* pEntry = new ACheatEntry;
			const char* new_cheat = "新金手指";

			m_pTree->SetFocus();	//must
			hTVItem = AddSubTreeItem(
				m_MenuhItem,(LPTSTR)new_cheat,
				(LPARAM)new ParamStruct(ParamStruct::PS_CHEAT_ITEM,m_MenuFile,pEntry));
			m_pTree->Expand(m_MenuhItem,TVE_EXPAND);
			hEdit = m_pTree->EditLabel(hTVItem);
			::SendMessage(hEdit,EM_SETSEL,0,-1);

			pEntry->type = ACheatEntry::TYPE_CHEAT;
			pEntry->item.name = new_cheat;

			AddCheatEntryPointers(pEntry,hTVItem);

			m_MenuFile->bNeedSaving=true;

			return true;
		}
	case IDM_CHEATDIR_RENAME:
	case IDM_CHEATITEM_RENAME:
		{
			m_pTree->SetFocus();
			m_pTree->EditLabel(m_MenuhItem);
			return true;
		}
	case IDM_CHEATITEM_DELETE:
		{
			ACheatEntry* pEntry = (ACheatEntry*)m_MenuPS->pv2;
			assert(pEntry->type == ACheatEntry::TYPE_CHEAT);

			stringstream ss;
			ss<<"确定要删除金手指 "<<pEntry->item.name<<" ?";
			if(MessageBox(string(ss.str()).c_str(),"",MB_YESNO|MB_DEFBUTTON2)==IDNO){
				return false;
			}
			DeleteCheatItem(m_MenuhItem);
			m_MenuFile->bNeedSaving=true;
			return true;
		}
	case IDM_CHEATDIR_DELETE:
		{
			ACheatEntry* pEntry=(ACheatEntry*)m_MenuPS->pv2;
			assert(pEntry->type == ACheatEntry::TYPE_NODE);

			stringstream ss;
			ss<<"确定要删除金手指目录(包含子目录项) "<<pEntry->node.name<<" ?";
			if(MessageBox(string(ss.str()).c_str(),"",MB_YESNO|MB_DEFBUTTON2)==IDNO){
				return false;
			}
			DeleteCheatNode(m_MenuhItem);
			m_MenuFile->bNeedSaving=true;
			return true;
		}
	case IDM_CHEATFILE_DELETE:
		{
			stringstream ss;
			ss<<"确定要删除文件 " << m_MenuFile->name << " ?" << endl;
			ss<<m_MenuFile->path;
			if(MessageBox(string(ss.str()).c_str(),"",MB_YESNO|MB_DEFBUTTON2) == IDNO)
				return false;
			if(m_pCheatFileCurrent == m_MenuFile){
				m_CheatInfoDlg->ShowCheatInfo(0,0);
			}
			::DeleteFile(m_MenuFile->path.c_str());
			FreeAllCheatFile(m_MenuhItem);
			return true;
		}
		return false;
	}
	return false;
}

INT_PTR AMainWindow::HandleDblClick()
{
	switch(m_MenuPS->type)
	{
	case ParamStruct::PS_CHEAT_ITEM:
		{
			ACheatEntry* entry = (ACheatEntry*)m_MenuPS->pv2;
			m_CheatInfoDlg->ShowCheatInfo(m_MenuFile,entry);
			m_pCheatFileCurrent = m_MenuFile;
			m_pCheatEntryCurrent = entry;
			m_hCheatEntryCurrent = m_MenuhItem;
			return SetDlgResult(0);
		}
	}
	return 0;
}

INT_PTR AMainWindow::OnSize(int width,int height)
{
	m_layout.SizeItems();
	return 0;
}

INT_PTR AMainWindow::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AMainWindow::OnClose()
{
	ShowWindow(SW_HIDE);
	return SetDlgResult(TRUE);
}

INT_PTR AMainWindow::OnNotify(LPNMHDR phdr)
{
	if(phdr->idFrom == m_pTree->GetCtrlID()){
		if(phdr->code == NM_DBLCLK){
			if(!GetMenuTreeItem()) return 0;
			return HandleDblClick();
		}else if(phdr->code == NM_RCLICK){
			HTREEITEM hItem;
			if(!GetMenuTreeItem(&hItem)){
				//如果没有右键菜单,当前选中项...跳来跳去, 很不爽
				if(hItem) m_pTree->SelectItem(hItem);
				else{
					POINT pt; GetCursorPos(&pt);
					HMENU hMenu = GetSubMenu(LoadMenu(theApp->GetAppInstance(),MAKEINTRESOURCE(IDR_MENU_MAIN)),0);
					TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,this->GetHwnd(),0);
				}
				return 0;
			}
			//同上
			m_pTree->SelectItem(m_MenuhItem);
			ShowMenuFor(m_MenuPS->type);
			return SetDlgResult(0);
		}else if(phdr->code == TVN_BEGINLABELEDIT){
			LPNMTVDISPINFO pdi = (LPNMTVDISPINFO)phdr;
			LPARAM param = m_pTree->GetItemLParam(pdi->item.hItem);
			ParamStruct* ps = (ParamStruct*)param;
			switch(ps->type)
			{
			//暂时只允许目录或金手指项改名
			case ParamStruct::PS_CHEAT_ITEM:
			case ParamStruct::PS_CHEAT_DIR:
				break;
			default:
				return SetDlgResult(TRUE);
			}
			return SetDlgResult(FALSE);
		}else if(phdr->code == TVN_ENDLABELEDIT){
			LPNMTVDISPINFO pdi = (LPNMTVDISPINFO)phdr;
			ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(pdi->item.hItem);
			switch(ps->type)
			{
			case ParamStruct::PS_CHEAT_DIR:
				{
					ACheatEntry* pEntry = (ACheatEntry*)ps->pv2;
					assert(pEntry->type == ACheatEntry::TYPE_NODE);
					if(pdi->item.pszText != NULL){
						pEntry->node.name = pdi->item.pszText;
						((ACheatFile*)ps->pv1)->bNeedSaving=true;
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
						((ACheatFile*)ps->pv1)->bNeedSaving=true;
						//如果当前的金手指正在被查看,则更新
						if(m_pCheatEntryCurrent == pEntry){
							m_CheatInfoDlg->ShowCheatInfo((ACheatFile*)ps->pv1,pEntry);
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
			m_pTree->SelectItem(hItem);
			ParamStruct* ps = (ParamStruct*)m_pTree->GetItemLParam(hItem);
			if(ps->type!=ParamStruct::PS_CHEAT_DIR && ps->type!=ParamStruct::PS_CHEAT_ITEM){
				return 0;
			}
			hImg = TreeView_CreateDragImage(m_pTree->GetHwnd(),pnmtv->itemNew.hItem);
			ImageList_BeginDrag(hImg,0,0,0);
			ImageList_DragEnter(m_pTree->GetHwnd(),pnmtv->ptDrag.x,pnmtv->ptDrag.y);
			//ShowCursor(FALSE);
			SetCapture(this->GetHwnd());
			m_pTree->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,0);
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
							ACheatFile* pfile = (ACheatFile*)ps->pv1;
							if(pfile->bNeedSaving){
								c2= RGB(255,0,0);
								c1 = RGB(255,255,255);
								break;
							}else{
								return SetDlgResult(CDRF_DODEFAULT);
							}
						}
					case ParamStruct::PS_CHEAT_DIR:
						c1 = RGB(255,255,255);
						c2 = RGB(0,0,255);
						break;
					case  ParamStruct::PS_CHEAT_ITEM:
						return SetDlgResult(CDRF_DODEFAULT);
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
	return 0;
}

INT_PTR AMainWindow::OnMouseMove(int key,int x,int y)
{
	if(m_bDragging){
		TVHITTESTINFO tvht = {0};
		HTREEITEM hItem = 0;
		POINT pt = {x,y};
		::ClientToScreen(this->GetHwnd(),&pt);
		::ScreenToClient(m_pTree->GetHwnd(),&pt);
		x=pt.x;y=pt.y;
		ImageList_DragMove(x,y);
		ImageList_DragShowNolock(FALSE);
		tvht.pt.x = x;
		tvht.pt.y = y;
		if((hItem=m_pTree->HitTest(&tvht)) && tvht.flags&TVHT_ONITEM){
			m_pTree->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,(LPARAM)hItem);
			ImageList_DragShowNolock(TRUE);
		}else{
			m_pTree->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,0);
		}
		return 0;
	}
	return 0;
}

INT_PTR AMainWindow::OnLButtonUp(int key,int x,int y)
{
	if(m_bDragging){
		HTREEITEM hItem = 0;
		ImageList_DragLeave(m_pTree->GetHwnd());
		ImageList_EndDrag();
		hItem = (HTREEITEM)m_pTree->SendMessage(TVM_GETNEXTITEM,TVGN_DROPHILITE,0);
		m_pTree->SendMessage(TVM_SELECTITEM,TVGN_CARET,(LPARAM)hItem);
		m_pTree->SendMessage(TVM_SELECTITEM,TVGN_DROPHILITE,0);
		ReleaseCapture();
		ShowCursor(TRUE);

		m_bDragging = FALSE;

		if(hItem){
			try{
				MoveCheatBetweenNodes(m_hDraggingItem,hItem);
				ACheatFile* pfile = (ACheatFile*)(((ParamStruct*)m_pTree->GetItemLParam(hItem))->pv1);
				pfile->bNeedSaving=true;
				//m_MenuFile->bNeedSaving = true;
			}
			catch(const char* err){
				MessageBox(err,0,MB_ICONINFORMATION);
			}
		}
	}
	return 0;
}

INT_PTR AMainWindow::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(!codeNotify && !hWndCtrl){//Menus
		switch(ctrlID)
		{
		case IDM_MAIN_OPEN_CHEATXML:
			{
				const int bufsize = 1<<20;
				char* files = new char[bufsize];
				memset(files,0,bufsize);
				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				ofn.Flags = OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER|OFN_HIDEREADONLY|
					OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR|OFN_NONETWORKBUTTON|OFN_PATHMUSTEXIST;
				ofn.hInstance = theApp->GetAppInstance();
				ofn.hwndOwner = this->GetHwnd();
				ofn.lpstrFile = files;
				ofn.nMaxFile = bufsize;
				ofn.lpstrFilter = "NES金手指文件(*.xml)\0*.xml\0所有文件(*.*)\0*.*\0";

				if(::GetOpenFileName(&ofn)){
					vector<string> filelist;
					GetFileListFromOFN(&filelist,files);

					for(auto it=filelist.begin();
						it != filelist.end();
						it ++)
					{
						try{
							OpenCheatXml(it->c_str());
						}
						catch(const char* s)
						{
							MessageBox(s,NULL,MB_ICONERROR);
						}
					}
				}

				delete[] files;
				return SetDlgResult(0);
			}
		case IDM_MAIN_NEW_CHEATXML:
			{
				ACheatFileInfoDlg dlg(this,0);
				if(dlg.GetDlgCode() == ACheatFileInfoDlg::RET_OK){
					ACheatFile* file = dlg.GetFile();
					file->bNeedSaving =  true;
					OpenCheatXml(file);
				}
				return 0;
			}
		}
	}
	return 0;
}

INT_PTR AMainWindow::OnDropFiles(HDROP hDrop)
{
	UINT cnt = DragQueryFile(hDrop,-1,0,0);
	if(cnt==0) return 0;
	for(UINT i=0; i<cnt; ++i){
		char file[MAX_PATH]={0};
		DragQueryFile(hDrop,i,file,sizeof(file)/sizeof(*file));
		try{
			OpenCheatXml(file);
		}
		catch(const char* s)
		{
			MessageBox(s,NULL,MB_ICONERROR);
		}
	}
	DragFinish(hDrop);
	return 0;
}

INT_PTR AMainWindow::OnNull(LPARAM lParam)
{
	ControlMessage* pcm = (ControlMessage*)lParam;
	if(!pcm) return 0;
	if(pcm->self = m_CheatInfoDlg){
		if(pcm->uMsg == WM_SETTEXT){
			UpdateTreeItemText(m_hCheatEntryCurrent,(const char*)pcm->wParam);
			return SetDlgResult(TRUE);
		}
	}
	return 0;
}

INT_PTR AMainWindow::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;
	ShowWindow(SW_HIDE);
	m_pTree->AttachCtrl(this,IDC_MAIN_TREE);
	m_CheatInfoDlg = new ACheatInfoDlg(this);
	m_CheatInfoDlg->EnableWindow(FALSE);
	m_CheatInfoDlg->ShowWindow(SW_SHOW);

	::SetMenu(hWnd,::LoadMenu(theApp->GetAppInstance(),MAKEINTRESOURCE(IDR_MENU_MAIN)));

	RECT rc;
	GetClientRect(&rc);
	int height = rc.bottom;
	int width=rc.right;
	//if(GetMenu(hWnd)) height -= GetSystemMetrics(SM_CYMENU);
	m_pTree->SetWindowPos(5,5,200,height-10);
	m_CheatInfoDlg->SetWindowPos(210,5,width-210-5,height-5-5);

	DragAcceptFiles(TRUE);

	this->CenterWindow(NULL);
	this->ShowWindow(SW_SHOWNORMAL);
	InitTreeViewStyles();

	m_layout.Add(m_pTree->GetHwnd(),hWnd,LayoutItem::Align(LayoutItem::kLEFT|LayoutItem::kTOP|LayoutItem::kRisizeVert));
	m_layout.Add(m_CheatInfoDlg->GetHwnd(),hWnd,LayoutItem::Align(LayoutItem::kLEFT|LayoutItem::kTOP|LayoutItem::kRisizeVert|LayoutItem::kRisizeHorz));

	return TRUE;
}

void AMainWindow::GetFileListFromOFN(vector<string>* files,const char* fs)
{
	string dir = fs;
	const char* p=fs;

	while(*p++)
		;
	if(!*p){//只有一个文件
		files->push_back(dir);
	}else{
		do{
			string afile = dir + '\\';
			afile += p;
			files->push_back(afile);
			while(*p++)
				;
		}while(*p);
	}
}

HTREEITEM AMainWindow::AddSubTreeItem(HTREEITEM hParent,const char* pszText,LPARAM param)
{
	TVINSERTSTRUCT tvis = {0};
	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT|TVIF_PARAM;
	tvis.itemex.iImage = 0;
	tvis.itemex.pszText = (LPTSTR)pszText;
	tvis.itemex.lParam = param;
	return m_pTree->InsertItem(&tvis);
}

void AMainWindow::UpdateTreeItemText(HTREEITEM hItem,const char* pszText)
{
	TVITEMEX tvi = {0};
	tvi.mask = TVIF_TEXT;
	tvi.hItem = hItem;
	tvi.pszText = (LPTSTR)pszText;
	m_pTree->SetItem(&tvi);
}
