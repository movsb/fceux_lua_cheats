#include "CheatInfoDlg.h"
#include "../../res/resource.h"
#include "../tinyxml/tinyxml.h"
#include <CommCtrl.h>
#include "EditBox.h"
#include "ListView.h"
#include "TreeView.h"
//#include "../Str.h"
#include "../debug.h"
#include <string>
#include <vector>
#include <sstream>
#include "CheatCustomDlg.h"
#include "CheatCheatInfoDlg.h"

//#include "../lua5.2/include/lua.hpp"
//#pragma comment(lib,"C:\\Users\\YangTao\\Desktop\\ndb\\ndb\\src\\lua5.2\\lua52.lib")
//# 

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#define __OUT__
#include "E:\\Program\\FC\\fceux-2.2.2\\src\\drivers\\win\\CheatInterface.h"

#include "EditCheatDlg.h"

#include "Clipboard.h"
#include "App.h"



extern HWND MainhWnd;
HWND hThisDlg;
AWindowBase* ThisDlg;

/**************************************************
��  ��:boolean pause([boolean new_state])
��  ��:����ģ��������״̬
��  ��:new_state ��ѡ:
			true:��ͣ
			false:����
��  ��:���ص���ǰ��ģ����״̬
˵  ��:
**************************************************/
extern "C" static int fceu_pause(lua_State* L)
{
	int args = lua_gettop(L);
	if(args == 0){
		lua_pushboolean(L,SendMessage(MainhWnd,CI_PAUSEEMU,-1,0)!=0);
		return 1;
	}else{
		int old=SendMessage(MainhWnd,CI_PAUSEEMU,lua_toboolean(L,1),0)!=0;
		lua_pushboolean(L,old);
		return 1;
	}
}

/**************************************************
��  ��:lua_Integer read(lua_Integer addr [,lua_Integer count])
��  ��:��NES�ڴ�,��ΧΪ0x0000~0xFFFF
��  ��:	addr - ��ȡ�ڴ����ʼ��ַ
		count ��ѡ - �ֽ���,���ܶ���16��
��  ��:	��δָ���ֽ���,����addr��ַ��һ���ֽ�
		���򷵻�countָ���Ĵ�addr��ʼ�������ֽ�������ֵַ
˵  ��:
**************************************************/
extern "C" static int fceu_read(lua_State* L)
{
	unsigned char args;
	unsigned short addr;
	unsigned char  value;
	unsigned char  count;

	args = lua_gettop(L);

	addr = (unsigned short)luaL_checkinteger(L,1);

	if(args>1)	count = luaL_checkinteger(L,2);
	else		count = 1;

	for(args=0; args<count; args++){
		value = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr,0);
		lua_pushinteger(L,value);
		addr++;
	}
	
	return count;
}

extern "C" static int fceu_rd2b(lua_State* L)
{
	unsigned short addr;
	unsigned char v[2];

	addr = (unsigned short)luaL_checkinteger(L,1);

	v[0] = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr++,0);
	v[1] = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr++,0);

	lua_pushinteger(L,*(unsigned short*)&v[0]);

	return 1;
}

extern "C" static int fceu_rd4b(lua_State* L)
{
	unsigned short addr;
	unsigned char v[4];

	addr = (unsigned short)luaL_checkinteger(L,1);

	v[0] = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr++,0);
	v[1] = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr++,0);
	v[2] = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr++,0);
	v[3] = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr++,0);

	lua_pushinteger(L,*(unsigned int*)&v[0]);

	return 1;
}

/**************************************************
��  ��:write(lua_Integer addr,lua_Integer value [,value1 [,value2 [,...]]])
��  ��:дnes�ڴ�
��  ��:	addr - д���ڴ����ʼ��ַ
		value, value1, value2, ... - ��д����ڴ���ֽ�ֵ,���ö���16��
��  ��:
˵  ��:
**************************************************/
extern "C" static int fceu_write(lua_State* L)
{
	unsigned char args;
	unsigned short addr;
	unsigned char  value;
	unsigned char  count;

	args = lua_gettop(L);

	addr = (unsigned short)luaL_checkinteger(L,1);

	for(count=2; count<=args; count++){
		value = (unsigned char)luaL_checkinteger(L,count);
		SendMessage(MainhWnd,CI_WRITEMEMORY,addr,value);
		addr ++;
	}

	return 0;
}

extern "C" static int fceu_wr2b(lua_State* L)
{
	unsigned short A;
	unsigned short V;
	unsigned char VA[2];
	A = (unsigned short)luaL_checkinteger(L,1);
	V = (unsigned short)luaL_checkinteger(L,2);
	*(unsigned short*)&VA[0] = V;
	SendMessage(MainhWnd,CI_WRITEMEMORY,A++,VA[0]);
	SendMessage(MainhWnd,CI_WRITEMEMORY,A++,VA[1]);
	return 0;
}

