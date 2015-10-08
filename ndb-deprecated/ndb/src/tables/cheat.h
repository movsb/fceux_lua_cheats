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
	int cb;				//�ṹ���С
	int version;		//�汾
	char* comment;		//��Ҫ˵��
	char* description;	//��ϸ˵��

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

	TYPE type;				//����:TYPE_NODE:Ŀ¼,TYPE_CHEAT:cheat

	struct NODE{	
		string name;		//Ŀ¼��
	};
	struct VALUE{
		list_s entry;
		string name;		//ֵ��
		string script;		//����ָlua�ű�
	};
	struct ITEM{
		ITEM(){list_init(&values);}	
		string name;		//����ָ����
		string desc;		//����
		string custom;		//�Զ������ָ
		list_s values;		//ֵ-˫������
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
	float version;		//NesCheat�汾	
	string name;		//����
	string md5;			//Md5
	string crc32;		//crc32
	string author;		//�ļ�����
	string time;		//ʱ��
	string desc;		//�ļ�����

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
