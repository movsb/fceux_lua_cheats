#include "state.h"
#include "../win/App.h"

#include <cassert>
#include <sstream>

using namespace std;

void AState::CreateStateTable(sqlite3* db)
{
	char* err=NULL;
	string sql = "create table if not exists states (idx integer primary key,name text,file text,data blob);";
	if(sqlite3_exec(db,sql.c_str(),NULL,NULL,&err) == SQLITE_OK){

	}else{
		assert(0 && "ARom::CreateStateTable()");
	}
}

void AState::OpenDB(sqlite3* db)
{
	m_db = db;
}

bool AState::AddState(AStateHeader* phdr,const void* data,size_t size,int* pidx)
{
	stringstream ss;
	ss<<"insert into states (name,file,data) values ('";
	ss<<phdr->name<<"','";
	ss<<phdr->file<<"',?)";

	int ret;
	sqlite3_stmt* stmt=0;
	try{
		string sql(ss.str());
		ret = sqlite3_prepare(m_db,sql.c_str(),-1,&stmt,0);
		if(ret != SQLITE_OK) throw mkerr(("%s\n\n(%s)",sql.c_str(),sqlite3_errmsg(m_db)));
		ret = sqlite3_bind_blob(stmt,1,data,size,0);
		if(ret != SQLITE_OK) throw mkerr(("%s\n\nbind failed!",sql.c_str()));
		ret = sqlite3_step(stmt);
		if(ret != SQLITE_DONE) throw mkerr(("%s\n\nstep failed!",sql.c_str()));

		if(pidx) *pidx = (int)sqlite3_last_insert_rowid(m_db);

		sqlite3_finalize(stmt);

		return true;
	}catch(string e){
		if(stmt) sqlite3_finalize(stmt);

		throw;
	}
}

bool AState::DelState(int idx)
{
	stringstream ss;
	ss<<"delete from states where idx="<<idx;
	
	string sql = ss.str();

	return sqlite3_exec(m_db,sql.c_str(),NULL,NULL,&m_err)==SQLITE_OK;
}

AStateMap* AState::GetStateHeaders()
{
	AStateMap sm;
	AStateMap* pmap = &sm;
	int ret;

	string sql = "select idx,name,file from states;";
	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		bool loop=true;
		for(;loop;){
			switch(sqlite3_step(m_stmt))
			{
			case SQLITE_ROW:
				{
					string name = (char*)sqlite3_column_text(m_stmt,1);
					string file = (char*)sqlite3_column_text(m_stmt,2);
					int idx = (int)sqlite3_column_int(m_stmt,0);

					AStateMap* m = new AStateMap(idx,new AStateHeader(name,file));

					pmap->next = m;
					pmap = m;
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
	pmap->next = NULL;
	return sm.next;
}
