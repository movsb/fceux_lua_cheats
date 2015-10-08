#ifndef __CHEAT_H__
#define __CHEAT_H__

#include <cstdio>
#include <string>

#include "../sqlite3/sqlite3.h"
#include "../list/list.h"
//#include "../tinyxml/tinyxml.h"

using namespace std;

#pragma pack(push,1)
struct ACheatHeader
{
	int cb;				//结构体大小
	int version;		//版本
	char* comment;		//简要说明
	char* description;	//详细说明

	union{
		unsigned char reserved[128];
	};
};
#pragma pack(pop)

struct ACheatMap{
	ACheatMap(){
		phdr=0;
		idx=0;
		next=0;
	}
	ACheatMap(ACheatHeader* _hdr,int _idx)
	{
		phdr = _hdr;
		idx = _idx;
		next = 0;
	}

	ACheatHeader* phdr;
	ACheatMap* next;
	int idx;
};

struct ACheatEntry
{
	enum TYPE{TYPE_NODE,TYPE_CHEAT};

	ACheatEntry()
	{
		parent=child=prev=next=0;
	}

	TYPE type;				//类型:TYPE_NODE:目录,TYPE_CHEAT:cheat

	struct NODE{	
		string name;		//目录名
	};
	struct VALUE{
		list_s entry;
		string name;		//值名
		string script;		//金手指lua脚本
	};
	struct ITEM{
		ITEM(){list_init(&values);}	
		string name;		//金手指名称
		string desc;		//描述
		string custom;		//自定义金手指
		list_s values;		//值-双向链表
	};

//	union{
		NODE node;
		ITEM item;
//	};
	

	ACheatEntry *parent,*child,*prev,*next;
};

struct ACheatFile
{
	ACheatFile()
	{
		version = 0.0f;
		entry = 0;
		//name = md5 = crc32 = author = time = desc = 0;
	}
	float version;		//NesCheat版本	
	string name;		//名字
	string md5;			//Md5
	string crc32;		//crc32
	string author;		//文件作者
	string time;		//时间
	string desc;		//文件描述

	ACheatEntry* entry;
};


class ACheat
{
public:
	ACheat();
	~ACheat();
	static void CreateCheatTable(sqlite3* db);

public:
	void OpenDb(sqlite3* db);
	bool Add(const char* filename,ACheatHeader** pphdr,int* row_id);
	bool Add(ACheatHeader* phdr,const char* pdata,size_t cbdata,int* row_id);
	bool Update(ACheatHeader* phdr,const char* pdata,size_t cbdata,int row_id);
	bool Delete(const char* idx);
	bool Delete(int idx);
	ACheatMap* GetCheatHeaders();
	bool GetCheatData(int idx,void** ppdata,size_t* psize);
	void MakeCheatHeader(ACheatHeader** pphdr,char* cmt,char* desc);
	void MakeCheatHeaderFromDataBase(ACheatHeader* phdr);
	bool MakeCheatsListFromFile(char* fn,ACheatFile** ppCheatFile);
	bool ReadXmlFileContent(const char* fn,void** ppdata,size_t* psize);

private:
	list_s* TravelCheatNode(ACheatEntry* cheat,void* node);

private:
	sqlite3* m_db;
	sqlite3_stmt* m_stmt;
	char* m_err;
};

#endif//!__CHEAT_H__
