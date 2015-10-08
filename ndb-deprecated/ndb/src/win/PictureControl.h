#ifndef __PICTURE_CONTROL_H__
#define __PICTURE_CONTROL_H__

#include <windows.h>
#include <ObjIdl.h>

#ifndef _UNICODE
#define PICTURE_CONCTROL_CLASS		"女孩不哭的图像控件"
#else
#define PICTURE_CONCTROL_CLASS		L"女孩不哭的图像控件"
#endif

enum{
	PCM_SETIMAGE = WM_USER+0,
	/*
		WPARAM:wchar_t 图片路径
		LPARAM:(not used)
	*/
	PCM_SETSTREAM,
};

extern "C" {

void PictureControl_RegisterClass(void);

}

class PCMemStream : public IStream
{
public:
	ULONG m_refCount;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject)
	{
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return (ULONG)InterlockedIncrement(&m_refCount);
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		return (ULONG)InterlockedDecrement(&m_refCount);
	}
public:
	PCMemStream(void* pData,size_t size) : 
		m_refCount(0),
		m_szPos(0),
		m_szSize(0)
	{
		m_pData  = pData;
		m_szSize = size;
		m_szPos  = 0;
	}
	~PCMemStream()
	{
		//if(m_pData) delete[] m_pData;
	}
private:
	size_t			m_szPos;
	size_t			m_szSize;
	void*			m_pData;

public:
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Seek( 
		/* [in] */ LARGE_INTEGER dlibMove,
		/* [in] */ DWORD dwOrigin,
		/* [annotation] */ 
		_Out_opt_  ULARGE_INTEGER *plibNewPosition)
	{
		size_t posnew;
		if(dwOrigin == STREAM_SEEK_SET){
			posnew = 0;
		}else if(dwOrigin == STREAM_SEEK_CUR){
			posnew = m_szPos;
		}else if(dwOrigin == STREAM_SEEK_END){
			posnew = m_szSize-1;
		}
		posnew += dlibMove.LowPart;

		if(plibNewPosition){
			plibNewPosition->QuadPart = posnew;
		}
		m_szPos = posnew;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE SetSize( 
		/* [in] */ ULARGE_INTEGER libNewSize)
	{
		return S_OK;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE CopyTo( 
		/* [annotation][unique][in] */ 
		_In_  IStream *pstm,
		/* [in] */ ULARGE_INTEGER cb,
		/* [annotation] */ 
		_Out_opt_  ULARGE_INTEGER *pcbRead,
		/* [annotation] */ 
		_Out_opt_  ULARGE_INTEGER *pcbWritten)
	{
		return E_INVALIDARG;
	}

	virtual HRESULT STDMETHODCALLTYPE Commit( 
		/* [in] */ DWORD grfCommitFlags)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Revert( void)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE LockRegion( 
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE UnlockRegion( 
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Stat( 
		/* [out] */ __RPC__out STATSTG *pstatstg,
		/* [in] */ DWORD grfStatFlag)
	{
		pstatstg->cbSize.QuadPart = m_szSize;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone( 
		/* [out] */ __RPC__deref_out_opt IStream **ppstm)
	{
		return E_NOTIMPL;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Read( 
		/* [annotation] */ 
		_Out_writes_bytes_to_(cb, *pcbRead)  void *pv,
		/* [annotation][in] */ 
		_In_  ULONG cb,
		/* [annotation] */ 
		_Out_opt_  ULONG *pcbRead)
	{
		if(m_szPos + cb > m_szSize-1){
			cb = (m_szSize-1) - m_szPos + 1;
		}

		if(cb==0 || m_szPos>(long)m_szSize-1){
			return E_INVALIDARG;
		}

		memcpy(pv,(char*)m_pData+m_szPos,cb);
		if(pcbRead) *pcbRead= cb;

		m_szPos += cb;

		return S_OK;
	}

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Write( 
		/* [annotation] */ 
		_In_reads_bytes_(cb)  const void *pv,
		/* [annotation][in] */ 
		_In_  ULONG cb,
		/* [annotation] */ 
		_Out_opt_  ULONG *pcbWritten)
	{
		return E_NOTIMPL;
	}
};

#endif
