

#undef NOMINMAX
#include "PictureControl.h"
#include <windows.h>

#include <GdiPlus.h>

using namespace Gdiplus;

struct PICTURE_CONTROL_STRUCT{
	wchar_t file[MAX_PATH];
	PCMemStream* stream;
	Image* image;


	PICTURE_CONTROL_STRUCT()
	{
		*file=0;
		stream=0;
		image=0;
	}
};

class _PictureControlDummy{
public:
	_PictureControlDummy()
	{
		PictureControl_RegisterClass();
	}
};

_PictureControlDummy _picctrldummy; 

LRESULT CALLBACK PictureControl_WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PICTURE_CONTROL_STRUCT* ppcs = (PICTURE_CONTROL_STRUCT*)GetWindowLong(hWnd,0);
	switch(uMsg)
	{
	case WM_PAINT:
		{
			//InvalidateRect(hWnd,NULL,TRUE);

			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;

			hdc = BeginPaint(hWnd,&ps);
			
			
			//Image image(ppcs->stream);

			Image& image = *ppcs->image;

			if(image.GetLastStatus() == Status::Ok){
				SetWindowPos(hWnd,0,0,0,image.GetWidth(),image.GetHeight(),SWP_NOMOVE|SWP_NOZORDER);
				Graphics g(hdc);
				GetClientRect(hWnd,&rc);
				g.DrawImage(&image,0,0,rc.right-rc.left,rc.bottom-rc.top);
				//g.DrawImage()
			}

			EndPaint(hWnd,&ps);

			return 0;
		}
	case WM_NCCREATE:
		{
			PICTURE_CONTROL_STRUCT* p = new PICTURE_CONTROL_STRUCT;
			//memset(p,0,sizeof(*p));
			SetWindowLong(hWnd,0,(LONG)p);
			return TRUE;
		}
	case WM_NCDESTROY:
		{
			delete ppcs;
			return 0;
		}

	case WM_LBUTTONDOWN:
		{
			//InvalidateRect(hWnd,NULL,TRUE);
			SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,0);
			return 0;
		}

	//////////////////////////////////////////////////////////////////////////
	case PCM_SETSTREAM:
		{
			PCMemStream* stream = (PCMemStream*)wParam;
			if(ppcs->image){
				delete ppcs->image;
			}
			ppcs->image = new Image(stream);
			//delete stream;
			InvalidateRect(hWnd,NULL,TRUE);
			return 0;
		}
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void PictureControl_RegisterClass(void)
{
	WNDCLASSEX wce = {0};
	wce.cbSize = sizeof(wce);
	wce.cbWndExtra = sizeof(PICTURE_CONTROL_STRUCT*);
	wce.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);//(HBRUSH)GetStockObject(WHITE_BRUSH);
	wce.hInstance = GetModuleHandle(NULL);
	wce.lpfnWndProc = PictureControl_WndProc;
	wce.lpszClassName = PICTURE_CONCTROL_CLASS;
	wce.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	RegisterClassEx(&wce);
}

