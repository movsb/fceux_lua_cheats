#include "cheat.h"

#include <cassert>
#include <string>
#include <cstdio>
using namespace std;

#include "../Str.h"
#include "../tinyxml/tinyxml.h"
#include "../list/list.h"

ACheat::ACheat()
{

}
ACheat::~ACheat()
{

}

void ACheat::OpenDb(sqlite3* db)
{
	m_db = db;
}

bool ACheat::MakeCheatsListFromFile(char* fn,ACheatFile** ppCheatFile)
{
	TiXmlDocument doc(fn);
	doc.SetCondenseWhiteSpace(false);
	if(!doc.LoadFile()) {
		return false;
	}

	ACheatFile* cheat = new ACheatFile;

	TiXmlNode* nodeNesCheat = doc.FirstChild("NesCheat");
	if(nodeNesCheat == NULL){
		//no node NesCheat found
		return false;
	}

	TiXmlNode* nodeInfo = nodeNesCheat->FirstChild("Info");
	if(nodeInfo != NULL){
		TiXmlHandle h(nodeInfo);
		TiXmlText* name = h.FirstChild("Name").FirstChild().ToText();
		if(name) cheat->name = name->Value();

		TiXmlText* md5 = h.FirstChild("Md5").FirstChild().ToText();
		if(md5) cheat->md5 = md5->Value();

		TiXmlText* crc32 = h.FirstChild("Crc32").FirstChild().ToText();
		if(crc32) cheat->crc32 = crc32->Value();

		TiXmlText* author = h.FirstChild("Author").FirstChild().ToText();
		if(author) cheat->author = author->Value();

		TiXmlText* time = h.FirstChild("Time").FirstChild().ToText();
		if(time) cheat->time = time->Value();

		TiXmlText* desc = h.FirstChild("Desc").FirstChild().ToText();
		if(desc) cheat->desc = desc->Value();
	}

	TiXmlNode* nodeCheats = nodeNesCheat->FirstChild("Cheats");
	if(nodeCheats != NULL){
		//cheat->entry->child=cheat->entry->parent=cheat->entry->prev=cheat->entry->next=0;
		ACheatEntry tmpentry;
		TravelCheatNode(&tmpentry,nodeCheats);
		//tmpentry只是个假的父节点
		ACheatEntry* pt = tmpentry.child;
		while(pt){
			pt->parent = 0;
			pt = pt->next;
		}
		cheat->entry = tmpentry.child;
	}

	*ppCheatFile = cheat;
	return true;
}

list_s* ACheat::TravelCheatNode(ACheatEntry* cheat,void* node)
{
	TiXmlNode* xnode = (TiXmlNode*)node;
	TiXmlNode* child = xnode->FirstChild();

	ACheatEntry* lastChild = new ACheatEntry;
	ACheatEntry* oldlastChild=lastChild;

	if(child != NULL){
		do{
			if(strcmp(child->Value(),"Node")==0){
				//如果是目录
				ACheatEntry* dir = new ACheatEntry;
				dir->type = ACheatEntry::TYPE_NODE;
				dir->parent = cheat;

				const char* name = child->ToElement()->Attribute("name");

				name = name?name:"目录";
				dir->node.name = name;

				lastChild->next = dir;	//目录作为上一个结点的sibling
				dir->prev = lastChild;
				lastChild = dir;

				TravelCheatNode(dir,child);

			}else if(strcmp(child->Value(),"Item")==0){
				//当前是CheatItem
				ACheatEntry* entry = new ACheatEntry;
				entry->type = ACheatEntry::TYPE_CHEAT;
				entry->parent = cheat;

				TiXmlHandle h(child);

				TiXmlText* name = h.FirstChild("Name").FirstChild().ToText();
				if(name) entry->item.name = name->Value();

				TiXmlText* desc = h.FirstChild("Desc").FirstChild().ToText();
				if(desc) entry->item.desc = desc->Value();

				TiXmlText* custom = h.FirstChild("Custom").FirstChild().ToText();
				if(custom) entry->item.custom = custom->Value();

				//遍历并得到所有的值
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

							list_insert_tail(&entry->item.values,&onevalue->entry);
						}while(vvv=vvv->NextSibling("Value"));
					}
				}
				
				lastChild->next = entry;
				entry->prev = lastChild;
				lastChild = entry;
			}
		}while(child = child->NextSibling());

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
	return 0;
}

void ACheat::CreateCheatTable(sqlite3* db)
{
	char* err=NULL;
	string sql = "create table if not exists cheats (idx integer primary key,hdr blob,data blob);";
	if(sqlite3_exec(db,sql.c_str(),NULL,NULL,&err) == SQLITE_OK){

	}else{
		assert(0 && "ACheat::CreateCheatTable()");
	}
}

bool ACheat::ReadXmlFileContent(const char* fn,void** ppdata,size_t* psize)
{

	FILE* fp = fopen(fn,"rb");
	if(!fp) return false;

	fseek(fp,0,SEEK_END);

	size_t fsize;
	char* data = NULL;

	fsize = ftell(fp);
	data = new char[fsize];

	fseek(fp,0,SEEK_SET);
	if(fread(data,1,fsize,fp) != fsize){
		delete[] data;
		fclose(fp);
		return false;
	}
	fclose(fp);

	*ppdata = data;
	*psize = fsize;
	return true;
}

