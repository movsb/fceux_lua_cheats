#include "RomInfoDlg.h"
#include "../../res/resource.h"
#include "PictureControl.h"
#include "App.h"
#include "SharedMemory.h"
#include "Archive.h"

ARomInfoDlg::~ARomInfoDlg()
{

}

ARomInfoDlg::ARomInfoDlg(AWindowBase* parent)
{
	//m_map = rom_map;
	this->SetParent(parent);
	CreateDialog(GetModuleHandle(0),MAKEINTRESOURCE(IDD_ROM_INFO),parent->GetHwnd(),(DLGPROC)GetWindowThunk());
}

INT_PTR ARomInfoDlg::DoDefault(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR ARomInfoDlg::OnInitDialog(HWND hWnd,HWND hWndFocus,LPARAM InitParam)
{
	m_hWnd = hWnd;
	//this->ShowWindow(SW_SHOW);

	::CreateWindowEx(0,PICTURE_CONCTROL_CLASS,NULL,WS_VISIBLE|WS_CHILD,25,30,250,240,this->GetHwnd(),HMENU(10000),NULL,0);

	return TRUE;
}

INT_PTR ARomInfoDlg::OnCommand(int codeNotify,int ctrlID,HWND hWndCtrl)
{
	if(hWndCtrl){
		if(ctrlID == ROMINFO_BTN_RUNGAME){
// 			void* data=0;
// 			size_t size=0;
// 			//m_rom->GetRomData(m_rommap->idx,&data,&size);
// 			m_rom->SaveRomAsFile(m_rommap->idx,m_rommap->phdr->comment);
// 			MEM_FILE* mf = GetFirstNesFileFromArchive(m_rommap->phdr->comment);
// 			data = mf->fdata;
// 			size = mf->fsize;
// 
// 			NSM_LOADGAME* plg = (NSM_LOADGAME*)pshared_memory;
// 			plg->hdr.hwndNdb = this->GetHwnd();
// 			plg->hdr.uMsg = NSMM_LOADGAME;
// 			plg->size = size;
// 			strcpy(plg->fname,"shared://");
// 			strcat(plg->fname,m_rommap->phdr->comment);
// 			memcpy(plg->data,data,size);
// 
// 			//delete[] data;
// 
// 			HWND hfc = ::FindWindow("FCEUXWindowClass",0);
// 			if(hfc){
// 				::SendMessage(hfc,WM_APP+100,0,0);
// 			}
// 
// 			return 0;
			try{
				int ret;
				string nes = theApp->GetAppPath() + m_rommap->phdr->comment;
				string fce = "E:\\Program\\FC\\fceux-2.2.2\\output\\fceux.exe";
				m_rom->SaveRomAsFile(m_rommap->idx,nes.c_str());

				HWND hfc = ::FindWindow("FCEUXWindowClass",0);
				if(hfc){
					COPYDATASTRUCT cds = {0};
					cds.dwData = 1;
					cds.cbData = nes.size()+1;
					cds.lpData = (void*)nes.c_str();
					::SendMessage(hfc,WM_COPYDATA,(WPARAM)this->GetHwnd(),(LPARAM)&cds);
					return SetDlgResult(TRUE);
				}else{
					if(nes.find(' ')!=string::npos) nes = "\"" + nes + "\"";
					ret = (int)::ShellExecute(this->GetHwnd(),"open",fce.c_str(),nes.c_str(),0,SW_SHOWNORMAL);
					if(ret > 32)	return SetDlgResult(TRUE);
					else			return SetDlgResult(FALSE);
				}
			}
			catch(const char* e){
				MessageBox(e,0,MB_ICONERROR);
				return SetDlgResult(FALSE);
			}
			catch(string s){
				MessageBox(s.c_str(),0,MB_ICONERROR);
				return SetDlgResult(FALSE);
			}
		}
	}
	return 0;
}
void ARomInfoDlg::ShowRomInfo(ARom* rom,ARomMap* rom_map)
{
	m_rom = rom;
	m_rommap = rom_map;

	//INESHeader ines;
	//ARom::MakeINESHeader(rom_map->phdr,&ines);
	HWND hWnd = this->GetHwnd();

// 	char md5[33]={0};
// 	char crc32[9]={0};
// 	memcpy(md5,rom_map->phdr->md5,32);
// 	memcpy(crc32,rom_map->phdr->crc32,8);

// 	SetDlgItemText(hWnd,ROMINFO_EDIT_NAME,rom_map->phdr->comment);
// 	SetDlgItemText(hWnd,ROMINFO_EDIT_MD5,(LPCSTR)md5);
// 	SetDlgItemText(hWnd,ROMINFO_EDIT_CRC32,(LPCSTR)crc32);
// 	SetDlgItemInt(hWnd,ROMINFO_EDIT_NPROM,ines.n16kbprom,FALSE);
// 	SetDlgItemInt(hWnd,ROMINFO_EDIT_NCHROM,ines.n8kbchrom,FALSE);
// 	SetDlgItemInt(hWnd,ROMINFO_EDIT_MAPPER,ines.mapper,FALSE);

	::SetDlgItemText(hWnd,IDC_ROMINFO_STATICNAME,rom_map->phdr->comment);

	if(rom_map->phdr->splash.type == ARomHeader::SPLASH::RHST_PNG){
		ARomHeader::SPLASH* splash = &rom_map->phdr->splash;
		//string f = theApp->GetAppPath() + "splash.png";
		//MakeSplashFile(splash->data,splash->cb,splash->type);
		PCMemStream* stm = new PCMemStream(splash->data,splash->cb);
		SendDlgItemMessage(hWnd,10000,PCM_SETSTREAM,(WPARAM)stm,0);
		//SendDlgItemMessage(hWnd,10000,PCM_SETIMAGE,(WPARAM)L"splash.png",0);
	}
}
