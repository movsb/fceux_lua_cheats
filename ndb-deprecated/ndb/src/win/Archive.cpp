#include "../libs/md5.h"
#include "../libs/crc32.h"

#define __IM_ARCHIVE__
#include "Archive.h"

#include <windows.h>
#include <initguid.h>
#include <objbase.h>
#include <oleauto.h>

#include <string>
#include <vector>
#include <cstdio>
#include <cassert>

#include "../7zip/IArchive.h"



class AInMemStream : public IInStream, private IStreamGetSize
{
private:
	ULONG m_refCount;

private:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID,void**)
	{
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++m_refCount;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return --m_refCount;
	}

public:
	HRESULT STDMETHODCALLTYPE GetSize(UInt64* size)
	{
		*size = m_Size;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Read(void* pdata,UInt32 length,UInt32* readLength)
	{
		if(m_curpos + length > m_Size-1){
			length = (m_Size-1) - m_curpos + 1;
		}

		if(length==0 || m_curpos>(long)m_Size-1){
			return E_INVALIDARG;
		}

		memcpy(pdata,(char*)m_pData+m_curpos,length);
		if(readLength) *readLength = length;

		m_curpos += length;

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Seek(Int64 offset,UInt32 origin,UInt64* pos)
	{
		long newpos;
		if(origin == SEEK_SET){
			newpos = 0 + (long)offset;
		}else if(origin == SEEK_CUR){
			newpos = m_curpos + (long)offset;
		}else if(origin == SEEK_END){
			newpos = (long)m_Size-1 + (long)offset;
		}

		if(0>newpos || newpos>(long)m_Size-1){
			return E_INVALIDARG;
		}

		m_curpos = newpos;
		if(pos) *pos=m_curpos;
		return S_OK;
	}


private:
	size_t		m_Size;
	const void*	m_pData;
	long		m_curpos;

public:
	explicit AInMemStream(const void* pdata,size_t sz):
	m_refCount(0),
		m_Size(0),
		m_pData(0),
		m_curpos(0)
	{
		m_pData = pdata;
		m_curpos = 0;
		m_Size  = sz;
	}
	void rewind()
	{
		m_curpos = 0;
	}

	~AInMemStream()
	{
	}
};

class AExtractCallback : public IArchiveExtractCallback
{
private:
	class OutMemStream : public ISequentialOutStream
	{
	private:
		ULONG m_refCount;

		HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID,void**)
		{
			return E_NOINTERFACE;
		}

		ULONG STDMETHODCALLTYPE AddRef()
		{
			return ++m_refCount;
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			return --m_refCount;
		}

	private:
		HRESULT STDMETHODCALLTYPE Write(const void *data, UInt32 size, UInt32 *processedSize)
		{
			if(size > GetSpaceLeft()) ReallocateDataSpace(size);
			memcpy((char*)m_pData+GetSpaceUsed(),data,size);
			GetSpaceUsed() += size;
			*processedSize = size;
			return S_OK;
		}

	private:
		void*	m_pData;
		size_t	m_iDataSize;
		size_t	m_iDataPos;

	private:
		size_t GetGranularity()
		{
			return (size_t)1<<20;
		}
		size_t GetSpaceLeft()
		{
			//如果m_iDataPos == m_iDataSize
			//结果正确, 但貌似表达式错误?
			// -1 + 1 = 0, size_t 得不到-1, 0x~FF + 1,也得0
			return m_iDataSize-1 - m_iDataPos + 1;
		}
		size_t& GetSpaceUsed()
		{
			//结束-开始+1
			//return m_iDataPos-1 -0 + 1;
			return m_iDataPos;
		}

		void ReallocateDataSpace(size_t szAddition)
		{
			//只有此条件才会分配空间,也即调用此函数前必须检测空间剩余
			assert(GetSpaceLeft()<szAddition);

			//计算除去剩余空间后还应增加的空间大小
			size_t left = szAddition - GetSpaceLeft();

			//按分配粒度计算块数及剩余
			size_t nBlocks = left / GetGranularity();
			size_t cbRemain = left - nBlocks*GetGranularity();
			if(cbRemain) ++nBlocks;

			//计算新空间并重新分配
			size_t newDataSize = m_iDataSize + nBlocks*GetGranularity();
			void*  pData = (void*)new char[newDataSize];

			//复制原来的数据到新空间
			//memcpy支持第3个参数传0吗?
			if(GetSpaceUsed()) memcpy(pData,m_pData,GetSpaceUsed());

			if(m_pData) delete[] m_pData;
			m_pData = pData;
			m_iDataSize = newDataSize;
		}

	public:
		OutMemStream():
			m_refCount(0),
			m_pData(0),
			m_iDataSize(0),
			m_iDataPos(0)
		{}
		~OutMemStream()
		{
			if(m_pData){
				delete[] m_pData;
				m_pData = 0;
			}
		}
		void* GetDataPtr()
		{
			return m_pData;
		}
		size_t GetDataSize()
		{
			return GetSpaceUsed();
		}
	};
private:
	ULONG m_refCount;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID,void**)
	{
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++m_refCount;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return --m_refCount;
	}

