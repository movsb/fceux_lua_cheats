#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include <string>
#include <vector>
// #include <cstdio>
// #include <cassert>

#define BEGIN_NAMESPACE(name)	namespace name{
#define	END_NAMESPACE			}

//BEGIN_NAMESPACE(NArchive)

#ifdef __IM_ARCHIVE__


#endif//!__IM_ARCHIVE__

struct MEM_FILE
{
	std::string fname;
	void*		fdata;
	size_t		fsize;

	MEM_FILE():
		fdata(0),
		fsize(0)
	{}

	~MEM_FILE()
	{
		if(fdata) delete[] fdata;
	}
};

bool initArchiveSystem();

void CalcMd5(void* pdata,size_t cbdata,unsigned char md5[32]);
void CalcCrc32(void* pdata,size_t cbdata,unsigned char crc32[8]);

MEM_FILE* GetFirstNesFileFromArchive(std::string fname);
void GetFileListFromOFN(std::vector<std::string>* files,const char* fs);


//END_NAMESPACE

#endif//!__ARCHIVE_H__
