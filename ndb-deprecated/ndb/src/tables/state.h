#ifndef __STATE_H__
#define __STATE_H__

#include <string>
#include "../sqlite3/sqlite3.h"
#include "../list/list.h"

// #pragma pack(push,1)
// struct AStateHeaderFile
// {
// 	size_t		cb;
// 	char*		name;
// 	char*		file;
// };
// #pragma pack(pop)

struct AStateHeader
{
	std::string	name;
	std::string	file;

	AStateHeader(){}
	AStateHeader(std::string _name,std::string _file)
	{
		name = _name;
		file = _file;
		//list_init(&entry);
	}
};

struct AStateMap
{
	int idx;
	AStateHeader* phdr;
	AStateMap* next;
	list_s entry;


	AStateMap(int _idx,AStateHeader* _phdr)
	{
		idx=_idx;
		phdr=_phdr;
		next = 0;
	}
	AStateMap()
	{
		idx=-1;
		phdr=0;
		next=0;
	}
};

class AState
{
public:
	void OpenDB(sqlite3* db);
	static void CreateStateTable(sqlite3* db);
public:
	
// 	void MakeStateHeaderFile(const AStateHeader& header,AStateHeaderFile** phdr);
// 	void MakeStateHeader(AStateHeaderFile* phdr,AStateHeader* header);
	bool AddState(AStateHeader* phdr,const void* data,size_t size,int* idx);
	bool DelState(int idx);
	AStateMap* GetStateHeaders();


private:
	sqlite3* m_db;
	sqlite3_stmt* m_stmt;
	char* m_err;
};

#endif//!__STATE_H__
