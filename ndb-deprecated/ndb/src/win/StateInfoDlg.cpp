#include "StateInfoDlg.h"
#include "../../res/resource.h"
#include "App.h"

AStateInfoDlg::AStateInfoDlg(AWindowBase* parent)
{
	SetParent(parent);
	CreateDialog(theApp->GetAppInstance(),MAKEINTRESOURCE(IDD_STATE),parent->GetHwnd(),(DLGPROC)GetWindowThunk());
}

AStateInfoDlg::~AStateInfoDlg()
{

}

INT_PTR AStateInfoDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR AStateInfoDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;

	::SetFocus(NULL);
	return FALSE;
}

INT_PTR AStateInfoDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == IDC_STATEINFO_DELETE){
			if(codeNotify==BN_CLICKED){
				int isel = ::SendDlgItemMessage(this->GetHwnd(),IDC_STATEINFO_STATELIST,LB_GETCURSEL,0,0);
				if(isel != LB_ERR){
					AStateMap* state = GetScriptIndexof(isel);
					list_remove(m_ls,&state->entry);
					::SendDlgItemMessage(this->GetHwnd(),IDC_STATEINFO_STATELIST,LB_DELETESTRING,isel,0);
					m_state->DelState(state->idx);
					delete state;
					return SetDlgResult(TRUE);
				}
			}
		}
	}
	return 0;
}

AStateMap* AStateInfoDlg::GetScriptIndexof(int index)
{
	int nItems = ::SendDlgItemMessage(this->GetHwnd(),IDC_STATEINFO_STATELIST,LB_GETCOUNT,0,0);
	assert(index>=0 && nItems!=-1 && index<nItems);

	int i=index;
	list_s* p = m_ls->next;

	for(; p!=m_ls && i>0;){
		i--;
		p=p->next;
	}
	assert(p!=m_ls && "AStateInfoDlg::GetScriptIndexof(int index)");

	return list_data(p,AStateMap,entry);;
}

void AStateInfoDlg::ShowStateInfo(AState* state,list_s* ls)
{
	m_state = state;
	m_ls = ls;

	::SendDlgItemMessage(this->GetHwnd(),IDC_STATEINFO_STATELIST,LB_RESETCONTENT,0,0);

	for(list_s* p = ls->next; p!=ls; p=p->next){
		AStateMap* smap = list_data(p,AStateMap,entry);
		::SendDlgItemMessage(this->GetHwnd(),IDC_STATEINFO_STATELIST,LB_INSERTSTRING,-1,(LPARAM)smap->phdr->name.c_str());
	}
}