private:
	HRESULT STDMETHODCALLTYPE PrepareOperation(Int32)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetTotal(UInt64)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetCompleted(const UInt64*)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetOperationResult(Int32)
	{
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetStream(UInt32 id,ISequentialOutStream** ptr,Int32 mode)
	{
		switch (mode)
		{
		case NArchive::NExtract::NAskMode::kExtract:
		case NArchive::NExtract::NAskMode::kTest:

			if (/*id != m_index || */ptr == NULL)
				return S_FALSE;
			else
				*ptr = &m_OutStream;

		case NArchive::NExtract::NAskMode::kSkip:
			return S_OK;

		default:
			return E_INVALIDARG;
		}
	}


public:
	OutMemStream& GetStream(){return m_OutStream;}
private:
	OutMemStream m_OutStream;
	//UInt32		 m_index;

public:
	AExtractCallback():
		m_refCount(0)
		//m_index(0)
	{

	}
};

class LibRef
{
public:
	HMODULE hMod;
	LibRef(std::string lib)
	{
		hMod = ::LoadLibrary(lib.c_str());
	}
	~LibRef()
	{
		if(hMod) ::FreeLibrary(hMod);
	}

protected:
	void* GetProcAddress(const char* proc)
	{
		return ::GetProcAddress(hMod,proc);
	}
};

class LibRef7Zip : public LibRef
{
private:
	typedef UINT32  (__stdcall* CreateObjectFunc)(const GUID* guidin,const GUID* guidout,void** ppOut);
	typedef HRESULT (__stdcall* GetNumberOfFormatsFunc)(UINT32* pnumFormats);
	typedef HRESULT (__stdcall* GetHandlerProperty2Func)(UInt32 formatIndex, PROPID propID, PROPVARIANT* value);

	struct FormatRecord
	{
		std::vector<char> signature;
		GUID guid;

	public:
		FormatRecord(){}
	};

private:
	bool GetInterface()
	{
		CreateObject = (CreateObjectFunc)GetProcAddress("CreateObject");
		GetNumberOfFormats = (GetNumberOfFormatsFunc)GetProcAddress("GetNumberOfFormats");
		GetHandlerProperty2 = (GetHandlerProperty2Func)GetProcAddress("GetHandlerProperty2");

		return CreateObject && GetNumberOfFormats && GetHandlerProperty2;
	}
	void GetFormatRecords()
	{
		UInt32 nFormats;
		GetNumberOfFormats(&nFormats);

		for(UInt32 i=0; i<nFormats; i++){
			PROPVARIANT prop;
			prop.vt = VT_EMPTY;

			GetHandlerProperty2(i,NArchive::kStartSignature,&prop);

			FormatRecord fr;
			UInt32 len;

			len = ::SysStringLen(prop.bstrVal);
			for(UInt32 x=0; x<len; x++){
				fr.signature.push_back(((char*)prop.bstrVal)[x]);
			}

			GetHandlerProperty2(i,NArchive::kClassID,&prop);
			memcpy(&fr.guid,prop.bstrVal,16);

			this->formatRecords.push_back(fr);

			::VariantClear(reinterpret_cast<VARIANTARG*>(&prop));
		}
	}


public:
	LibRef7Zip(std::string lib) : 
		LibRef(lib),
		CreateObject(0),
		GetNumberOfFormats(0),
		GetHandlerProperty2(0)
	{
		if(hMod){
			GetInterface();
			GetFormatRecords();
		}
	}

public:
	bool bstr2string(BSTR bstr,std::string* result)
	{
		std::wstring tws = bstr;
		int buflen = (tws.size()+1)*2;
		char* buf = new char[buflen];
		int ret = ::WideCharToMultiByte(CP_ACP,0,tws.c_str(),tws.size(),buf,buflen,0,0);
		if(ret == 0){
			delete[] buf;
			return false;
		}

		buf[ret] = 0;
		*result = buf;
		delete[] buf;
		return true;
	}
public:
	CreateObjectFunc			CreateObject;
	GetNumberOfFormatsFunc		GetNumberOfFormats;
	GetHandlerProperty2Func		GetHandlerProperty2;
	
public:
	std::vector<FormatRecord>	formatRecords;
};


static LibRef7Zip* lib7z;

bool initArchiveSystem()
{
	lib7z = new LibRef7Zip("7z.dll");
	return true;
}