extern "C" static int fceu_wr4b(lua_State* L)
{
	unsigned short A;
	unsigned int   V;
	unsigned char VA[4];
	A = (unsigned short)luaL_checkinteger(L,1);
	V = (unsigned int)luaL_checkinteger(L,2);
	*(unsigned int*)&VA[0] = V;
	SendMessage(MainhWnd,CI_WRITEMEMORY,A++,VA[0]);
	SendMessage(MainhWnd,CI_WRITEMEMORY,A++,VA[1]);
	SendMessage(MainhWnd,CI_WRITEMEMORY,A++,VA[2]);
	SendMessage(MainhWnd,CI_WRITEMEMORY,A++,VA[3]);
	return 0;
}


extern "C" static int fceu_memset(lua_State* L)
{
	unsigned short addr = (unsigned short)luaL_checkinteger(L,1);
	unsigned char  mask = (unsigned char) luaL_checkinteger(L,2);
	unsigned char value = (unsigned char) luaL_checkinteger(L,3);

	unsigned char t = (unsigned char)SendMessage(MainhWnd,CI_READMEMORY,addr,0);
	t &= ~mask;
	t |= value & mask;
	SendMessage(MainhWnd,CI_WRITEMEMORY,addr,t);
	return 0;
}

/**************************************************
��  ��:lock(lua_Integer addr,value1, [value2 [,...]])
��  ��:�����ڴ�ֵΪ����ֵ
��  ��:	addr - ��ʼ�������ڴ��ַ
		valuex - �������ֽ�ֵ,���ö���16��
��  ��:
˵  ��:�޸�:���������ڴ�������ǰ�ᱻ����
**************************************************/
extern "C" static int fceu_lock(lua_State* L)
{
	unsigned char args;
	unsigned short addr;
	unsigned char  value;
	unsigned char  count;

	args = lua_gettop(L);
	addr = (unsigned short)luaL_checkinteger(L,1);

	for(count=2; count<=args; count++){
		value = (unsigned char) luaL_checkinteger(L,count);
		SendMessage(MainhWnd,CI_UNLOCKMEMORY,addr,0);
		SendMessage(MainhWnd,CI_LOCKMEMORY,addr,value);
	}

	return 0;
}

/**************************************************
��  ��:unlock(lua_Integer addr [value1 [,value2 [,...]]])
��  ��:�����ڴ�,д���ڴ�
��  ��:	addr - ��ʼ�������ڴ��ַ
		valuex ��ѡ- ��ָ��,�������д���ڴ��ֽ�ֵ, ������16��
��  ��:
˵  ��:
**************************************************/
extern "C" static int fceu_unlock(lua_State* L)
{
	unsigned char args;
	unsigned short addr;
	unsigned char  value;
	unsigned char  count;

	args = lua_gettop(L);
	addr = (unsigned short)luaL_checkinteger(L,1);

	if(args == 1){
		SendMessage(MainhWnd,CI_UNLOCKMEMORY,addr,0);
		return 0;
	}else{
		for(count=2; count<=args; count++){
			value = (unsigned char)luaL_checkinteger(L,count);
			SendMessage(MainhWnd,CI_UNLOCKMEMORY,addr,0);
			SendMessage(MainhWnd,CI_WRITEMEMORY,addr,value);
			addr++;
		}
		return 0;
	}
}

