#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#include <windows.h>

class AClipboard
{
private:
	const char* const m_fmtname;
	UINT m_fmtid;
public:
	AClipboard() :
		m_fmtname("ndb_private_clipboard")
	{
		m_fmtid = ::RegisterClipboardFormat(m_fmtname);
	}
	~AClipboard()
	{
		
	}
	bool IsAvailable()
	{
		return ::IsClipboardFormatAvailable(m_fmtid)!=0;
	}
	UINT GetID() const {return m_fmtid;}
};

extern AClipboard clip;

#endif // !__CLIPBOARD_H__
