#include "EditBox.h"
//#include "Utils.h"
//#include "resource.h"
//# 
#include "../../../res/resource.h"

AEditBox::AEditBox(void):
	m_hFont(NULL)
{

}


AEditBox::~AEditBox(void)
{
	::DeleteObject(m_hFont);// NULL?
}

void AEditBox::DoCopy()
{
	SendMessage(WM_COPY);
}
void AEditBox::DoCut()
{
	SendMessage(WM_CUT);
}
void AEditBox::DoPaste()
{
	SendMessage(WM_PASTE);
}
void AEditBox::DoDelete()
{
	SendMessage(WM_CLEAR);
}
void AEditBox::DoSelAll()
{
	int len = this->GetWindowTextLength();
	Edit_SetSel(this->GetHwnd(),0,len);
}
void AEditBox::DoUndo()
{
	SendMessage(WM_UNDO);
}

BOOL AEditBox::GetModify()
{
	return SendMessage(EM_GETMODIFY)!=0;
}

void AEditBox::SetModify(BOOL bModified)
{
	SendMessage(EM_SETMODIFY,bModified);
}

DWORD AEditBox::GetSel(DWORD* dwStart,DWORD* dwEnd)
{
	(DWORD)SendMessage(EM_GETSEL,WPARAM(dwStart),LPARAM(dwEnd));
	return *dwEnd-*dwStart;
}

BOOL AEditBox::CanUnDo()
{
	return (BOOL)SendMessage(EM_CANUNDO);
}

//bugbug:未删除原来的字体
void AEditBox::DoChooseFont()
{
	LOGFONT lf={0};
	CHOOSEFONT cf={0};

	HFONT hFont = (HFONT)SendMessage(WM_GETFONT);
	GetObject(hFont,sizeof(lf),(void*)&lf);
	//这里是为什么?
	if(strcmp(lf.lfFaceName,"MS Shell Dlg 2")==0)
		*lf.lfFaceName = 0;

	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = GetHwnd();
	cf.lpLogFont = &lf;
	cf.Flags = CF_INITTOLOGFONTSTRUCT
		| CF_SCREENFONTS
		| CF_EFFECTS;
	cf.nFontType = SCREEN_FONTTYPE;
	if(ChooseFont(&cf)){
		m_hFont = CreateFontIndirect(&lf);
		if(m_hFont){
			SendMessage(WM_SETFONT,(WPARAM)m_hFont,MAKELPARAM(TRUE,0));
		}
	}
}

INT_PTR AEditBox::OnContextMenu(HWND hWnd,int x,int y)
{
	HMENU hMenu = ::LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_MENU_EDITBOX));
	hMenu = ::GetSubMenu(hMenu,0);
	assert(hMenu != NULL);

	this->SetFocus();

	DWORD iStart,iEnd;
	DWORD len = GetSel(&iStart,&iEnd);
	EnableMenuItem(hMenu,IDM_EDITBOX_COPY,		len?MF_ENABLED:(MF_DISABLED|MF_GRAYED));
	EnableMenuItem(hMenu,IDM_EDITBOX_CUT,		len?MF_ENABLED:(MF_DISABLED|MF_GRAYED));
	EnableMenuItem(hMenu,IDM_EDITBOX_DELETE,	len?MF_ENABLED:(MF_DISABLED|MF_GRAYED));
	EnableMenuItem(hMenu,IDM_EDITBOX_PASTE,		IsClipboardFormatAvailable(CF_TEXT)?MF_ENABLED:(MF_DISABLED|MF_GRAYED));
	EnableMenuItem(hMenu,IDM_EDITBOX_UNDO,		CanUnDo()?MF_ENABLED:(MF_DISABLED|MF_GRAYED));
	EnableMenuItem(hMenu,IDM_EDITBOX_SELALL,	GetWindowTextLength()?MF_ENABLED:(MF_DISABLED|MF_GRAYED));

	//传递给父窗口,看是否需要扩展菜单
	ControlMessage cm = {0};
	cm.self = this;
	cm.uMsg = WM_CONTEXTMENU;
	cm.wParam = 0;
	cm.lParam = (LPARAM)hMenu;
	this->NotifyParent(&cm);

	TrackPopupMenu(hMenu,TPM_LEFTALIGN,x,y,0,this->GetHwnd(),NULL);
	return 0;
}

INT_PTR AEditBox::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(codeNotify==0 && hWndCtrl==NULL){
		switch(ctrlID)
		{
		case IDM_EDITBOX_COPY:	DoCopy();break;
		case IDM_EDITBOX_CUT:	DoCut();break;
		case IDM_EDITBOX_DELETE:DoDelete();break;
		case IDM_EDITBOX_PASTE:	DoPaste();break;
		case IDM_EDITBOX_UNDO:	DoUndo();break;
		case IDM_EDITBOX_SELALL:DoSelAll();break;
		case IDM_EDITBOX_FONT:	DoChooseFont();break;
		default:
			{
				ControlMessage cm = {0};
				cm.self = NULL;
				cm.uMsg = WM_CONTEXTMENU;
				cm.lParam = ctrlID;
				return NotifyParent(&cm);
			}
		}
	}
	return 0;
}

INT_PTR AEditBox::OnDropFiles(HDROP hDrop)
{
	ControlMessage cm = {0};
	cm.self = this;
	cm.uMsg = WM_DROPFILES;
	cm.lParam = LPARAM(hDrop);
	return NotifyParent(&cm);
}

INT_PTR AEditBox::OnChar(int key)
{
// 	if(key==VK_RETURN||key==VK_TAB){
// 		ControlMessage cm = {0};
// 		cm.self = this;
// 		cm.uMsg = WM_CHAR;
// 		cm.wParam = key;
// 		cm.lParam = m_lParam;
// 		return NotifyParent(&cm);
// 	}
	return DODEFAULT;
}

INT_PTR AEditBox::OnGetDlgCode(WPARAM vk,MSG* msg)
{
	return DLGC_WANTALLKEYS;
}