/**************************************************
��  ��:lua_Integer msgbox([����])
��  ��:������Ϣ�Ի���,��ʾ��Ϣ
��  ��:
		��������������:
			1.msgbox(string text)
				����ʾ�ı�,�޷���ֵ
			2.msgbox(string text,string title)
				��ʾ�ı��ͱ���,�޷���ֵ
			3.msgbox(string text,integer type)
				��ʾ�ı���ָ�����͵İ�ť,�����ذ��°�ť������
				Ԥ����İ�ť���ͼ�ֵ��˵��
			4.msgbox(string text,string title,integer type)
				��ʾ�ı�,���⼰ָ�����͵İ�ť,�����ذ��°�ť������
				Ԥ����İ�ť���ͼ�ֵ��˵��

��  ��:����������
˵  ��:	Ԥ����İ�ť����:
			0 - ȷ��[Ok]
			1 - ȷ��ȡ��[OkCancel]
			2 - �Ƿ�[YesNo]
		��ť����ֵ:
			0 - ȷ��[Ok]
			1 - ȡ��[Cancel]
			0 - ��[Yes]
			1 - ��[No]
**************************************************/
extern "C" static int fceu_msgbox(lua_State* L)
{
	static int btn_type[3] = {MB_OK,MB_OKCANCEL,MB_YESNO};
	//static int btn_ret [4] = {IDOK,IDCANCEL,IDNO,IDYES}; //!!!!

	unsigned char args;
	const char* text=0;
	const char* title=0;
	unsigned char type=0;

	args = lua_gettop(L);

	switch(args)
	{
	case 1:
		text = luaL_checkstring(L,1);
		MessageBox(hThisDlg,text,"",MB_OK);
		return 0;
	case 2:
		text = luaL_checkstring(L,1);
		switch(lua_type(L,2))
		{
		case LUA_TSTRING:
			title = luaL_checkstring(L,2);
			MessageBox(hThisDlg,text,title,MB_OK);
			return 0;
		case LUA_TNUMBER:
			type = luaL_checkinteger(L,2);
			type = MessageBox(hThisDlg,text,"",btn_type[type]);
			//lua_pushinteger(L,btn_ret[type] & 0x01);// & 1 !!!!!
			if(type==IDOK ||type==IDYES) lua_pushinteger(L,0);
			else if(type==IDCANCEL || type==IDNO) lua_pushinteger(L,1);
			else lua_pushinteger(L,-1);
			return 1;
		}
		return 0;//never got here
	case 3:
		text = luaL_checkstring(L,1);
		title = luaL_checkstring(L,2);
		type = luaL_checkinteger(L,3);
		type = MessageBox(hThisDlg,text,title,btn_type[type]);
		//lua_pushinteger(L,btn_ret[type] & 0x01);
		if(type==IDOK ||type==IDYES) lua_pushinteger(L,0);
		else if(type==IDCANCEL || type==IDNO) lua_pushinteger(L,1);
		else lua_pushinteger(L,-1);
		return 1;
	}
	return 0;//never got here
}

/**************************************************
��  ��:boolean,integer dialog(table dlg)
��  ��:��ʾһ������Ի���
��  ��:	table,key-value,����
			text - ��ʾ����
			onok - ����[ȷ��ʱҪ���Ķ���lua����
				����C����ԭ�� int onok(unsigned char val)
				������1:�Ի���ر�,dialog��������true,val
				������0:�Ի��򲻹ر�
��  ��:������
˵  ��:
**************************************************/
extern "C" static int fceu_dialog(lua_State* L)
{
	int args = lua_gettop(L);
	if(args < 1) return 0;

	if(!lua_istable(L,1)){
		MessageBox(hThisDlg,"dialog��������!",0,0);
		lua_pushboolean(L,0);
		return 1;
	}
	
	ACheatCustomDlg dlg(ThisDlg,&hThisDlg,L);

	int code = dlg.GetDlgCode();
	int value = dlg.GetValue();

	lua_pushboolean(L,code!=0);
	lua_pushinteger(L,value);

	return 2;
}

extern "C" static int fceu_btoh(lua_State* L)
{
	unsigned char bcd = (unsigned char)luaL_checkinteger(L,1);
	unsigned char hex = bcd/16*10+bcd%16;
	lua_pushinteger(L,hex);
	return 1;
}

extern "C" static int fceu_htob(lua_State* L)
{
	unsigned char hex = (unsigned char)luaL_checkinteger(L,1);
	unsigned char bcd = hex/10*16+hex%10;
	lua_pushinteger(L,bcd);
	return 1;
}

extern "C" static int fceu_bitand(lua_State* L)
{
	unsigned char op1 = (unsigned char)luaL_checkinteger(L,1);
	unsigned char op2 = (unsigned char)luaL_checkinteger(L,2);
	lua_pushinteger(L,(unsigned char)(op1&op2));
	return 1;
}

extern "C" static int fceu_bitor(lua_State* L)
{
	unsigned char op1 = (unsigned char)luaL_checkinteger(L,1);
	unsigned char op2 = (unsigned char)luaL_checkinteger(L,2);
	lua_pushinteger(L,(unsigned char)(op1|op2));
	return 1;
}

extern "C" static int fceu_bitxor(lua_State* L)
{
	unsigned char op1 = (unsigned char)luaL_checkinteger(L,1);
	unsigned char op2 = (unsigned char)luaL_checkinteger(L,2);
	lua_pushinteger(L,(unsigned char)(op1^op2));
	return 1;
}

