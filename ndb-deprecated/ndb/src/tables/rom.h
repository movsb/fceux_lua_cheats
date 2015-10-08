#ifndef __ROM_H__
#define __ROM_H__

#include <cstdio>
#include <string>
#include <sstream>
//#include <map>

#include "../sqlite3/sqlite3.h"

using namespace std;

/************************************************************************
 ��:
 id		- integer:	primary key
 hdr	- blob:		ARomHeader
 data	- blob:		Data
 ************************************************************************/
#pragma pack(push,1)
struct ARomHeader
{
	int cb;							//�����ṹ���С(����˵��+��������)
	int offset;						//���ݲ���ƫ��
	int version;					//�汾��

	const char* comment;			//��Ҫ˵������
	const char* description;		//��������

	int type;						//ROM�ļ�����,�������TYPE_ENCRYPTED,��ROM�Ǳ����ܱ����
	size_t data_size;
	unsigned char md5[32];			//ROM's MD5У��
	unsigned char crc32[8];			//ROM's crc32У��

	unsigned char ines[16];			//NES�ļ�ͷ(16���ֽ�)

	struct SPLASH{
		enum{RHST_NONE,RHST_PNG};
		int type;
		unsigned char* data;	//NES��Ϸ����
		size_t cb;
	}splash;
	
	union{
		unsigned char reserved[128];
		unsigned char enc_data[16];	//���ܺ����֤����
		int additional_bytes;		//���ܺ��������Ч���ֵ������ֽ���
	};

	//��չ�ڴ�ṹ
	//comment
	// description
	// splash
};

struct INESHeader{
	enum{NESMIRROR_VERT,NESMIRROR_HORZ};
	int n16kbprom;
	int n8kbchrom;
	int mirror;
	int battery;
	int trainer;
	int mapper;
};

#pragma pack(pop)

struct ARomMap{
	ARomMap(){
		phdr=0;
		idx=0;
		next=0;
	}
	ARomMap(ARomHeader* _hdr,int _idx)
	{
		phdr = _hdr;
		idx = _idx;
	}
	ARomHeader* phdr;
	int idx;
	ARomMap* next;
};

class ARom
{
public:
	enum{
		TYPE_UNK=0,
		TYPE_NES,
		TYPE_ZIP,
		TYPE_7Z,
		TYPE_RAR,
		TYPE_ENCRYPTED=0x80000000
	};
public:
	ARom();
	~ARom();
	void OpenDb(sqlite3* db);
	static void CreateRomTable(sqlite3* db);
	//�������ROM����, ������ʼ��md5��crc32
	static void MakeRomHeader(ARomHeader** pphdr,const char* cmt,const char* desc,const char* pdata,size_t cbdata,int type,unsigned char* splash,size_t cbsplash,int sptype);
	static void MakeRomHeaderFromDataBase(ARomHeader* hdr);
	static void MakeINESHeader(ARomHeader* hdr,INESHeader* ines);
	static void SplitFileName(string file,string* drive,string* dir,string* name,string* ext);

public:
	bool Add(const char* file,const char* splash,int* rom_id,ARomHeader** pph);
	bool Add(const ARomHeader* phdr,const unsigned char* pdata,size_t cb,int* row_id);
	bool UpdateHeader(int idx,const ARomHeader* phdr);
	bool Delete(const char* idx);
	bool Delete(int idx);
	ARomMap* GetRomHeaders();
	bool GetRomData(int idx,void** ppdata,size_t* psize);
	bool SaveRomAsFile(int idx,const char* fname);
	bool SaveAllRoms(const char* folder);

private:
	ARom(const ARom&){}
	void CalculateRomMd5();
private:
	sqlite3* m_db;
	sqlite3_stmt* m_stmt;
	char* m_err;
};

#endif//!__ROM_H__
