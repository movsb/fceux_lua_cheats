#ifndef __TABLE_H__
#define __TABLE_H__

#include "../sqlite3/sqlite3.h"

#include <string>

#include "rom.h"
#include "cheat.h"
#include "state.h"

using namespace std;

class ATable
{
public:
	ATable();
	~ATable();
	static bool SaveToFile(void* data,size_t size,const char* fn);

public:
	bool Open(const char* db);
	bool Close();
	sqlite3* GetDb() const;

private:
	void InitAllTables();

private:
	sqlite3* m_db;
	sqlite3_stmt* m_stmt;
	const char* m_zerr;
};

#endif//!__TABLE_H__
