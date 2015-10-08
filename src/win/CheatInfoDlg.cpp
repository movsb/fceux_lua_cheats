#include "CheatInfoDlg.h"
#include "../../res/resource.h"
#include "../tinyxml/tinyxml.h"
#include <CommCtrl.h>
#include "ctl/EditBox.h"
#include "ctl/ListView.h"
#include "ctl/TreeView.h"
#include <string>
#include <vector>
#include <sstream>
#include "CheatCustomDlg.h"
#include "CheatCheatInfoDlg.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#define __OUT__
#include "E:\\Program\\FC\\fceux-2.2.2\\src\\drivers\\win\\CheatInterface.h"

#include "EditCheatDlg.h"

#include "ctl/Clipboard.h"
#include "ctl/App.h"



extern HWND MainhWnd;
HWND hThisDlg;
AWindowBase* ThisDlg;

/**************************************************
函  数:boolean pause([boolean new_state])
功  能:设置模拟器运行状态
参  数:new_state 可选:
			true:暂停
			false:运行
返  回:返回调用前的模拟器状态
说  明:
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
函  数:lua_Integer read(lua_Integer addr [,lua_Integer count])
功  能:读NES内存,范围为0x0000~0xFFFF
参  数:	addr - 读取内存的起始地址
		count 可选 - 字节数,不能多于16个
返  回:	若未指定字节数,返回addr地址的一个字节
		否则返回count指定的从addr开始的连续字节数个地址值
说  明:
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
函  数:write(lua_Integer addr,lua_Integer value [,value1 [,value2 [,...]]])
功  能:写nes内存
参  数:	addr - 写入内存的起始地址
		value, value1, value2, ... - 待写入的内存的字节值,不得多于16个
返  回:
说  明:
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
函  数:lock(lua_Integer addr,value1, [value2 [,...]])
功  能:锁定内存值为给定值
参  数:	addr - 开始锁定的内存地址
		valuex - 连续的字节值,不得多于16个
返  回:
说  明:修改:待锁定的内存在锁定前会被解锁
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
函  数:unlock(lua_Integer addr [value1 [,value2 [,...]]])
功  能:解锁内存,写入内存
参  数:	addr - 开始解锁的内存地址
		valuex 可选- 若指定,则解锁并写入内存字节值, 不超过16个
返  回:
说  明:
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
函  数:lua_Integer msgbox([见下])
功  能:弹出消息对话框,显示信息
参  数:
		参数个数及类型:
			1.msgbox(string text)
				仅显示文本,无返回值
			2.msgbox(string text,string title)
				显示文本和标题,无返回值
			3.msgbox(string text,integer type)
				显示文本及指定类型的按钮,并返回按下按钮的类型
				预定义的按钮类型及值见说明
			4.msgbox(string text,string title,integer type)
				显示文本,标题及指定类型的按钮,并返回按下按钮的类型
				预定义的按钮类型及值见说明

返  回:见参数部分
说  明:	预定义的按钮类型:
			0 - 确定[Ok]
			1 - 确定取消[OkCancel]
			2 - 是否[YesNo]
		按钮返回值:
			0 - 确定[Ok]
			1 - 取消[Cancel]
			0 - 是[Yes]
			1 - 否[No]
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
函  数:boolean,integer dialog(table dlg)
功  能:显示一个输入对话框
参  数:	table,key-value,见下
			text - 提示内容
			onok - 按下[确定时要做的动作lua函数
				函数C语言原型 int onok(unsigned char val)
				若返回1:对话框关闭,dialog函数返回true,val
				若返回0:对话框不关闭
返  回:见参数
说  明:
**************************************************/
extern "C" static int fceu_dialog(lua_State* L)
{
	int args = lua_gettop(L);
	if(args < 1) return 0;

	if(!lua_istable(L,1)){
		MessageBox(hThisDlg,"dialog参数错误!",0,0);
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

	m_layout.SetLayout(hWnd, MAKEINTRESOURCE(IDR_RCDATA2),GetModuleHandle(NULL));
	m_layout.ResizeLayout();

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
							ACheatEntry::VALUE* pvalue = m_pEntry->item.values[isel];
							pnewva = new ACheatEntry::VALUE;
							*pnewva = *pvalue;
						}else if(id==IDM_CHEATVALUES_CUT){
							auto& vs = m_pEntry->item.values;
							pnewva = vs[isel];
							auto s = vs.begin();
							for(; *s!=pnewva; ++s)
								;
							vs.erase(s);
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
					if(id==IDM_CHEATVALUES_CUT)
						m_pFile->bNeedSaving=true;
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
							throw "抱歉, 当前不支持跨进程操作金手指~";
						while(*pp){
							ACheatEntry::VALUE* p = new ACheatEntry::VALUE;
							*p = **pp;
							m_pEntry->item.values.push_back(p);
							AddListItem(p->name.c_str(),0);

							pp++;
						}
						CloseClipboard();
						m_pFile->bNeedSaving = true;
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
			if(m_iDraggingItem != iItem)
				m_pFile->bNeedSaving = true;
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
				this->MessageBox("选中一个值先~");
				return 0;
			}

			extern FCEUGI *GameInfo;
			if(!GameInfo){
				MessageBox("请先打开游戏~","",MB_ICONINFORMATION);
				return 0;
			}

			std::string& str = m_pEntry->item.values[isel]->script;
			int error = luaL_loadbuffer((lua_State*)luastate,str.c_str(),str.size(),"") 
				|| lua_pcall((lua_State*)luastate,0,0,0);

			if(error != 0){
				//fprintf(stderr,"lua_pcall() error:%s\n",lua_tostring(L,-1));
				this->MessageBox(lua_tostring((lua_State*)luastate,-1),"lua脚本错误",MB_ICONERROR);
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
				m_pFile->bNeedSaving = true;
				return SetDlgResult(TRUE);
			}
			return 0;
		}else if(ctrlID == IDC_CHEAT_EDITSCRIPT){
			int isel = m_CheatsList->GetNextItem(-1,LVNI_SELECTED);
			if(isel==-1){
				this->MessageBox("选中一个值先~");
				return 0;
			}
			ACheatEntry::VALUE* v = m_pEntry->item.values[isel];
			AEditCheatDlg dlg(this,(char*)v->name.c_str(),(char*)v->script.c_str());
			int code = dlg.GetDlgCode();
			if(code == AEditCheatDlg::RET_CLOSE || code==AEditCheatDlg::RET_CANCEL){

			}else if(code == AEditCheatDlg::RET_OK){
				std::string& name = dlg.GetName();
				std::string& script = dlg.GetScript();

				v->name = name;
				v->script = script;

				UpdateListItem(isel,v->name.c_str(),0);
				m_pFile->bNeedSaving = true;
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
				m_pEntry->item.values.push_back(pValue);
				AddListItem(pValue->name.c_str(),0);
				m_pFile->bNeedSaving = true;
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
				MessageBox("选择至少一个要删除的值~","",MB_ICONINFORMATION);
				return SetDlgResult(FALSE);
			}else{
				stringstream ss;
				ss<<"删除选中的 "<<count_selected<<" 项?";
				string str = ss.str();
				if(MessageBox(str.c_str(),"确认",MB_OKCANCEL|MB_ICONQUESTION) == IDCANCEL){
					return SetDlgResult(FALSE);
				}
			}

			for(i=-1;(i=m_CheatsList->GetNextItem(i,LVNI_SELECTED))!=-1;){
				m_CheatsList->DeleteItem(i);
				auto& vs = m_pEntry->item.values;
				auto v = vs[i];
				delete v;

				auto s=vs.begin();
				for(; *s != v; ++s)
					;
				vs.erase(s);
				i--;
			}
			m_pFile->bNeedSaving = true;

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
					"当前金手指未提供可自定义的脚本~"
					"\n\n"
					"你可以点击 '信息', 查看并修改自定义金手指脚本!",
					"金手指自定义",
					MB_ICONINFORMATION);
				return 0;
			}

			extern FCEUGI *GameInfo;
			if(!GameInfo){
				MessageBox("请先打开游戏~","",MB_ICONINFORMATION);
				return 0;
			}

			int ret=luaL_loadbuffer(L,m_pEntry->item.custom.c_str(),m_pEntry->item.custom.size(),"")
				||lua_pcall(L,0,0,0);
			if(ret != 0){
				this->MessageBox(lua_tostring(L,-1),"脚本错误",MB_ICONERROR);
				lua_pop(L,1);
			}
			return SetDlgResult(0);
		}
	}
	return 0;
}


