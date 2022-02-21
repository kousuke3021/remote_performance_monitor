/*!
graph.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include"graph.h"
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include<vector>

// ライブラリ
#pragma comment( lib, "gdiplus.lib" )

#define WM_GRAPH_CHANGE_COLOR (WM_APP+1)
#define WM_GRAPH_UPDATE (WM_APP+2)

using namespace std;
using namespace Gdiplus;

bool Graph::wndclass_regist = false;
std::map< HWND, Graph*> Graph::grhhash;

Graph::Graph(size_t size_){
	size = size_;
	data.resize(100, 0);
	hWnd = NULL;
	hParent = NULL;
	hInst = NULL;
	BaseColor_R = 200;
	BaseColor_G = 200;
	BaseColor_B = 200;
}

int Graph::RegisterWndClass(HINSTANCE hInst){
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"Graph Class";
	wc.hInstance = hInst;
	wc.lpfnWndProc = GlobalWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClassEx(&wc)){
		return -1;
	}
	wndclass_regist = true;
	return 0;
}

Graph* Graph::getObjPtr(HWND hWnd){
	map< HWND, Graph* >::iterator it = Graph::grhhash.find(hWnd);
	if(it != Graph::grhhash.end()){
		return it->second;
	}
	return NULL;
}

void Graph::regist(Graph* pGrh){
	// ボタンのウィンドウハンドル取得
	HWND hWnd = pGrh->hWnd;
	// バイナリツリーに登録
	Graph::grhhash.insert(make_pair(hWnd, pGrh));
}

HWND Graph::GethWnd(){
	return hWnd;
}

HWND Graph::Create(int x, int y, int width, int height, HWND hParent_,HINSTANCE hInst_){
	hParent = hParent_;
	hInst = hInst_;

	if(!wndclass_regist){
		if(RegisterWndClass(hInst)) return NULL;
	}
	if(hWnd != NULL){
		return NULL;
	}
	hWnd = CreateWindow(L"Graph Class", L"Graph", WS_VISIBLE | WS_CHILD, x, y, width, height, hParent, NULL, hInst, this);
	
	if(hWnd != NULL){
		regist(this);
	}
	return hWnd;
}

Color Graph::SetColor(int R_, int G_, int B_){
	Color pre_color = Color(BaseColor_R, BaseColor_G, BaseColor_B);
	BaseColor_R = R_;
	BaseColor_G = G_;
	BaseColor_B = B_;

	if(hWnd != NULL) SendMessage(hWnd, WM_GRAPH_CHANGE_COLOR, 0, 0);

	return pre_color;
}

void Graph::update(float value){
	data.push_front(value);
	data.pop_back();
	if(hWnd != NULL) InvalidateRect(hWnd, NULL, FALSE);
}

void Graph::val2pos(float val,int time,int&x,int&y,RECT rc){
	if(hWnd == NULL){
		x = -1;
		y = -1;
	}
	y = (float)(1 - (float)(val - min) / (max - min)) * (rc.bottom - rc.top) + rc.top;
	x = (float)(1 - (float)time / size) * (rc.right - rc.left) + rc.left;
}

void Graph::SetShowInfoFormat(const char* format_){
	format_current = string(format_);
}

void Graph::SetminmaxFormat(const char* format_){
	format_scale = string(format_);
}

void Graph::SetName(const wchar_t* name_){
	name = wstring(name_);
}

void Graph::SetMinMax(float min_, float max_){
	min = min_;
	max = max_;
}

void Graph::ClearData(){
	data.clear();
	data.resize(100, 0);
}

HDC Graph::Paint(HWND hWnd, Graph* graph){
	RECT rc, graph_rect;
	HFONT oldFont;
	HBRUSH oldBrush;
	wchar_t wtext[64];
	char text[64];

	GetClientRect(hWnd, &rc);
	if(graph->only_graph){
		graph_rect = {rc.left ,rc.top ,rc.right - 1,rc.bottom - 1};
	} else{
		graph_rect = {rc.left + graph->scale_cx + 3,rc.top + 18,rc.right - 1,rc.bottom - 20};
	}

	Graphics offScreen(graph->hdcMem);

	SolidBrush  BkBrush(Color(255, 255, 255));
	offScreen.FillRectangle(&BkBrush, 0, 0, rc.right, rc.bottom);
	if(graph->grid_flag){
		//grid
		Pen oPen_grid(Color(220, 220, 220));
		for(int i = 0; i < 10; i++){
			int x, y;

			graph->val2pos(graph->max / 10.0 * i, 0, x, y, graph_rect);
			offScreen.DrawLine(&oPen_grid, graph_rect.left, y, graph_rect.right, y);
		}
	}

	//data
	GraphicsPath oPath;
	Pen oPen_border(Color(graph->BaseColor_R, graph->BaseColor_G, graph->BaseColor_B));
	SolidBrush  solidBrush(Color(50, graph->BaseColor_R, graph->BaseColor_G, graph->BaseColor_B));

	offScreen.SetSmoothingMode(SmoothingModeAntiAlias);
	oPath.AddLine(Point(graph_rect.right, graph_rect.bottom), Point(graph_rect.left, graph_rect.bottom));
	{
		int x, y, prev_x, prev_y;
		graph->val2pos(graph->data.at(graph->size - 1), graph->size - 1, prev_x, prev_y, graph_rect);
		oPath.AddLine(Point(graph_rect.left, graph_rect.bottom), Point(prev_x, prev_y));
		for(int tim = graph->size - 2; tim >= 0; --tim){
			graph->val2pos(graph->data.at(tim), tim, x, y, graph_rect);
			if(y < graph_rect.top){
				y = graph_rect.top;
			}
			offScreen.DrawLine(&oPen_border, prev_x, prev_y, x, y);
			oPath.AddLine(Point(prev_x, prev_y), Point(x, y));
			prev_x = x;
			prev_y = y;
		}
		oPath.AddLine(Point(prev_x, prev_y), Point(graph_rect.right, graph_rect.bottom));
	}
	oPath.CloseAllFigures();
	
	offScreen.FillPath(&solidBrush, &oPath);
	offScreen.DrawLine(&oPen_border, (int)graph_rect.left, (int)graph_rect.top, (int)graph_rect.right, (int)graph_rect.top);
	offScreen.DrawLine(&oPen_border, (int)graph_rect.right, (int)graph_rect.top, (int)graph_rect.right, (int)graph_rect.bottom);
	offScreen.DrawLine(&oPen_border, (int)graph_rect.right, (int)graph_rect.bottom, (int)graph_rect.left, (int)graph_rect.bottom);
	offScreen.DrawLine(&oPen_border, (int)graph_rect.left, (int)graph_rect.bottom, (int)graph_rect.left, (int)graph_rect.top);

	if(!graph->only_graph){
		//Show Info
		SIZE size;

		oldFont = (HFONT)SelectObject(graph->hdcMem, graph->hFont_data);
		SetTextColor(graph->hdcMem, RGB(100, 100, 100));
		if(graph->show_fraction_flag){
			snprintf(text, 64, graph->format_current.c_str(), graph->data.at(0), graph->max);
		} else{
			snprintf(text, 64, graph->format_current.c_str(), graph->data.at(0));
		}
		MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
		GetTextExtentPoint32(graph->hdcMem, wtext, lstrlen(wtext), &size);
		TextOut(graph->hdcMem, graph_rect.right - size.cx, 0, wtext, lstrlen(wtext));
		SelectObject(graph->hdcMem, oldFont);

		//graph mem

		oldFont = (HFONT)SelectObject(graph->hdcMem, graph->hFont_scale);
		SetTextColor(graph->hdcMem, RGB(100, 100, 100));
		//max
		if(graph->mem_val_flag){
			snprintf(text, 64, graph->format_scale.c_str(), graph->max);
		} else{
			snprintf(text, 64, graph->format_scale.c_str(), 100.0);
		}
		MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
		GetTextExtentPoint32(graph->hdcMem, wtext, lstrlen(wtext), &size);
		TextOut(graph->hdcMem, 0, graph_rect.top - size.cy / 2, wtext, lstrlen(wtext));
		graph->scale_cx = size.cx;
		//min
		snprintf(text, 64, graph->format_scale.c_str(), graph->min);
		MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
		GetTextExtentPoint32(graph->hdcMem, wtext, lstrlen(wtext), &size);
		TextOut(graph->hdcMem, 0, graph_rect.bottom - size.cy / 2, wtext, lstrlen(wtext));
		graph->scale_cx = (graph->scale_cx < size.cx) ? size.cx : graph->scale_cx;
		SelectObject(graph->hdcMem, oldFont);
		
		//graph title
		oldFont = (HFONT)SelectObject(graph->hdcMem, graph->hFont_title);
		SetTextColor(graph->hdcMem, RGB(0, 0, 0));
		GetTextExtentPoint32(graph->hdcMem, graph->name.c_str(), lstrlen(graph->name.c_str()), &size);
		TextOut(graph->hdcMem, graph_rect.left, 0, graph->name.c_str(), lstrlen(graph->name.c_str()));
		SelectObject(graph->hdcMem, oldFont);
	}

	return graph->hdcMem;
}

LRESULT CALLBACK Graph::GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	RECT rc;
	HDC hdc;
	PAINTSTRUCT ps;
	HFONT oldFont;
	HBRUSH oldBrush;
	wchar_t wtext[64];
	char text[64];
	
	Graph* graph = getObjPtr(hWnd);
	if(graph != NULL || msg == WM_CREATE){
		switch(msg){
			case WM_CREATE:
				graph = (Graph*)(((CREATESTRUCT*)lp)->lpCreateParams);
				graph->hFont_data = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,L"メイリオ");
				graph->hFont_scale = CreateFont(12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				graph->hFont_title = CreateFont(18, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				{
					SIZE size;

					hdc = GetDC(hWnd);
					oldFont = (HFONT)SelectObject(hdc, graph->hFont_scale);
					//max
					snprintf(text, 64, graph->format_scale.c_str(), graph->max);
					MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
					GetTextExtentPoint32(hdc, wtext, lstrlen(wtext), &size);
					graph->scale_cx = size.cx;
					//min
					snprintf(text, 64, graph->format_scale.c_str(), graph->min);
					MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
					GetTextExtentPoint32(hdc, wtext, lstrlen(wtext), &size);
					graph->scale_cx = (graph->scale_cx < size.cx) ? size.cx : graph->scale_cx;
					SelectObject(hdc, oldFont);

					GetClientRect(hWnd, &rc);  	// デスクトップのサイズを取得
					graph->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
					graph->hdcMem = CreateCompatibleDC(NULL);		// カレントスクリーン互換
					SelectObject(graph->hdcMem, graph->hBitmap);		// MDCにビットマップを割り付け

					ReleaseDC(hWnd, hdc);
				}
				break;

			case WM_SIZE:
				hdc = GetDC(hWnd);

				DeleteObject(graph->hdcMem);
				DeleteObject(graph->hBitmap);
				GetClientRect(hWnd, &rc);  	// デスクトップのサイズを取得
				graph->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				graph->hdcMem = CreateCompatibleDC(NULL);		// カレントスクリーン互換
				SelectObject(graph->hdcMem, graph->hBitmap);		// MDCにビットマップを割り付け

				ReleaseDC(hWnd, hdc);
				break;


			case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rc);
				BitBlt(hdc, 0, 0, rc.right, rc.bottom, graph->Paint(hWnd,graph), 0, 0, SRCCOPY);

				EndPaint(hWnd, &ps);
				break;

			case WM_GRAPH_CHANGE_COLOR:
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case WM_DESTROY:
				DeleteObject(graph->hFont_data);
				DeleteObject(graph->hFont_scale);
				DeleteObject(graph->hFont_title);
				DeleteObject(graph->hdcMem);
				DeleteObject(graph->hBitmap);
				Graph::grhhash.erase(hWnd);
				break;

			case WM_ERASEBKGND:
				return 1;

			default:
				return DefWindowProc(hWnd, msg, wp, lp);
		}	
		
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}