extern "C" static int fceu_bitshl(lua_State* L)
{
	unsigned char op1 = (unsigned char)luaL_checkinteger(L,1);
	unsigned char op2 = (unsigned char)luaL_checkinteger(L,2);
	lua_pushinteger(L,(unsigned char)(op1<<op2));
	return 1;
}

extern "C" static int fceu_bitshr(lua_State* L)
{
	unsigned char op1 = (unsigned char)luaL_checkinteger(L,1);
	unsigned char op2 = (unsigned char)luaL_checkinteger(L,2);
	lua_pushinteger(L,(unsigned char)(op1>>op2));
	return 1;
}

extern "C" static int fceu_bitset(lua_State* L)
{
	unsigned char value = (unsigned char)luaL_checkinteger(L,1);
	unsigned char which = (unsigned char)luaL_checkinteger(L,2)/* & 0x07*/;
	unsigned char newva = (unsigned char)luaL_checkinteger(L,3)/* & 0x01*/;

	value &= ~which;
	value |= newva & which;

	lua_pushinteger(L,value);
	return 1;
}

extern "C" static int fceu_bitneg(lua_State* L)
{
	unsigned char value = (unsigned char)luaL_checkinteger(L,1);
	value = ~value;
	lua_pushinteger(L,(unsigned char)value);
	return 1;
}

extern "C" static int fceu_bitunpack(lua_State* L)
{
	unsigned int value = (unsigned int)luaL_checkinteger(L,1);
	int bits = -1;
	if(lua_gettop(L)==2) bits = (int)luaL_checkinteger(L,2) % 10;

	std::vector<int> bitlist;
	while(value){
		bitlist.push_back(value%10);
		value /= 10;
	}

	if(bits!=-1){
		int left = bits - (int)bitlist.size();
		if(left > 0){
			while(left--)
				bitlist.push_back(0);
		}else if(left < 0){
			left = -left;
			while(left--)
				bitlist.pop_back();
		}
	}

	lua_newtable(L);
	int sz = (int)bitlist.size();
	for(int i=1; i<=sz; i++){
		lua_pushinteger(L,bitlist[sz-i]);
		lua_rawseti(L,-2,i);
	}
	return 1;
}

extern "C" static int fceu_bitpack(lua_State* L)
{
	int args = (int)lua_gettop(L);
	if(!args) return 0;

	int i;
	unsigned int tmp=0;
	for(i=1; i<=args; i++){
		tmp = luaL_checkinteger(L,i)%10 + tmp*10;
	}
	lua_pushinteger(L,tmp);
	return 1;
}

ACheatInfoDlg::~ACheatInfoDlg()
{
	delete m_editName;
	delete m_CheatsList;
	lua_close((lua_State*)luastate);
}

ACheatInfoDlg::ACheatInfoDlg(AWindowBase* parent)
{
	m_bDragging = false;

	m_editName = new AEditBox;
	m_CheatsList = new AListView;

	luastate = luaL_newstate();
	lua_State* L = (lua_State*)luastate;
	luaL_openlibs(L);
	lua_register(L,"lock",fceu_lock);
	lua_register(L,"unlock",fceu_unlock);
	lua_register(L,"msgbox",fceu_msgbox);
	lua_register(L,"pause",fceu_pause);
	lua_register(L,"write",fceu_write);
	lua_register(L,"wr2b",fceu_wr2b);
	lua_register(L,"wr4b",fceu_wr4b);
	lua_register(L,"read",fceu_read);
	lua_register(L,"rd2b",fceu_rd2b);
	lua_register(L,"rd4b",fceu_rd4b);
	lua_register(L,"memset",fceu_memset);
	lua_register(L,"dialog",fceu_dialog);
	lua_register(L,"htob",fceu_htob);
	lua_register(L,"btoh",fceu_btoh);
	lua_register(L,"bitand",fceu_bitand);
	lua_register(L,"bitor",fceu_bitor);
	lua_register(L,"bitxor",fceu_bitxor);
	lua_register(L,"bitshl",fceu_bitshl);
	lua_register(L,"bitshr",fceu_bitshr);
	lua_register(L,"bitset",fceu_bitset);
	lua_register(L,"bitneg",fceu_bitneg);
	lua_register(L,"bitpack",fceu_bitpack);
	lua_register(L,"bitunpack",fceu_bitunpack);

	this->SetParent(parent);
	CreateDialog(GetModuleHandle(0),MAKEINTRESOURCE(IDD_CHEAT_INFO),parent->GetHwnd(),(DLGPROC)GetWindowThunk());
}

INT_PTR ACheatInfoDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR ACheatInfoDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;

	m_editName->AttachCtrl(this,IDC_CHEAT_CHEAT_NAME);

	m_CheatsList->AttachCtrl(this,IDC_CHEAT_CHEATSLIST);

	m_editName->SubClass();

	InitControls();

	hThisDlg = hWnd;
	ThisDlg = this;

	return TRUE;
}