bool ACheat::Add(const char* filename,ACheatHeader** pphdr,int* row_id)
{
	char* data=0;
	size_t fsize=0;

	if(!ReadXmlFileContent(filename,(void**)&data,&fsize))
		return false;

	ACheatHeader* phdr = NULL;
	string strfile = filename;
	string name = strfile.substr(strfile.find_last_of('\\')+1);
	MakeCheatHeader(&phdr,(char*)name.c_str(),"");

	bool ret = Add(phdr,data,fsize,row_id);

	if(ret && pphdr){
		*pphdr = phdr;
	}else{
		delete[] pphdr;
	}
	delete[] data;
	return ret;
}

bool ACheat::Add(ACheatHeader* phdr,const char* pdata,size_t cbdata,int* row_id)
{
	int ret;
	string sql = "insert into cheats (hdr,data) values (?,?);";
	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		ret = sqlite3_bind_blob(m_stmt,1,phdr,phdr->cb,NULL);
		if(ret==SQLITE_OK){
			ret = sqlite3_bind_blob(m_stmt,2,pdata,cbdata,NULL);
			if(ret == SQLITE_OK){
				ret = sqlite3_step(m_stmt) == SQLITE_DONE;
				if(ret){
					ret = SQLITE_OK;
					if(row_id) *row_id = (int)sqlite3_last_insert_rowid(m_db);
				}
			}
		}
		sqlite3_finalize(m_stmt);
	}
	return ret == SQLITE_OK;
}

bool ACheat::Update(ACheatHeader* phdr,const char* pdata,size_t cbdata,int row_id)
{
	int ret;
	char id[13];
	sprintf(id,"%d",row_id);

	string sql = "select idx from cheats where idx=";
	sql += id;

	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,0);
	if(ret==SQLITE_OK){
		ret = sqlite3_step(m_stmt);
		sqlite3_finalize(m_stmt);
		if(ret == SQLITE_DONE){
			return false;
		}else if(ret == SQLITE_ROW){

		}else{
			return false;
		}
	}

	sql = "update cheats set hdr=?,data=? where idx=";
	sql += id;

	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		ret = sqlite3_bind_blob(m_stmt,1,phdr,phdr->cb,NULL);
		if(ret==SQLITE_OK){
			ret = sqlite3_bind_blob(m_stmt,2,pdata,cbdata,NULL);
			if(ret == SQLITE_OK){
				ret = sqlite3_step(m_stmt) == SQLITE_DONE;
				if(ret){
					ret = SQLITE_OK;
				}
			}
		}
		sqlite3_finalize(m_stmt);
	}
	return ret == SQLITE_OK;
}

bool ACheat::Delete(const char* idx)
{
	string sql = "delete from cheats where idx=";
	sql += idx;

	return sqlite3_exec(m_db,sql.c_str(),NULL,NULL,&m_err)==SQLITE_OK;
}
bool ACheat::Delete(int idx)
{
	char s[12];
	sprintf(s,"%d",idx);
	return Delete(s);
}

ACheatMap* ACheat::GetCheatHeaders()
{
	ACheatMap cheat_map;
	ACheatMap* pcheat = &cheat_map;
	int ret;

	string sql = "select idx,hdr from cheats;";
	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		bool loop=true;
		for(;loop;){
			switch(sqlite3_step(m_stmt))
			{
			case SQLITE_ROW:
				{
					int idx = (int)sqlite3_column_int(m_stmt,0);
					size_t hsize = sqlite3_column_bytes(m_stmt,1);
					ACheatHeader* phdr = (ACheatHeader*)new unsigned char[hsize];
					memcpy(phdr,sqlite3_column_blob(m_stmt,1),hsize);
					MakeCheatHeaderFromDataBase(phdr);
					pcheat->next = new ACheatMap(phdr,idx);
					pcheat = pcheat->next;
					continue;
				}
			case SQLITE_DONE:
				loop = false;
				continue;
			default:
				assert(0 && "sqlite3_step() failed");
				loop = false;
				break;
			}
		}
		sqlite3_finalize(m_stmt);
	}
	pcheat->next = NULL;
	return cheat_map.next;
}

bool ACheat::GetCheatData(int idx,void** ppdata,size_t* psize)
{
	int ret;

	char id[12];
	sprintf(id,"%d",idx);

	void* data;
	size_t size;

	string sql = "select data from cheats where idx=";
	sql += id;

	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		ret = sqlite3_step(m_stmt);
		switch(ret)
		{
		case SQLITE_DONE:
			sqlite3_finalize(m_stmt);
			return false;
		case SQLITE_ROW:
			size = sqlite3_column_bytes(m_stmt,0);
			data = (void*)new char[size];
			memcpy(data,sqlite3_column_blob(m_stmt,0),size);
			sqlite3_finalize(m_stmt);

			*ppdata = data;
			*psize = size;

			return true;
		default:
			return false;
		}
	}
	return false;
}

void ACheat::MakeCheatHeader(ACheatHeader** pphdr,char* cmt,char* desc)
{
	int len_cmt = strlen(cmt)+1;
	int len_dsc = strlen(desc)+1;
	size_t hdrsize = len_cmt+len_dsc+sizeof(ACheatHeader);
	ACheatHeader* hdr = (ACheatHeader*)new char[hdrsize];
	memset(hdr,0,hdrsize);

	hdr->cb = hdrsize;
	hdr->version = 0x01;
	hdr->comment = (char*)hdr + sizeof(ACheatHeader);
	hdr->description = hdr->comment + len_cmt;
	memcpy(hdr->comment,cmt,len_cmt);
	memcpy(hdr->description,desc,len_dsc);

	*pphdr = hdr;
}

void ACheat::MakeCheatHeaderFromDataBase(ACheatHeader* phdr)
{
	phdr->comment = (char*)phdr+sizeof(ACheatHeader);
	phdr->description = phdr->comment + strlen(phdr->comment) + 1;
}
