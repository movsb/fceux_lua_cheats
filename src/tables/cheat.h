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

	TYPE type;				//类型:TYPE_NODE:目录,TYPE_CHEAT:cheat

	struct NODE{	
		string name;		//目录名
	};
	struct VALUE{
		//list_s entry;
		string name;		//值名
		string script;		//金手指lua脚本
	};
	struct ITEM{
		//ITEM(){list_init(&values);}	
		string name;		//金手指名称
		string desc;		//描述
		string custom;		//自定义金手指
		//list_s values;		//值-双向链表
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
	string name;		//名字
	string author;		//文件作者
	string time;		//时间
	string desc;		//文件描述

	string path;
	bool bNeedSaving;

	ACheatEntry* entry;
};


#endif//!__CHEAT_H__
