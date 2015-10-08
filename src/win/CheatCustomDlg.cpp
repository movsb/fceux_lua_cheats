#include "CheatCustomDlg.h"
#include "../../res/resource.h"
#include "ctl/EditBox.h"
#include "../debug.h"
#include <string>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

ACheatCustomDlg::~ACheatCustomDlg()
{
	delete m_editCustom;
}

ACheatCustomDlg::ACheatCustomDlg(AWindowBase* parent,HWND* phModal,lua_State* L)
{
	HWND hOld=0;
	m_editCustom = new AEditBox;
	this->SetParent(parent);
	m_Lua = L;
	m_phModal = phModal;
	hOld = *phModal;
	m_ModalDialogResultCode=DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_CUSTOMCHEAT),parent->GetHwnd(),(DLGPROC)GetWindowThunk());
	*phModal = hOld;
}

void ACheatCustomDlg::InitializeFromTable()
{
	const char* text = 0;
	lua_getfield(m_Lua,-1,"title");
	text = lua_tostring(m_Lua,-1);
	lua_pop(m_Lua,1);
	if(!text) text = "";

	::SetWindowText(m_Static,text);

	lua_getfield(m_Lua,-1,"min");
	if(lua_isnumber(m_Lua,-1)){
		m_minValue = (int)luaL_checkinteger(m_Lua,-1);
	}else{
		m_minValue = -1;
	}
	lua_pop(m_Lua,1);

	lua_getfield(m_Lua,-1,"max");
	if(lua_isnumber(m_Lua,-1)){
		m_maxValue = (int)luaL_checkinteger(m_Lua,-1);
	}else{
		m_maxValue = -1;
	}
	lua_pop(m_Lua,1);
}

bool ACheatCustomDlg::CheckValid(int n)
{
	return false;
}

bool ACheatCustomDlg::LuaOnOk(int val)
{
	int error;
	lua_getfield(m_Lua,-1,"onok");
	if(lua_type(m_Lua,-1)==LUA_TNIL){
		lua_pop(m_Lua,1);
		return true;
	}

	lua_pushinteger(m_Lua,val);

	error = lua_pcall(m_Lua,1,1,0);

	if(error != /*LUA_OK*/0){
		MessageBox(lua_tostring(m_Lua,-1),"脚本错误!",MB_ICONERROR);
		lua_pop(m_Lua,1);
		return false;
	}

	int ret = lua_tointeger(m_Lua,-1);
	lua_pop(m_Lua,1);
	return ret!=0;
}

INT_PTR ACheatCustomDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR ACheatCustomDlg::OnClose()
{
	EndDialog(0);
	return 0;
}

INT_PTR ACheatCustomDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;

	m_editCustom->AttachCtrl(this,IDC_CUSTOMCHEAT_EDIT);
	m_Static = GetDlgItem(hWnd,IDC_CUSTOMCHEAT_STATIC);

	m_editCustom->SubClass();

	CenterWindow(GetParent()->GetHwnd());

	InitializeFromTable();

	*m_phModal = hWnd;

	m_layout.SetLayout(hWnd, MAKEINTRESOURCE(IDR_RCDATA6), GetModuleHandle(NULL));
	m_layout.ResizeLayout();

	ShowWindow(SW_SHOWNORMAL);
	m_editCustom->SetFocus();
	return FALSE;
}

INT_PTR ACheatCustomDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == IDOK){
			string s = m_editCustom->GetWindowText();
			if(s.size()==0){
				m_editCustom->SetFocus();
				return SetDlgResult(FALSE);
			}

			m_Value = atoi(s.c_str());

			if(m_minValue!=-1 && m_Value<m_minValue){
				MessageBox("输入的值过小~");
				m_editCustom->SetFocus();
				return SetDlgResult(FALSE);
			}
			if(m_maxValue!=-1 && m_Value>m_maxValue){
				MessageBox("输入的值过大~");
				m_editCustom->SetFocus();
				return SetDlgResult(FALSE);
			}
			if(LuaOnOk(m_Value)){
				EndDialog(1);
				return SetDlgResult(0);
			}else{
				m_editCustom->SetFocus();
			}
			return SetDlgResult(0);
		}else if(ctrlID == IDCANCEL){
			EndDialog(0);
			return SetDlgResult(0);
		}
	}
	return 0;
}

INT_PTR ACheatCustomDlg::OnSize( int width,int height )
{
	m_layout.ResizeLayout();
	return 0;
}
