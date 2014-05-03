#include <windows.h>
#include <vector>

using namespace std;

class LayoutItem
{
public:
	enum Align{
		kLEFT		=0x00000001,
		kTOP		=0x00000002,
		kRIGHT		=0x00000004,
		kBOTTOM		=0x00000008,
		kRisizeHorz	=0x00000010,
		kRisizeVert	=0x00000020
	};
	LayoutItem(int id, HWND hParent,Align align)
	{
		m_hParent = hParent;
		m_hWnd = GetDlgItem(m_hParent,id);
		GetBorder(hParent);
		SetAlign(align);
	}
	LayoutItem(HWND hWnd,HWND hParent,Align align)
	{
		m_hParent = hParent;
		m_hWnd = hWnd;
		GetBorder(hParent);
		SetAlign(align);
	}

	void SetAlign(Align align)
	{
		m_Align = align;

		m_rcAlign.left=m_rcAlign.top=m_rcAlign.right=m_rcAlign.bottom=0;

		RECT rcP,rcC;
		GetWindowRect(m_hParent,&rcP);
		GetWindowRect(m_hWnd,&rcC);

		m_rcAlign.left = rcC.left-rcP.left-m_border.cx;
		m_rcAlign.top = rcC.top-rcP.top-m_border.cy;
		m_rcAlign.right = (rcP.right-rcP.left)-(rcC.right-rcC.left)-(rcC.left-rcP.left);
		m_rcAlign.bottom = (rcP.bottom-rcP.top)-(rcC.bottom-rcC.top)-(rcC.top-rcP.top);

	}

	void UpdatePos()
	{
		RECT rcP,rcC;
		GetWindowRect(m_hParent,&rcP);
		GetWindowRect(m_hWnd,&rcC);

		int wP = rcP.right-rcP.left;
		int hP = rcP.bottom-rcP.top;
		int wC = rcC.right-rcC.left;
		int hC = rcC.bottom-rcC.top;

		RECT rcNew = {0};

		if(m_Align & Align::kLEFT){
			rcNew.left = m_rcAlign.left;
		}
		if(m_Align & Align::kTOP){
			rcNew.top = m_rcAlign.top;
		}
		if(m_Align & Align::kRIGHT){
			rcNew.left = wP - (wC+m_rcAlign.right) - m_border.cx;
		}
		if(m_Align& Align::kBOTTOM){
			rcNew.top = hP - (hC+m_rcAlign.bottom) - m_border.cy;
		}
		rcNew.right = rcNew.left + wC;
		rcNew.bottom = rcNew.top + hC;

		if(m_Align & (Align::kTOP|Align::kLEFT)){
			RECT rc;
			GetClientRect(m_hParent,&rc);
			if(m_Align & (Align::kRisizeHorz)){
				rcNew.right = rcNew.left + (rc.right - rcNew.left - m_rcAlign.right);
			}
			if(m_Align & (Align::kRisizeVert)){
				rcNew.bottom = rcNew.top + (rc.bottom - rcNew.top - m_rcAlign.bottom);
			}
		}

		if(rcNew.left<0
			|| rcNew.top < 0
			|| rcNew.right-rcNew.left < 0
			|| rcNew.bottom-rcNew.top <0 )
		{
			return;
		}

		SetWindowPos(m_hWnd,HWND_TOP,rcNew.left,rcNew.top,rcNew.right-rcNew.left,rcNew.bottom-rcNew.top,SWP_NOZORDER);
		SetFocus(SetFocus(m_hWnd));
	}

	void GetBorder(HWND hParent)
	{
		RECT rcP,rcC;
		int hCap;
		hCap = GetSystemMetrics(SM_CYCAPTION);
		if(GetMenu(hParent)) hCap += GetSystemMetrics(SM_CYMENU);
		GetWindowRect(hParent,&rcP);
		GetClientRect(hParent,&rcC);
		m_border.cx = ((rcP.right-rcP.left)-rcC.right)/2;
		m_border.cy = (rcP.bottom-rcP.top-rcC.bottom-hCap)/2 + hCap;
		return;
	}

private:
	HWND m_hWnd;		//子窗口句柄
	HWND m_hParent;		//父窗口句柄
	Align m_Align;		//对齐方式
	RECT m_rcAlign;
	SIZE m_border;
};

class CLayout
{
public:
	CLayout(){}
	~CLayout(){Empty();}

	void Empty()
	{
		for(auto item=m_items.begin(); item!=m_items.end();item++){
			delete *item;
		}
	}
	void Add(int id, HWND hParent,LayoutItem::Align align)
	{
		m_items.push_back(new LayoutItem(id,hParent,align));
	}
	void Add(HWND hWnd, HWND hParent,LayoutItem::Align align)
	{
		m_items.push_back(new LayoutItem(hWnd,hParent,align));
	}
	void SizeItems()
	{
		for(auto item=m_items.begin(); item!=m_items.end();item++){
			(*item)->UpdatePos();
		}
	}
private:
	vector<LayoutItem*> m_items;
};

