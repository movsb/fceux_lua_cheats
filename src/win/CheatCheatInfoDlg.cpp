#include "CheatCheatInfoDlg.h"
#include "../../res/resource.h"
#include "ctl/EditBox.h"
#include <string>


ACheatCheatInfoDlg::~ACheatCheatInfoDlg()
{
	delete m_editName;
	delete m_editEdit;
}

ACheatCheatInfoDlg::ACheatCheatInfoDlg(AWindowBase* parent,ACheatEntry* pEntry)
{
	m_editName   = new AEditBox;
	m_editEdit   = new AEditBox;

	m_pEntry = pEntry;
	m_bCurrentDesc = true;

	SetParent(parent);

	m_ModalDialogResultCode = DialogBox(
		GetModuleHandle(0),
		MAKEINTRESOURCE(IDD_EDITCHEATITEM),
		parent->GetHwnd(),
		(DLGPROC)GetWindowThunk()
	);
}

INT_PTR ACheatCheatInfoDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR ACheatCheatInfoDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;

	m_editName		->AttachCtrl(this,IDC_EDITCHEATITEM_NAME);
	m_editEdit		->AttachCtrl(this,IDC_EDITCHEATITEM_EDIT);

	m_editName		->SubClass();
	m_editEdit		->SubClass();

	m_editName		->SetWindowText(m_pEntry->item.name.c_str());
	m_editEdit		->SetWindowText(m_pEntry->item.desc.c_str());

	m_desc = m_pEntry->item.desc;
	m_custom = m_pEntry->item.custom;

	m_layout.SetLayout(hWnd, MAKEINTRESOURCE(IDR_RCDATA4), GetModuleHandle(NULL));
	m_layout.ResizeLayout();

	CenterWindow(GetParent()->GetHwnd());
	ShowWindow(SW_SHOW);

	::SetFocus(GetDlgItem(hWnd,IDCANCEL));
	return FALSE;
}

INT_PTR ACheatCheatInfoDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == IDC_EDITCHEATITEM_DESC){
			if(m_bCurrentDesc==false){
				m_custom = m_editEdit->GetWindowText();
				m_editEdit->SetWindowText(m_desc.c_str());
				m_bCurrentDesc = true;
			}
			return SetDlgResult(TRUE);
		}else if(ctrlID == IDC_EDITCHEATITEM_CUSTOM){
			if(m_bCurrentDesc==true){
				m_desc = m_editEdit->GetWindowText();
				m_editEdit->SetWindowText(m_custom.c_str());
				m_bCurrentDesc = false;
			}
			return SetDlgResult(TRUE);
		}else if(ctrlID == IDCANCEL){
			EndDialog(IDCANCEL);
			return 0;
		}else if(ctrlID == IDOK){
			if(m_bCurrentDesc) m_desc = m_editEdit->GetWindowText();
			else			   m_custom = m_editEdit->GetWindowText();
			m_pEntry->item.name = m_editName->GetWindowText();
			m_pEntry->item.desc = m_desc;
			m_pEntry->item.custom = m_custom;
			EndDialog(IDOK);
			return SetDlgResult(TRUE);
		}
	}
	return 0;
}

INT_PTR ACheatCheatInfoDlg::OnClose()
{
	EndDialog(IDCANCEL);
	return SetDlgResult(TRUE);
}

INT_PTR ACheatCheatInfoDlg::OnSize( int width,int height )
{
	m_layout.ResizeLayout();
	return 0;
}
