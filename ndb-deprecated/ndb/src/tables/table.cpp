#include "table.h"
#include "../Str.h"

#include <cassert>

ATable::ATable()
{

}

ATable::~ATable()
{

}

bool ATable::Open(const char* db)
{
	bool ret = sqlite3_open(AStr(db,false).toUtf8(),&m_db)==SQLITE_OK;
	if(ret){
		InitAllTables();
	}
	return ret;
}

bool ATable::Close()
{
	bool ret = sqlite3_close(m_db)==SQLITE_OK;
	if(ret){
		m_db = NULL;
	}
	return ret;
}

sqlite3* ATable::GetDb() const
{
	assert(m_db!=NULL && "ATable::GetDb()");
	return m_db;
}

void ATable::InitAllTables()
{
	//ARom::CreateRomTable(m_db);
	ACheat::CreateCheatTable(m_db);
	//AState::CreateStateTable(m_db);
}

bool ATable::SaveToFile(void* data,size_t size,const char* fn)
{
	FILE* fp = fopen(fn,"wb");
	if(!fp) return false;
	if(fwrite(data,1,size,fp)==size){
		fclose(fp);
		return true;
	}else{
		fclose(fp);
		remove(fn);
		return false;
	}
}