INT_PTR ACheatInfoDlg::OnNotify(LPNMHDR phdr)
{
	if(phdr->idFrom == m_CheatsList->GetCtrlID()){
		if(phdr->code == LVN_BEGINDRAG){
			int iItem = ((NMLISTVIEW*)phdr)->iItem;
			POINT ptStart;
			HIMAGELIST hImg = ListView_CreateDragImage(m_CheatsList->GetHwnd(),iItem,&ptStart);
			ImageList_BeginDrag(hImg,0,0,0);
			ImageList_DragEnter(m_CheatsList->GetHwnd(),ptStart.x,ptStart.y);
			m_iDraggingItem = iItem;
			SetCapture(this->GetHwnd());
			m_bDragging = true;
			return SetDlgResult(0);
		}else if(phdr->code == NM_DBLCLK){
			int isel = m_CheatsList->GetNextItem(-1,LVNI_SELECTED);
			if(isel==-1) return SetDlgResult(0);
			SendMessage(WM_COMMAND,MAKEWPARAM(IDC_CHEAT_EXECLUA,BN_CLICKED),
				(LPARAM)GetDlgItem(this->GetHwnd(),IDC_CHEAT_EXECLUA));
			return SetDlgResult(0);
		}else if(phdr->code == NM_RCLICK){
			//std::vector<int> selection;
			POINT pt;
			::GetCursorPos(&pt);
			HMENU hMenu = ::GetSubMenu(::LoadMenu(theApp->GetAppInstance(),MAKEINTRESOURCE(IDR_MENU_CHEATVALUES)),0);

			int i = m_CheatsList->GetNextItem(-1,LVNI_SELECTED);
			::EnableMenuItem(hMenu,IDM_CHEATVALUES_COPY,MF_BYCOMMAND|(i!=-1?MF_ENABLED:MF_DISABLED|MF_GRAYED));
			::EnableMenuItem(hMenu,IDM_CHEATVALUES_CUT,MF_BYCOMMAND|(i!=-1?MF_ENABLED:MF_DISABLED|MF_GRAYED));
			::EnableMenuItem(hMenu,IDM_CHEATVALUES_PASTE,MF_BYCOMMAND|(clip.IsAvailable()?MF_ENABLED:MF_DISABLED|MF_GRAYED));

			UINT id = ::TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,pt.x,pt.y,0,this->GetHwnd(),NULL);
			switch(id)
			{
			case IDM_CHEATVALUES_COPY:
			case IDM_CHEATVALUES_CUT:
				{
					std::vector<ACheatEntry::VALUE*> values;
					values.push_back((ACheatEntry::VALUE*)this->GetHwnd());
					int isel;
					ACheatEntry::VALUE* pnewva=0;
					for(isel=-1;(isel=m_CheatsList->GetNextItem(isel,LVNI_SELECTED))!=-1;){
						if(id==IDM_CHEATVALUES_COPY){
							ACheatEntry::VALUE* pvalue = GetScriptIndexof(isel);
							pnewva = new ACheatEntry::VALUE;
							*pnewva = *pvalue;
						}else if(id==IDM_CHEATVALUES_CUT){
							pnewva = GetScriptIndexof(isel);
							list_remove(&m_pEntry->item.values,&pnewva->entry);
							m_CheatsList->DeleteItem(isel);
							isel--;
						}
						values.push_back(pnewva);
					}
					values.push_back(0);

					try{
						if(!OpenClipboard(this->GetHwnd()))
							throw "Open";
						EmptyClipboard();

						size_t size = values.size()*sizeof(values[0]);

						HGLOBAL hglo = ::GlobalAlloc(GMEM_MOVEABLE,size);
						if(!hglo) throw "GlobalAlloc";

						void* pv = ::GlobalLock(hglo);
						if(!pv) throw "lock";

						memcpy(pv,&values[0],size);

						::SetClipboardData(clip.GetID(),pv);

						::GlobalUnlock(hglo);

						CloseClipboard();
					}
					catch(const char* s){
						MessageBox(s,0,MB_ICONERROR);
					}
					return SetDlgResult(TRUE);
				}
			case IDM_CHEATVALUES_PASTE:
				{
					try{
						if(!OpenClipboard(this->GetHwnd()))
							throw "open";
						void* pv = (void*)GetClipboardData(clip.GetID());
						if(!pv) throw "Get";
						ACheatEntry::VALUE** pp = (ACheatEntry::VALUE**)pv;
						HWND hwnd = (HWND)*pp++;
						if(hwnd != this->GetHwnd())
							throw "��Ǹ, ��ǰ��֧�ֿ���̲�������ָ~";
						while(*pp){
							ACheatEntry::VALUE* p = new ACheatEntry::VALUE;
							*p = **pp;
							list_insert_tail(&m_pEntry->item.values,&p->entry);
							AddListItem(p->name.c_str(),0);

							pp++;
						}
						CloseClipboard();
						m_SetModify(true);
					}
					catch(const char* s){
						CloseClipboard();
						MessageBox(s,0,MB_ICONERROR);
					}
					return SetDlgResult(TRUE);
				}
			default:
				break;
			}
			return SetDlgResult(0);
		}
	}
	return 0;
}

