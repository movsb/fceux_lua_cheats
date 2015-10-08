#include <cassert>
#include <cstdio>

#include "rom.h"
#include "../win/Archive.h"
#include "../win/App.h"

//using namespace NArchive;

ARom::ARom()
{
}
ARom::~ARom()
{
}

void ARom::SplitFileName(string file,string* drive,string* dir,string* name,string* ext)
{
	string::size_type posDrive = file.find_first_of(':');
	if(posDrive!=string::npos){
		if(drive) *drive = file.substr(0,3);//333333333
	}else{
		if(drive) *drive = "";
	}

	string::size_type posDir = file.find_last_of('\\');
	if(posDir!=string::npos){
		if(dir) *dir = file.substr(0,posDir+1);
	}else{
		if(dir) *dir = "";
	}

	if(posDir!=string::npos){
		if(name) *name = file.substr(posDir+1);
	}else{
		if(name) *name = file;
	}

	string::size_type posDot = (*name).find_last_of('.');
	if(posDot==0 || posDot==string::npos){
		//there is no ext..
		if(ext) *ext = "";
	}else{
		if(ext) *ext = (*name).substr(posDot);
	}
}

void ARom::OpenDb(sqlite3* db)
{
	m_db = db;
	//CalculateRomMd5();
}

void ARom::CreateRomTable(sqlite3* db)
{
	char* err=NULL;
	string sql = "create table if not exists roms (idx integer primary key,hdr blob,data blob);";
	if(sqlite3_exec(db,sql.c_str(),NULL,NULL,&err) == SQLITE_OK){

	}else{
		assert(0 && "ARom::CreateRomTable()");
	}
}

bool ARom::Add(const char* file,const char* splash,int* rom_id,ARomHeader** pph)
{
	FILE* fprom = NULL;
	size_t rom_size;

	MEM_FILE* mf=0;

	mf = GetFirstNesFileFromArchive(file);


	FILE* fpsplash = NULL;
	size_t splash_size;

	fprom = fopen(file,"rb");
	if(fprom == NULL) return false;

	fseek(fprom,0,SEEK_END);
	rom_size = ftell(fprom);
	fseek(fprom,0,SEEK_SET);

	unsigned char* rom_data = new unsigned char[rom_size];
	if(fread(rom_data,1,rom_size,fprom) != rom_size){
		fclose(fprom);
		delete[] rom_data;
		delete mf;
		return false;
	}
	fclose(fprom);

	fpsplash = fopen(splash,"rb");
	if(fpsplash == NULL) return false;

	fseek(fpsplash,0,SEEK_END);
	splash_size = ftell(fpsplash);
	fseek(fpsplash,0,SEEK_SET);

	unsigned char* splash_data = new unsigned char[splash_size];
	if(fread(splash_data,1,splash_size,fpsplash) != splash_size){
		fclose(fpsplash);
		delete[] splash_data;
		delete mf;
		return false;
	}
	fclose(fpsplash);

	ARomHeader* phdr = NULL;

	string name;
	string ext;
	SplitFileName(file,0,0,&name,&ext);

	int type;
#define IS(s) (_stricmp(ext.c_str(),s)==0)
	if(IS(".nes"))		type = TYPE_NES;
	else if(IS(".zip"))	type = TYPE_ZIP;
	else if(IS(".7z"))	type = TYPE_7Z;
	else if(IS(".rar"))	type = TYPE_RAR;
	else				type = TYPE_UNK;
#undef IS	
	
	MakeRomHeader(&phdr,name.c_str(),file,(char*)mf->fdata,mf->fsize,type,splash_data,splash_size,ARomHeader::SPLASH::RHST_PNG);
	phdr->data_size = rom_size;//!!!!!!!TODO:
	bool ret = Add(phdr,(unsigned char*)rom_data,rom_size,rom_id);

	if(pph){
		*pph = phdr;
	}else{
		delete[] phdr;
	}

	delete mf;

	return ret;
}

