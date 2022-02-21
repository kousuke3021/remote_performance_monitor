/*!
CustomButton.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include"CustomButton.h"
#include<commctrl.h>
#include <gdiplus.h>

#define WM_SUBCLASS (WM_APP+1)

std::map< HWND, GraphButton*> GraphButton::gbuttonhash;
GraphButton* GraphButton::selected;

using namespace std;
using namespace Gdiplus;

GraphButton* GraphButton::getObjPtr(HWND hWnd){
	map< HWND, GraphButton* >::iterator it = GraphButton::gbuttonhash.find(hWnd);
	if(it != GraphButton::gbuttonhash.end()){
		return it->second;
	}
	return NULL;
};

void GraphButton::regist(GraphButton* pGinfo){
	// ボタンのウィンドウハンドル取得
	HWND hWnd = pGinfo->hWnd;
	// バイナリツリーに登録
	GraphButton::gbuttonhash.insert(make_pair(hWnd, pGinfo));
};

void GraphButton::SetColor(int R,int G,int B){
	show.SetColor(R, G, B);
}

void GraphButton::ShowFormat(const char* format_){
	format = string(format_);
}

void GraphButton::SetTitle(const wchar_t* name_){
	title = name_;
}

HWND GraphButton::Create(int x, int y, int width, int height,int id, HWND hParent_, HINSTANCE hInst_, float info){
	hParent = hParent_;
	hInst = hInst_;

	show.SetColor(255, 100, 255);
	show.SetMinMax(0, 100);
	show.grid_flag = false;
	show.only_graph = true;

	if(hWnd != NULL){
		return NULL;
	}
	hWnd = CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | 0 | WS_CLIPCHILDREN, x, y, width, height, hParent, (HMENU)id, hInst, this);
	
	if(hWnd != NULL){
		DefaultButtonProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
		regist(this);
		SetWindowSubclass(hWnd, GlobalWindowProc, 1, 0);
		SendMessage(hWnd, WM_SUBCLASS, 0, 0);
	}
	// 自前のカスタムプロシージャを設定
	
	return hWnd;
}

void GraphButton::Update(float val){
	show.update(current_val = val);
	InvalidateRect(hWnd, NULL, FALSE);
}

HWND GraphButton::GethWnd(){
	return hWnd;
}

LRESULT CALLBACK GraphButton::GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData){
	static HPEN hPen;
	static HFONT hFont_title,hFont_val;
	static HBRUSH hBrush_over,hBrush_push;
	RECT rc,grc;
	HDC hdc;
	PAINTSTRUCT ps;
	HFONT oldFont;
	HPEN oldPen;
	HBRUSH oldBrush;
	POINT point;
	wchar_t wtext[64];
	char text[64];
	
	GraphButton* gbutton = getObjPtr(hWnd);

	switch(msg){
		case WM_SUBCLASS:
			GetClientRect(hWnd, &rc);

			gbutton->show.Create(4, 8, 60, rc.bottom-16, hWnd, gbutton->hInst);
			ShowWindow(gbutton->show.GethWnd(), SW_HIDE);

			hdc = GetDC(hWnd);
			gbutton->hBitmap1 = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			gbutton->hdcMem1 = CreateCompatibleDC(NULL);		// カレントスクリーン互換
			SelectObject(gbutton->hdcMem1, gbutton->hBitmap1);		// MDCにビットマップを割り付け
			gbutton->hBitmap2 = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			gbutton->hdcMem2 = CreateCompatibleDC(NULL);		// カレントスクリーン互換
			SelectObject(gbutton->hdcMem2, gbutton->hBitmap2);		// MDCにビットマップを割り付け
			ReleaseDC(hWnd, hdc);

			if(hPen == NULL){
				hPen = CreatePen(PS_SOLID, 1, RGB(205, 232, 255));
			}
			if(hFont_title == NULL){
				hFont_title = CreateFont(20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
			}
			if(hFont_val == NULL){
				hFont_val = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
			}
			if(hBrush_over == NULL){
				hBrush_over = CreateSolidBrush(RGB(229, 243, 255));
			}
			if(hBrush_push == NULL){
				hBrush_push = CreateSolidBrush(RGB(205, 232, 255));
			}
			break;


		case WM_ERASEBKGND:
			return 1;

		case WM_SIZE:
			DeleteObject(gbutton->hBitmap1);
			DeleteObject(gbutton->hdcMem1);
			DeleteObject(gbutton->hBitmap2);
			DeleteObject(gbutton->hdcMem2);
			GetClientRect(hWnd, &rc);
			hdc = GetDC(hWnd);
			gbutton->hBitmap1 = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			gbutton->hdcMem1 = CreateCompatibleDC(NULL);		// カレントスクリーン互換
			SelectObject(gbutton->hdcMem1, gbutton->hBitmap1);		// MDCにビットマップを割り付け
			gbutton->hBitmap2 = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			gbutton->hdcMem2 = CreateCompatibleDC(NULL);		// カレントスクリーン互換
			SelectObject(gbutton->hdcMem2, gbutton->hBitmap2);		// MDCにビットマップを割り付け
			ReleaseDC(hWnd, hdc);

			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MOUSEMOVE:
			{
				gbutton->mouse_over = true;
				TRACKMOUSEEVENT tme = {sizeof(tme)};
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			SetCursor(LoadCursor(NULL, IDC_HAND));
			break;

		case WM_LBUTTONDOWN:
			gbutton->mouse_l_push = true;
			break;

		case WM_LBUTTONUP:
			gbutton->mouse_l_push = false;
			break;

		case WM_SETFOCUS:
			gbutton->focus = true;
			PostMessage(hWnd, WM_NEXTDLGCTL, 0, 0);
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_KILLFOCUS:
			gbutton->focus = false;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MOUSELEAVE:
			gbutton->mouse_over = false;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_BTN_SELECTED:
			if(selected != NULL)
				InvalidateRect(selected->hWnd, NULL, FALSE);
			selected = gbutton;
			break;

		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rc);
				SetBkMode(gbutton->hdcMem1, TRANSPARENT);

				Graphics offScreen(gbutton->hdcMem1);
				Color bkcolor;
				

				GetCursorPos(&point);
				ScreenToClient(hWnd, &point);
				if(selected == gbutton || gbutton->mouse_l_push){//灰色
					bkcolor = Color(200, 200, 200, 200);
				} else{
					bkcolor = Color(255, 255, 255);
				}
				SolidBrush  BkBrush(bkcolor);
				offScreen.FillRectangle(&BkBrush, 0, 0, rc.right, rc.bottom);
				if((selected != gbutton && (0 < point.x && point.x < rc.right && 0 < point.y && point.y < rc.bottom)) || gbutton->focus){
					SolidBrush alpha_brush(Color(200, 180, 220, 255));
					offScreen.FillRectangle(&alpha_brush, 0, 0, rc.right, rc.bottom);
				}
				if(selected != gbutton && (0 < point.x && point.x < rc.right && 0 < point.y && point.y < rc.bottom)){
					Pen oPen_border(Color(250, 180, 220, 255));
					offScreen.DrawRectangle(&oPen_border, 0, 0, rc.right-1, rc.bottom-1);
				}
				/*if((0 < point.x && point.x < rc.right && 0 < point.y && point.y < rc.bottom) || !gbutton->selected){
					oldBrush = (HBRUSH)SelectObject(gbutton->hdcMem1, hBrush_over);
				} else{
					oldBrush = (HBRUSH)SelectObject(gbutton->hdcMem1, GetStockObject(WHITE_BRUSH));
				}
				if(gbutton->mouse_l_push || gbutton->focus){
					oldPen = (HPEN)SelectObject(gbutton->hdcMem1, hPen);
					SelectObject(gbutton->hdcMem1, hBrush_push);
				} else{
					oldPen = (HPEN)SelectObject(gbutton->hdcMem1, GetStockObject(WHITE_PEN));
				}

				Rectangle(gbutton->hdcMem1, 0, 0, rc.right, rc.bottom);
				SelectObject(gbutton->hdcMem1, oldPen);
				SelectObject(gbutton->hdcMem1, oldBrush);*/

				oldFont = (HFONT)SelectObject(gbutton->hdcMem1, hFont_title);
				TextOut(gbutton->hdcMem1, 70, 5, gbutton->title.c_str(), lstrlen(gbutton->title.c_str()));
				SelectObject(gbutton->hdcMem1, oldFont);

				oldFont = (HFONT)SelectObject(gbutton->hdcMem1, hFont_val);
				snprintf(text, 64, gbutton->format.c_str(), gbutton->current_val);
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				TextOut(gbutton->hdcMem1, 75, 30, wtext, lstrlen(wtext));
				SelectObject(gbutton->hdcMem1, oldFont);

				BitBlt(gbutton->hdcMem2, 0, 0, rc.right, rc.bottom, gbutton->hdcMem1, 0, 0, SRCCOPY);
				GetClientRect(gbutton->show.GethWnd(), &grc);
				StretchBlt(gbutton->hdcMem2, 5, 5, 60, rc.bottom - 10, gbutton->show.Paint(gbutton->show.GethWnd(), &gbutton->show), 0, 0, grc.right, grc.bottom, SRCCOPY);
				BitBlt(hdc, 0, 0, rc.right, rc.bottom, gbutton->hdcMem2, 0, 0, SRCCOPY);
				EndPaint(hWnd, &ps);
			}
			break;

		case WM_DESTROY:
			DeleteObject(hPen);
			break;
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}