INT_PTR ACheatInfoDlg::OnMouseMove(int key,int x,int y)
{
	if(m_bDragging){
		POINT pt = {x,y};
		::ClientToScreen(this->GetHwnd(),&pt);
		::ScreenToClient(m_CheatsList->GetHwnd(),&pt);

		LVHITTESTINFO lvht = {0};
		int iItem=-1;
		lvht.pt = pt;
		ImageList_DragMove(pt.x,pt.y);
		if((iItem = m_CheatsList->HitTest(&lvht))!=-1 && lvht.flags&LVHT_ONITEM){
			//m_CheatsList->SendMessage(LVM_SETHOTITEM,iItem);
			m_CheatsList->SetItemState(iItem,LVIS_SELECTED,LVIS_SELECTED);
		}else{
			m_CheatsList->SetItemState(-1,0,LVIS_SELECTED);

		}
		return SetDlgResult(0);
	}
	return 0;
}
INT_PTR ACheatInfoDlg::OnLButtonUp(int key,int x,int y)
{
	if(m_bDragging){
		ImageList_DragLeave(m_CheatsList->GetHwnd());
		ImageList_EndDrag();
		ReleaseCapture();

		POINT pt={x,y};
		::ClientToScreen(this->GetHwnd(),&pt);
		::ScreenToClient(m_CheatsList->GetHwnd(),&pt);

		LVHITTESTINFO lvht={0};
		int iItem = -1;
		lvht.pt = pt;
		if((iItem=m_CheatsList->HitTest(&lvht))!=-1
			&& lvht.flags&LVHT_ONITEM)
		{
			m_CheatsList->SetItemState(-1,0,LVIS_SELECTED);
			m_CheatsList->SetItemState(iItem,LVIS_SELECTED,LVIS_SELECTED);
			MoveItemToAnotherPlace(m_iDraggingItem,iItem);
			m_SetModify(true);
		}
		m_bDragging = false;
	}
	return 0;
}

