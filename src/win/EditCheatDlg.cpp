#include "EditCheatDlg.h"
#include "ctl/EditBox.h"
#include "../../res/resource.h"

AEditCheatDlg::AEditCheatDlg(AWindowBase* parent,char* desc,char* lua)
{
	m_editDesc = new AEditBox;
	m_editScript = new AEditBox;
	SetParent(parent);
	m_desc = desc;
	m_lua = lua;
	m_ModalDialogResultCode = DialogBox(GetModuleHandle(0),MAKEINTRESOURCE(IDD_EDITCHEAT),parent->GetHwnd(),(DLGPROC)GetWindowThunk());
}

AEditCheatDlg::~AEditCheatDlg()
{
	DeleteObject(m_hFont);
	delete m_editDesc;
	delete m_editScript;
}

INT_PTR AEditCheatDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AEditCheatDlg::OnClose()
{
	EndDialog(RET_CLOSE);
	return 1;
}

INT_PTR AEditCheatDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == IDC_EDITCHEAT_OK){
			m_Name = m_editDesc->GetWindowText();
			m_Script = m_editScript->GetWindowText();
			EndDialog(RET_OK);
			return 0;
		}else if(ctrlID == IDC_EDITCHEAT_CANCEL){
			EndDialog(RET_CANCEL);
			return 0;
		}
	}
	return 0;
}

INT_PTR AEditCheatDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;

	LOGFONT lf={0};
	GetObject(GetStockObject(DEFAULT_GUI_FONT),sizeof(lf),&lf);
	lf.lfHeight = -16;
	strcpy(lf.lfFaceName,"微软雅黑");
	m_hFont=CreateFontIndirect(&lf);

	m_editDesc->AttachCtrl(this,IDC_EDITCHEAT_DESC);
	m_editScript->AttachCtrl(this,IDC_EDITCHEAT_SCRIPT);

	m_editDesc->SubClass();
	m_editScript->SubClass();

	m_editScript->SendMessage(WM_SETFONT,WPARAM(m_hFont),MAKELPARAM(TRUE,0));

	CenterWindow(GetParent()->GetHwnd());
	ShowWindow(SW_SHOWNORMAL);

	m_editDesc->SetWindowText(m_desc?m_desc:"");
	m_editScript->SetWindowText(m_lua?m_lua:"");

	SetWindowText(m_desc&&m_lua?"编辑金手指":"添加金手指");

	// Layout info
	m_layout.Add(IDC_EDITCHEAT_DESC,hWnd,LayoutItem::Align(LayoutItem::kLEFT|LayoutItem::kTOP|LayoutItem::kRisizeHorz));
	m_layout.Add(IDC_EDITCHEAT_SCRIPT,hWnd,LayoutItem::Align(LayoutItem::kLEFT|LayoutItem::kTOP|LayoutItem::kRisizeHorz|LayoutItem::kRisizeVert));
	m_layout.Add(IDC_EDITCHEAT_OK,hWnd,LayoutItem::Align(LayoutItem::kRIGHT|LayoutItem::kBOTTOM));
	m_layout.Add(IDC_EDITCHEAT_CANCEL,hWnd,LayoutItem::Align(LayoutItem::kRIGHT|LayoutItem::kBOTTOM));

	return 0;
}

INT_PTR AEditCheatDlg::OnGetDlgCode(WPARAM vk,MSG* msg)
{
	if(!msg) return 0;
	if(msg->hwnd == m_editScript->GetHwnd()){
		return SetDlgResult(DLGC_WANTTAB);
	}
	return 0;
}
