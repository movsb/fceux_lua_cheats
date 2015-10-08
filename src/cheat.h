#ifndef __CHEAT_H__
#define __CHEAT_H__

#include <cstdio>
#include <string>
#include <vector>
//#include "../list/list.h"

using namespace std;

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
		//list_s entry;
		string name;		//ֵ��
		string script;		//����ָlua�ű�
	};
	struct ITEM{
		//ITEM(){list_init(&values);}	
		string name;		//����ָ����
		string desc;		//����
		string custom;		//�Զ������ָ
		//list_s values;		//ֵ-˫������
		vector<VALUE*> values;
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
		entry = 0;
		bNeedSaving = false;
	}
	string name;		//����
	string author;		//�ļ�����
	string time;		//ʱ��
	string desc;		//�ļ�����

	string path;
	bool bNeedSaving;

	ACheatEntry* entry;
};


#endif//!__CHEAT_H__
