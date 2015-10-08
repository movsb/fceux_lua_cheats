#include "CheatFileInfoDlg.h"
#include "../../res/resource.h"
#include <CommCtrl.h>
#include "ctl/EditBox.h"
#include <string>


ACheatFileInfoDlg::~ACheatFileInfoDlg()
{
	delete m_editName;
	delete m_editAuthor;
	delete m_editTime;
	delete m_editDesc;
}

ACheatFileInfoDlg::ACheatFileInfoDlg(AWindowBase* parent,ACheatFile* pFile)
{
	m_editName   = new AEditBox;
	m_editAuthor = new AEditBox;
	m_editTime   = new AEditBox;
	m_editDesc   = new AEditBox;

	m_pFile = pFile;
	SetParent(parent);

	m_ModalDialogResultCode = DialogBox(
		GetModuleHandle(0),
		MAKEINTRESOURCE(IDD_CHEATFILE),
		parent->GetHwnd(),
		(DLGPROC)GetWindowThunk()
	);
}

INT_PTR ACheatFileInfoDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR ACheatFileInfoDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;
	m_editName		->AttachCtrl(this,IDC_CHEATFILE_NAME);
	m_editAuthor	->AttachCtrl(this,IDC_CHEATFILE_AUTHOR);
	m_editTime		->AttachCtrl(this,IDC_CHEATFILE_TIME);
	m_editDesc		->AttachCtrl(this,IDC_CHEATFILE_DESC);

	m_editName		->SubClass();
	m_editAuthor	->SubClass();
	m_editTime		->SubClass();
	m_editDesc		->SubClass();

	ACheatFile tmpfile;
	ACheatFile* pFile=0;
	pFile = m_pFile?m_pFile:&tmpfile;

	m_editName		->SetWindowText(pFile->name.c_str());
	m_editAuthor	->SetWindowText(pFile->author.c_str());
	m_editTime		->SetWindowText(pFile->time.c_str());
	m_editDesc		->SetWindowText(pFile->desc.c_str());

	m_layout.SetLayout(hWnd, MAKEINTRESOURCE(IDR_RCDATA5),GetModuleHandle(NULL));
	m_layout.ResizeLayout();

	CenterWindow(GetParent()->GetHwnd());
	ShowWindow(SW_SHOW);

	::SetFocus(GetDlgItem(hWnd,IDC_CHEATFILE_BTNOK));
	return FALSE;
}

INT_PTR ACheatFileInfoDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == IDC_CHEATFILE_BTNOK){
			ACheatFile* file = 0;
			file = m_pFile?m_pFile:new ACheatFile;

			file->name   = m_editName->GetWindowText();
			file->author = m_editAuthor->GetWindowText();
			file->time   = m_editTime->GetWindowText();
			file->desc   = m_editDesc->GetWindowText();
			

			if(m_pFile==0) m_pFile = file;
			EndDialog(RET_OK);
			return SetDlgResult(TRUE);
		}else if(ctrlID == IDC_CHEATFILE_BTNCANCEL){
			EndDialog(RET_CANCEL);
			return SetDlgResult(TRUE);
		}
	}
	return 0;
}

INT_PTR ACheatFileInfoDlg::OnClose()
{
	EndDialog(RET_CLOSE);
	return SetDlgResult(TRUE);
}

INT_PTR ACheatFileInfoDlg::OnSize( int width,int height )
{
	m_layout.ResizeLayout();
	return 0;
}
