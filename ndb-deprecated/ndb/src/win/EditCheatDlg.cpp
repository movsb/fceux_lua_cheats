#include "EditCheatDlg.h"
#include "EditBox.h"
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
	m_editDesc->AttachCtrl(this,IDC_EDITCHEAT_DESC);
	m_editScript->AttachCtrl(this,IDC_EDITCHEAT_SCRIPT);

	m_editDesc->SubClass();
	m_editScript->SubClass();

	CenterWindow(GetParent()->GetHwnd());
	ShowWindow(SW_SHOWNORMAL);

	m_editDesc->SetWindowText(m_desc?m_desc:"");
	m_editScript->SetWindowText(m_lua?m_lua:"");

	SetWindowText(m_desc&&m_lua?"编辑金手指":"添加金手指");

	return 0;
}