INT_PTR ACheatInfoDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == IDC_CHEAT_EXECLUA){
			int isel = m_CheatsList->GetNextItem(-1,LVNI_SELECTED);
			if(isel==-1){
				this->MessageBox("ѡ��һ��ֵ��~");
				return 0;
			}
#define __FCEUX__
#ifndef __FCEUX__
			hFCEUX = FindWindow("FCEUXWindowClass",NULL);
			if(!hFCEUX){
				MessageBox("���ȴ�FCEUX",NULL,MB_ICONEXCLAMATION);
				return 0;
			}
#else
			extern FCEUGI *GameInfo;
			if(!GameInfo){
				MessageBox("���ȴ���Ϸ~","",MB_ICONINFORMATION);
				return 0;
			}
#endif
			ACheatEntry::VALUE* v = GetScriptIndexof(isel);
			std::string& str = v->script;
			int error = luaL_loadbuffer((lua_State*)luastate,str.c_str(),str.size(),"") 
				|| lua_pcall((lua_State*)luastate,0,0,0);

			if(error != 0){
				//fprintf(stderr,"lua_pcall() error:%s\n",lua_tostring(L,-1));
				this->MessageBox(lua_tostring((lua_State*)luastate,-1),"lua�ű�����",MB_ICONERROR);
				lua_pop((lua_State*)luastate,1);
			}
			return 0;
		}else if(ctrlID == IDC_CHEAT_CHEATINFO){
			ACheatCheatInfoDlg dlg(this,m_pEntry);
			if(dlg.GetDlgCode() == IDOK){
				m_editName->SetWindowText(m_pEntry->item.name.c_str());
				
				ControlMessage cm = {0};
				cm.self = this;
				cm.uMsg = WM_SETTEXT;
				cm.wParam = (WPARAM)m_pEntry->item.name.c_str();
				NotifyParent(&cm);
				m_SetModify(true);
				return SetDlgResult(TRUE);
			}
			return 0;
		}else if(ctrlID == IDC_CHEAT_EDITSCRIPT){
			int isel = m_CheatsList->GetNextItem(-1,LVNI_SELECTED);
			if(isel==-1){
				this->MessageBox("ѡ��һ��ֵ��~");
				return 0;
			}
			debug_out(("Edit:isel=%d\n",isel));
			ACheatEntry::VALUE* v = GetScriptIndexof(isel);
			AEditCheatDlg dlg(this,(char*)v->name.c_str(),(char*)v->script.c_str());
			int code = dlg.GetDlgCode();
			if(code == AEditCheatDlg::RET_CLOSE || code==AEditCheatDlg::RET_CANCEL){

			}else if(code == AEditCheatDlg::RET_OK){
				std::string& name = dlg.GetName();
				std::string& script = dlg.GetScript();

				v->name = name;
				v->script = script;

				UpdateListItem(isel,v->name.c_str(),0);
				m_SetModify(true);
				return 0;
			}
			return 0;
		}else if(ctrlID == IDC_CHEAT_ADDLUA){
			AEditCheatDlg dlg(this);
			int code = dlg.GetDlgCode();
			if(code == AEditCheatDlg::RET_CLOSE || code==AEditCheatDlg::RET_CANCEL){

			}else if(code == AEditCheatDlg::RET_OK){
				std::string& name = dlg.GetName();
				std::string& script = dlg.GetScript();
				ACheatEntry::VALUE* pValue = new ACheatEntry::VALUE;
				pValue->name = name;
				pValue->script = script;
				list_insert_tail(&m_pEntry->item.values,&pValue->entry);
				AddListItem(pValue->name.c_str(),0);
				m_SetModify(true);
				return 0;
			}
			return 0;
		}else if(ctrlID == IDC_CHEAT_DELETELUA){
			int count_selected = 0;
			int i;
			for(i=-1;(i=m_CheatsList->GetNextItem(i,LVNI_SELECTED))!=-1;){
				count_selected++;
			}
			if(count_selected == 0){
				MessageBox("ѡ������һ��Ҫɾ����ֵ~","",MB_ICONINFORMATION);
				return SetDlgResult(FALSE);
			}else{
				stringstream ss;
				ss<<"ɾ��ѡ�е� "<<count_selected<<" ��?";
				string str = ss.str();
				if(MessageBox(str.c_str(),"ȷ��",MB_OKCANCEL|MB_ICONQUESTION) == IDCANCEL){
					return SetDlgResult(FALSE);
				}
			}

			for(i=-1;(i=m_CheatsList->GetNextItem(i,LVNI_SELECTED))!=-1;){
				ACheatEntry::VALUE* pValue = GetScriptIndexof(i);
				list_remove(&m_pEntry->item.values,&pValue->entry);
				delete pValue;
				m_CheatsList->DeleteItem(i);
				i--;
			}
			m_SetModify(true);

			return SetDlgResult(TRUE);
		}else if(ctrlID == IDC_CHEAT_CUSTOM){
			lua_State* L = (lua_State*)luastate;
			
			bool need=false;
			for(string::size_type i=0;i<m_pEntry->item.custom.size(); i++){
				if(isalnum(m_pEntry->item.custom[i])){
					need = true;
					break;
				}
			}
			if(!need){
				MessageBox(
					"��ǰ����ָδ�ṩ���Զ���Ľű�~"
					"\n\n"
					"����Ե�� '��Ϣ', �鿴���޸��Զ������ָ�ű�!",
					"����ָ�Զ���",
					MB_ICONINFORMATION);
				return 0;
			}

#ifndef __FCEUX__
			hFCEUX = FindWindow("FCEUXWindowClass",NULL);
			if(!hFCEUX){
				MessageBox("���ȴ�FCEUX",NULL,MB_ICONEXCLAMATION);
				return 0;
			}
#else
			extern FCEUGI *GameInfo;
			if(!GameInfo){
				MessageBox("���ȴ���Ϸ~","",MB_ICONINFORMATION);
				return 0;
			}
#endif
			int ret=luaL_loadbuffer(L,m_pEntry->item.custom.c_str(),m_pEntry->item.custom.size(),"")
				||lua_pcall(L,0,0,0);
			if(ret != 0){
				this->MessageBox(lua_tostring(L,-1),"�ű�����",MB_ICONERROR);
				lua_pop(L,1);
			}
			return SetDlgResult(0);
		}
	}
	return 0;
}