bool ARom::Add(const ARomHeader* phdr,const unsigned char* pdata,size_t cb,int* row_id)
{
	//添加之前最好是先判断MD5,以决定是否有重复
	int ret;
	string sql = "insert into roms (hdr,data) values (?,?);";
	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		ret = sqlite3_bind_blob(m_stmt,1,phdr,phdr->cb,NULL);
		if(ret==SQLITE_OK){
			ret = sqlite3_bind_blob(m_stmt,2,pdata,cb,NULL);
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

bool ARom::UpdateHeader(int idx,const ARomHeader* phdr)
{
	char id[13]={0};
	sprintf(id,"%d",idx);

	string sql = "update roms set hdr=? where idx=";
	sql += id;

	int ret;
	ret = sqlite3_prepare(m_db,sql.c_str(),-1,&m_stmt,NULL);
	if(ret == SQLITE_OK){
		ret = sqlite3_bind_blob(m_stmt,1,phdr,phdr->cb,NULL);
		if(ret == SQLITE_OK){
			ret = sqlite3_step(m_stmt) == SQLITE_DONE;
			if(ret){
				ret = SQLITE_OK;
			}
		}
		sqlite3_finalize(m_stmt);
	}
	return ret == SQLITE_OK;
}

bool ARom::Delete(const char* idx)
{
	string sql = "delete from roms where idx=";
	sql += idx;

	return sqlite3_exec(m_db,sql.c_str(),NULL,NULL,&m_err)==SQLITE_OK;
}

bool ARom::Delete(int idx)
{
	char s[12];
	sprintf(s,"%d",idx);
	return Delete(s);
}

ARomMap* ARom::GetRomHeaders()
{
	ARomMap rom_map;
	ARomMap* pmap = &rom_map;
	int ret;

	string sql = "select idx,hdr from roms;";
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
					ARomHeader* phdr = (ARomHeader*)new unsigned char[hsize];
					memcpy(phdr,sqlite3_column_blob(m_stmt,1),hsize);
					MakeRomHeaderFromDataBase(phdr);
					pmap->next = new ARomMap(phdr,idx);
					pmap = pmap->next;
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
	return rom_map.next;
}

bool ARom::GetRomData(int idx,void** ppdata,size_t* psize)
{
	int ret;

	char id[12];
	sprintf(id,"%d",idx);

	void* data;
	size_t size;

	string sql = "select data from roms where idx=";
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

bool ARom::SaveAllRoms(const char* folder)
{
	std::string dir(folder);

	ARomMap* rm = GetRomHeaders();
	while(rm){
		std::string f = dir + rm->phdr->comment;
		SaveRomAsFile(rm->idx,f.c_str());

		rm=rm->next;
	}
	return true;
}

bool ARom::SaveRomAsFile(int idx,const char* fname)
{
	bool b;

	void* pData=0;
	size_t sz=0;
	FILE* fp=0;

	try{
		b = GetRomData(idx,&pData,&sz);
		if(b==false) throw mkerr(("无法取得指定索引(%d)的ROM数据!",idx));

		fp = fopen(fname,"wb");
		if(fp == NULL){
			throw mkerr(("无法打开文件(%s)用于写!",fname));
		}

		fseek(fp,0,SEEK_SET);
		if(fwrite(pData,1,sz,fp) != sz){
			throw "文件写入错误!";
		}
		fclose(fp);

		delete[] pData;
		return true;
	}
	catch(...){
		if(pData) delete[] pData;
		if(fp){
			fclose(fp);
			remove(fname);
		}
		throw;
	}
	return false;
}

void ARom::MakeRomHeader(ARomHeader** pphdr,const char* cmt,const char* desc,const char* pdata,size_t cbdata,int type,unsigned char* splash,size_t cbsplash,int sptype)
{
	int len_cmt = strlen(cmt)+1;
	int len_desc = strlen(desc)+1;

	size_t hdrsize = sizeof(ARomHeader) + len_cmt + len_desc + cbsplash;
	ARomHeader* hdr = (ARomHeader*)new unsigned char[hdrsize];
	memset(hdr,0,hdrsize);

	hdr->cb = hdrsize;
	hdr->offset = sizeof(ARomHeader);
	hdr->version = 0x01;
	
	hdr->comment = (char*)hdr + sizeof(ARomHeader);
	hdr->description = hdr->comment + len_cmt;
	memcpy((void*)hdr->comment,cmt,len_cmt);
	memcpy((void*)hdr->description,desc,len_desc);

	//assert(type == TYPE_NES);

	hdr->type = type;

	CalcMd5((char*)pdata+16,cbdata-16,hdr->md5);
	CalcCrc32((char**)pdata+16,cbdata-16,hdr->crc32);

	hdr->data_size = cbdata;

	memcpy(hdr->ines,pdata,16);

	if(splash){
		hdr->splash.type = sptype;
		hdr->splash.data = (unsigned char*) hdr->description + len_desc;
		hdr->splash.cb = cbsplash;
		memcpy(hdr->splash.data,splash,hdr->splash.cb);
	}else{
		hdr->splash.type = ARomHeader::SPLASH::RHST_NONE;
	}

	*pphdr = hdr;
}
void ARom::MakeRomHeaderFromDataBase(ARomHeader* hdr)
{
	size_t size = hdr->cb;

	hdr->comment = (char*)hdr + sizeof(ARomHeader);
	hdr->description = hdr->comment + strlen(hdr->comment) + 1;
	hdr->splash.data = (unsigned char*)hdr->description + strlen(hdr->description) + 1;
}

void ARom::MakeINESHeader(ARomHeader* hdr,INESHeader* ines)
{
	const unsigned char* h = (unsigned char*)hdr->ines;
	ines->n16kbprom = h[4];
	ines->n8kbchrom = h[5];
	ines->mirror = h[6]&0x01?INESHeader::NESMIRROR_VERT:INESHeader::NESMIRROR_HORZ;
	ines->battery = (h[6]&0x02)>0;
	ines->trainer = (h[6]&0x04)>0;
	ines->mapper = h[7]&0xF0 | h[6]>>4;
}

#include "../win/Archive.h"

void ARom::CalculateRomMd5()
{
	ARomMap* rm = GetRomHeaders();
	while(rm){
		SaveRomAsFile(rm->idx,"1");
		MEM_FILE* mf = GetFirstNesFileFromArchive("1");

		CalcMd5((char*)mf->fdata+16,mf->fsize-16,&rm->phdr->md5[0]);
		CalcCrc32((char*)mf->fdata+16,mf->fsize-16,&rm->phdr->crc32[0]);

		if(!UpdateHeader(rm->idx,rm->phdr))
			throw "3";

		rm=rm->next;
	}
}