MEM_FILE* GetFirstNesFileFromArchive(std::string fname)
{
	FILE* fp = 0;
	void* filebuf = 0;
	size_t filesize=0;

	try{
		fp = fopen(fname.c_str(),"rb");
		if(fp==NULL) throw "无法打开档案文件";

		fseek(fp,0,SEEK_END);
		filesize = ftell(fp);
		if(filesize > (1<<20)*8)	//8MB
			throw "我从未见过这么大的档案文件,难道你见过?";
		else if(filesize < 1<<10)	//1KB
			throw "我从未见过如此小的档案文件,难道你见过?";

		filebuf = (void*)new char[filesize];
		fseek(fp,0,SEEK_SET);
		if(fread(filebuf,1,filesize,fp) != filesize)
			throw "不知道为什么, 文件没有被正确读取!";
		fclose(fp);

		//判断是否是NES文件
		if(memcmp(filebuf,"NES\x1A",4)==0){
			MEM_FILE* mf = new MEM_FILE;
			mf->fname = fname;
			mf->fsize = filesize;
			mf->fdata = filebuf;
			return mf;
		}
	}
	catch(const char*){
		if(fp) fclose(fp);
		if(filebuf) delete[] filebuf;

		throw;
	}
	
	int matchingFormat = -1;
	for(UInt32 i=0; i<(int)lib7z->formatRecords.size(); i++){
		int size = (int)lib7z->formatRecords[i].signature.size();
		if(size == 0) continue;	//真tm扯淡,居然还有没有文件头的归档文件
		if(memcmp(&lib7z->formatRecords[i].signature[0],filebuf,size)==0){
			matchingFormat = i;
			break;
		}
	}
	
	if(matchingFormat == -1)
		throw "7zip这么强大的库都没有打开你的归档文件, 你能告诉我为什么吗?";
	
	IInArchive* archive=0;
	try{
		if(FAILED(lib7z->CreateObject(&lib7z->formatRecords[matchingFormat].guid,&IID_IInArchive,(void**)&archive)))
			throw "你的7zip貌似有问题哦~";

		AInMemStream ims(filebuf,filesize);
		ims.rewind();
		if(FAILED(archive->Open(&ims,0,0)))
			throw "7zip无法打开此归档文件~";

		UInt32 nFiles;
		if(FAILED(archive->GetNumberOfItems(&nFiles)))
			throw "7zip无法取得文件数目~";

		int nesIndex=-1;
		std::string nesName;

		for(UInt32 i=0; i<nFiles; i++){
			PROPVARIANT prop;
			prop.vt = VT_EMPTY;

			if(FAILED(archive->GetProperty(i,kpidIsFolder,&prop)))
				throw "7zip无法取得属性值!";

			if(prop.boolVal == VARIANT_TRUE)
				continue;

			if(FAILED(archive->GetProperty(i,kpidPath,&prop)))
				throw "7zip获取文件名失败!";

			std::string path;
			lib7z->bstr2string(prop.bstrVal,&path);

			_strlwr((char*)path.c_str());
			if(path.find(".nes") == path.size()-4){
				nesIndex = i;
				nesName = path;
				break;
			}

			VariantClear(reinterpret_cast<VARIANTARG*>(&prop));
		}

		if(nesIndex == -1)
			throw "归档文件中没有Nes文件!";

		//ims.rewind();
		AExtractCallback extract;
		UInt32 indices[1] = {nesIndex};
		if(FAILED(archive->Extract(&indices[0],1,0,&extract)))
			throw "7zip提取文件时出错!";

		MEM_FILE* mf = new MEM_FILE;
		mf->fname = nesName;
		mf->fsize = extract.GetStream().GetDataSize();
		mf->fdata = (void*)new char[mf->fsize];
		memcpy(mf->fdata,extract.GetStream().GetDataPtr(),mf->fsize);

		//去除文件中的路径
		std::string::size_type pos = mf->fname.find_last_of('\\');
		if(pos==std::string::npos)
			pos = mf->fname.find_last_of('/');

		if(pos != std::string::npos){
			mf->fname = mf->fname.substr(pos+1);
		}

		archive->Release();
		delete[] filebuf;
		return mf;
	}
	catch(const char*){
		if(archive)	archive->Release();
		if(filebuf) delete[] filebuf;

		throw;
	}
}

void CalcMd5(void* pdata,size_t cbdata,unsigned char md5[32])
{
	md5_byte_t digest[16];
	md5_state_t state;
	md5_byte_t result[33];

	md5_init(&state);
	md5_append(&state,(md5_byte_t*)pdata,cbdata);
	md5_finish(&state,&digest[0]);

	for(int i=0; i<16; i++){
		sprintf((char*)&result[i<<1],"%02X",digest[i]);
	}
	memcpy(md5,result,32);
}

void CalcCrc32(void* pdata,size_t cbdata,unsigned char crc32[8])
{
	char c[9];
	unsigned long c32;
	Crc32(~0,cbdata,(void*)pdata,&c32);
	sprintf(c,"%08X",~c32);
	memcpy(crc32,c,8);
}

void GetFileListFromOFN(std::vector<std::string>* files,const char* fs)
{
	std::string dir = fs;
	const char* p=fs;
	
	while(*p++)
		;
	if(!*p){//只有一个文件
		files->push_back(dir);
	}else{
		do{
			std::string afile = dir + '\\';
			afile += p;
			files->push_back(afile);

			while(*p++)
				;
		}while(*p);
	}
}