void ACheatInfoDlg::ShowCheatInfo(ACheatEntry* pEntry,void (__cdecl* SetModify)(bool bModified))
{
	m_SetModify = SetModify;
	m_pEntry = pEntry;
	
	//��pEntry=0ʱ...����ɾ����ĳ������ָ��ֵ0
	if(pEntry == NULL){
		m_editName->SetWindowText("<<���һ������ָ>>");
		m_CheatsList->DeleteAllItems();
		this->EnableWindow(FALSE);
		return;
	}else{
		this->EnableWindow(TRUE);
	}

	assert(pEntry->type == ACheatEntry::TYPE_CHEAT);

	m_CheatsList->DeleteAllItems();

	//����Name������
	m_editName->SetWindowText(pEntry->item.name.c_str());
	//��ӽ���ָ�б�ListView
	for(list_s* p=pEntry->item.values.next; p!=&pEntry->item.values; p=p->next){
		ACheatEntry::VALUE* pva = list_data(p,ACheatEntry::VALUE,entry);
		AddListItem(pva->name.c_str(),0);
	}
}

ACheatEntry::VALUE* ACheatInfoDlg::GetScriptIndexof(int index)
{
	int nItems = m_CheatsList->GetItemCount();
	assert(index>=0 && nItems!=-1 && index<nItems);

	int i=index;
	list_s* p = m_pEntry->item.values.next;

	for(; p!=&m_pEntry->item.values && i>0;){
		i--;
		p=p->next;
	}
	assert(p!=&m_pEntry->item.values && "ACheatInfoDlg::GetScriptIndexof(int index)");

	return list_data(p,ACheatEntry::VALUE,entry);
}

void ACheatInfoDlg::InitControls()
{
	//ListView
	RECT rc;
	LVCOLUMN lvc = {0};

	m_CheatsList->GetClientRect(&rc);
	lvc.mask = LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = "����ָ";
	lvc.cx = rc.right-rc.left - GetSystemMetrics(SM_CXHSCROLL)-1;
	m_CheatsList->InsertColumn(0,&lvc);

	m_CheatsList->SetExtendedListViewStyle(m_CheatsList->GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	m_CheatsList->SetStyle(m_CheatsList->GetStyle()&~LVS_SINGLESEL);

}

int ACheatInfoDlg::AddListItem(const char* pszText,LPARAM param)
{
	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT|LVIF_PARAM;
	lvi.pszText = (LPTSTR)pszText;
	lvi.lParam = param;
	lvi.iItem = 0x7FFFFFFF;
	return m_CheatsList->InsertItem(&lvi);
}

int ACheatInfoDlg::UpdateListItem(int index,const char* pszText,LPARAM param)
{
	assert(index>=0 && index<m_CheatsList->GetItemCount());
	
	LVITEM lvi = {0};
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = (LPTSTR)pszText;
	m_CheatsList->SetItem(&lvi);
	return 0;
}

void ACheatInfoDlg::MoveItemToAnotherPlace(int ifrom,int ito)
{
	int count = m_CheatsList->GetItemCount();
	assert(ifrom<count && ito<count);

	if(ifrom == ito) return;

	list_s* pfrom = 0;
	list_s* pto   = 0;
	int i;

	for(i=0,pfrom=m_pEntry->item.values.next;
		pfrom!=&m_pEntry->item.values && i<ifrom;
		i++,pfrom=pfrom->next
	){}
	for(i=0,pto=m_pEntry->item.values.next;
		pto!=&m_pEntry->item.values && i<ito;
		i++,pto=pto->next
	){}
	assert(pfrom != &m_pEntry->item.values);
	assert(pto   != &m_pEntry->item.values);

	list_remove(&m_pEntry->item.values,pfrom); //�Ƴ���ʼ�ڵ㲢���뵽��ֹ�ڵ��

	pfrom->next = pto->next;
	pfrom->prior = pto;

	pto->next->prior = pfrom;
	pto->next = pfrom;

	m_CheatsList->DeleteItem(ifrom);

	ito++;					//���뵽ito�ĺ���,����++
	if(ifrom>ito) {}
	if(ifrom<ito) {ito--;}	//ifrom<ito,ifrom���Ƴ�,λ�þ���ito+1-1

	m_CheatsList->SetItemState(-1,0,LVIS_SELECTED);

	LVITEM lvi={0};
	lvi.mask      = LVIF_TEXT|LVIF_STATE;
	lvi.pszText   = (LPTSTR)list_data(pfrom,ACheatEntry::VALUE,entry)->name.c_str();
	lvi.stateMask = LVIS_SELECTED;
	lvi.state     = LVIS_SELECTED;
	lvi.iItem     = ito;
	m_CheatsList->InsertItem(&lvi);
}