INT_PTR ACheatInfoDlg::OnSize( int width,int height )
{
	m_layout.ResizeLayout();
	UpdateListHeaderSize();	
	return 0;
}

void ACheatInfoDlg::ShowCheatInfo(ACheatFile* file,ACheatEntry* pEntry)
{

	m_pFile = file;
	m_pEntry = pEntry;
	
	//当pEntry=0时...比如删除了某个金手指后置0
	if(pEntry == NULL){
		m_editName->SetWindowText("<<请打开一个金手指>>");
		m_CheatsList->DeleteAllItems();
		this->EnableWindow(FALSE);
		return;
	}else{
		this->EnableWindow(TRUE);
	}

	assert(pEntry->type == ACheatEntry::TYPE_CHEAT);
	if(pEntry->type != ACheatEntry::TYPE_CHEAT){
		MessageBox("app error",0,MB_ICONERROR);
		return;
	}
	m_CheatsList->DeleteAllItems();

	//更新Name和描述
	m_editName->SetWindowText(pEntry->item.name.c_str());
	
	//lists items all to lv
	for(auto i : m_pEntry->item.values){
		AddListItem(i->name.c_str(),0);
	}
}

void ACheatInfoDlg::InitControls()
{
	//ListView
	RECT rc;
	LVCOLUMN lvc = {0};

	m_CheatsList->GetClientRect(&rc);
	lvc.mask = LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = "金手指";
	lvc.cx = rc.right-rc.left - GetSystemMetrics(SM_CXHSCROLL)-1;
	m_CheatsList->InsertColumn(0,&lvc);

	m_CheatsList->SetExtendedListViewStyle(m_CheatsList->GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	m_CheatsList->SetStyle(m_CheatsList->GetStyle()&~LVS_SINGLESEL);
}

void ACheatInfoDlg::UpdateListHeaderSize()
{
	RECT rc;
	m_CheatsList->GetClientRect(&rc);
	int width = ListView_GetColumnWidth(m_CheatsList->GetHwnd(),0);
	ListView_SetColumnWidth(m_CheatsList->GetHwnd(),0, rc.right- GetSystemMetrics(SM_CXHSCROLL)-1);
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

int ACheatInfoDlg::UpdateListItem(int index,const char* pszText,LPARAM/* param*/)
// updates list item index with specified text pszText and LPARAM param
// param - not used
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
// reorders the values lv. moves ifrom behind ito. starts from 0.
{
	int count = m_CheatsList->GetItemCount();
	assert(ifrom<count && ito<count);
	if(ifrom == ito) return;
	if(!count) return;
	if(count != m_pEntry->item.values.size()) return;

	//1st: reorders the lv.
	ito++;					//after ito
	if(ifrom>ito) {}		// no item was deleted before ito
	if(ifrom<ito) {ito--;}	// LT, ifrom was deleted, ito-1

	if(ifrom == ito) return;

	m_CheatsList->DeleteItem(ifrom);
	m_CheatsList->SetItemState(-1,0,LVIS_SELECTED);	//deselect all

	LVITEM lvi={0};
	lvi.mask      = LVIF_TEXT|LVIF_STATE;
	lvi.pszText   = (LPTSTR)m_pEntry->item.values[ifrom]->name.c_str();
	lvi.stateMask = LVIS_SELECTED;
	lvi.state     = LVIS_SELECTED;
	lvi.iItem     = ito;
	m_CheatsList->InsertItem(&lvi);

	//2nd: reorders the vector
	auto& v = m_pEntry->item.values;
	auto t = v[ifrom];
	v[ifrom] =v[ito];
	v[ito] = t;
}
