/*!
GUI.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#pragma once
#include"gpu_info.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include<windows.h>
#include<Commctrl.h>
#include"graph.h"
#include"General_Info.h"
#include"CustomButton.h"
#include"resource.h"

#pragma comment(lib,"comctl32.lib")

#define WM_CHANGE_STATUS (WM_APP+1)
#define WM_CONNECT_RESULT (WM_APP+2)
#define WM_GET_IP (WM_APP+3)

using namespace std;
using namespace Gdiplus;

LRESULT CALLBACK StatusWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK DlgProc_ConnectSetting(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
DWORD WINAPI ConnectThread(LPVOID param);

HINSTANCE hInst;
SSH ssh;
int Button_Width;

int Init_GUI(HWND hWnd, General_Info& General_Info_1, vector<GPU_Info*>& GPU_Info_array, GraphButton& GeneralButton, vector<GraphButton*>& GPU_Button_array, map<string, string> cpu_info, vector<map<string, string>> gpu_info){
	RECT rc;
	wchar_t wtext[256];
	int i,delete_cnt;

	General_Info_1.Init_GUI(cpu_info);
	GeneralButton.Create(2, 10, 140, 60, 1, hWnd, hInst, 50);
	GeneralButton.SetColor(255, 100, 255);
	GeneralButton.SetTitle(L"General");
	GeneralButton.ShowFormat("CPU:%3.1f%%");

	GetClientRect(hWnd, &rc);
	for(i = 0; i < gpu_info.size(); ++i){
		if(GPU_Info_array.size() > i){
			wsprintf(wtext, L"GPU:%d", i);
			GPU_Info_array[i]->Init_GUI(gpu_info[i]);
			GPU_Button_array[i]->SetColor(100, 100, 255);
			GPU_Button_array[i]->SetTitle(wtext);
			GPU_Button_array[i]->ShowFormat("%3.1f%%");
			UpdateWindow(GPU_Button_array[i]->GethWnd());
		} else{
			wsprintf(wtext, L"GPU:%d", i);
			GPU_Info_array.push_back(new GPU_Info);
			GPU_Button_array.push_back(new GraphButton);
			GPU_Info_array[i]->Create(150, 10, rc.right - 150, 400, hWnd, hInst, gpu_info[i]);
			GPU_Button_array[i]->Create(2, 70 + 60 * i, 140, 60, 2 + i, hWnd, hInst, 50);
			GPU_Button_array[i]->SetColor(100, 100, 255);
			GPU_Button_array[i]->SetTitle(wtext);
			GPU_Button_array[i]->ShowFormat("%3.1f%%");
			UpdateWindow(GPU_Button_array[i]->GethWnd());
		}
	}
	if(i < GPU_Info_array.size()){
		delete_cnt = i;
		for(; i < GPU_Info_array.size(); ++i){
			DestroyWindow(GPU_Info_array[i]->GethWnd());
			DestroyWindow(GPU_Button_array[i]->GethWnd());
			delete GPU_Info_array[i];
			delete GPU_Button_array[i];
		}
		GPU_Info_array.erase(GPU_Info_array.begin() + delete_cnt, GPU_Info_array.end());
		GPU_Button_array.erase(GPU_Button_array.begin() + delete_cnt, GPU_Button_array.end());
		return 1;
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp){
	static vector<GPU_Info*> GPU_Info_array;
	static vector<GraphButton*> GPU_Button_array;
	static GraphButton GeneralButton;
	static General_Info General_Info_1;
	static HWND hStatus;
	static HWND Current_Wnd;
	static int cnt = 0;
	static CONNECT_INFO ci;
	static HANDLE th;
	DWORD result;
	MENUITEMINFO menuInfo = {0};
	RECT rc;
	wchar_t wtext[256];
	static int status = 0;
	static bool connect_start = false;

	switch(msg){
		case WM_CREATE:
			{
				GetClientRect(hWnd, &rc);
				hStatus = CreateWindow(L"Status Window", L"", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN , 0, rc.bottom - 30, rc.bottom, 30, hWnd, NULL, hInst, NULL);
				
				ReadSetting((char*)"setting.cfg",ci);

				string status = string("Try connecting to ") + ci.user + string("@") + ci.hostname + string(" ...");
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, status.c_str(), 256, wtext, 256);
				SendMessage(hStatus, WM_CHANGE_STATUS, sizeof(wtext), (LPARAM)wtext);

				th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ConnectThread, (LPVOID)hWnd, 0, NULL);

				vector<map<string, string>> gpu_dummy;
				map<string, string> cpu_dummy = {
					{"cpu_name","test_cpu"},
					{"cpu_util","26"},
					{"mem_total","16000000000"},
					{"mem_used","5000000000"},
					{"cpu_temp","42000"},
					{"cpu_freq","3800000000"},
				};

				Current_Wnd = General_Info_1.Create(150, 10, rc.right - 150, rc.bottom - 30, hWnd, hInst, cpu_dummy);
				Init_GUI(hWnd, General_Info_1, GPU_Info_array, GeneralButton, GPU_Button_array, cpu_dummy, gpu_dummy);
				ShowWindow(Current_Wnd, SW_SHOW);
				SendMessage(GeneralButton.GethWnd(), WM_BTN_SELECTED, 0, 0);
				SetFocus(GeneralButton.GethWnd());

				SetMenu(hWnd, LoadMenu(hInst ,MAKEINTRESOURCE(IDR_MENU1)));
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_STATE;
				menuInfo.fState = MFS_UNCHECKED;
				SetMenuItemInfo(GetMenu(hWnd), IDM_TOPMOST, FALSE, &menuInfo);

				

				/*LONG style = GetWindowLongPtr(hWnd, GWL_STYLE);
				style &= ~WS_OVERLAPPEDWINDOW;
				SetWindowLongPtr(hWnd, GWL_STYLE, style);
				SetMenu(hWnd, NULL);*/
				Button_Width = 150;
				SetTimer(hWnd, 0, 1000, NULL);
			}
			break;

		case WM_GET_IP:
			{
				CONNECT_INFO *c = (CONNECT_INFO*)lp;
				*c = ci;
				connect_start = false;
			}
			return 0;

		case WM_CONNECT_RESULT:
			if(wp != 0){
				string status = string("Failed connecting to ") + ci.user + string("@") + ci.hostname;
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, status.c_str(), 256, wtext, 256);
				SendMessage(hStatus, WM_CHANGE_STATUS, sizeof(wtext), (LPARAM)wtext);
			} else{
				string status = string("Connecting to ") + ci.user + string("@") + ci.hostname;
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, status.c_str(), 256, wtext, 256);
				SendMessage(hStatus, WM_CHANGE_STATUS, sizeof(wtext), (LPARAM)wtext);
				auto v = GetGPUInfo(ssh);
				auto c = GetGeneralInfo(ssh);
				if(Init_GUI(hWnd, General_Info_1, GPU_Info_array, GeneralButton, GPU_Button_array, c, v) == 1){
					Current_Wnd = General_Info_1.GethWnd();
					ShowWindow(Current_Wnd, SW_SHOW);
				}
				connect_start = true;
			}
			break;


		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* pmmi;
			pmmi = (MINMAXINFO*)lp;
			pmmi->ptMinTrackSize.x = 500;//ç≈è¨ïù
			pmmi->ptMinTrackSize.y = 400;//ç≈è¨çÇ
			return 0;//èàóùÇµÇΩÇÁ0Çï‘Ç∑
		}

		case WM_SIZE:
			GetClientRect(hWnd, &rc);
			
			SetWindowPos(General_Info_1.GethWnd(), NULL, Button_Width, 10, rc.right - Button_Width, rc.bottom - 30 - 10, SWP_NOZORDER);
			SetWindowPos(GeneralButton.GethWnd(), NULL, 2, 10, Button_Width - 5, 60, SWP_NOZORDER | SWP_NOMOVE);
			for(int i = 0; i < GPU_Info_array.size(); ++i){
				SetWindowPos(GPU_Info_array[i]->GethWnd(), NULL, Button_Width, 10, rc.right - Button_Width, rc.bottom - 30 - 10, SWP_NOZORDER);
				SetWindowPos(GPU_Button_array[i]->GethWnd(), NULL, 2, 70 + 60 * i, Button_Width - 5, 60, SWP_NOZORDER | SWP_NOMOVE);
			}
			SetWindowPos(hStatus, HWND_TOP, 0, rc.bottom - 30, rc.right, 30, 0);
			break;

		case WM_TIMER:
			{
			if(connect_start){
				auto v = GetGPUInfo(ssh);
				auto v_p = GetGPUProcess(ssh);
				for(int i = 0; i < v.size(); ++i){
					GPU_Info_array[i]->Update(v[i], v_p);
					GPU_Button_array[i]->Update(stof(v[i]["utilization.gpu"]));
				}

				auto c = GetGeneralInfo(ssh);
				auto c_p = GetGeneralProcess(ssh);
				General_Info_1.Update(c, c_p);
				if(c.size() >= 6){
					GeneralButton.Update(stof(c["cpu_util"]));

					string status = string("Connecting to ") + ci.user + string("@") + ci.hostname + string(" (") + c["time"] + string(")");
					MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, status.c_str(), 256, wtext, 256);
					SendMessage(hStatus, WM_CHANGE_STATUS, sizeof(wtext), (LPARAM)wtext);
				}
			}
				
			}
			break;

		case WM_COMMAND:
			if(LOWORD(wp) == 1){//general
				ShowWindow(Current_Wnd, SW_HIDE);
				ShowWindow(General_Info_1.GethWnd(), SW_SHOW);
				Current_Wnd = General_Info_1.GethWnd();
				SendMessage(GeneralButton.GethWnd(), WM_BTN_SELECTED, 0, 0);
			} else if(LOWORD(wp) >= 2 && LOWORD(wp) <= GPU_Info_array.size()+1){//gpu
				ShowWindow(Current_Wnd, SW_HIDE);
				ShowWindow(GPU_Info_array[LOWORD(wp) - 2]->GethWnd(), SW_SHOW);
				Current_Wnd = GPU_Info_array[LOWORD(wp) - 2]->GethWnd();
				SendMessage(GPU_Button_array[LOWORD(wp) - 2]->GethWnd(), WM_BTN_SELECTED, 0, 0);
			}
			if(LOWORD(wp) == ID_CONNECT_SETTING){//MENU
				if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETTING), hWnd, (DLGPROC)DlgProc_ConnectSetting, (LPARAM)&ci) == 0){
					if(ssh.connect_flag){
						ssh.DisConnect();
					}
					string status = string("Try connecting to ") + ci.user + string("@") + ci.hostname + string(" ...");
					MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, status.c_str(), 256, wtext, 256);
					SendMessage(hStatus, WM_CHANGE_STATUS, sizeof(wtext), (LPARAM)wtext);
					th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ConnectThread, (LPVOID)hWnd, 0, NULL);
				}
			} else if(LOWORD(wp) == IDM_TOPMOST){
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_STATE;
				GetMenuItemInfo(GetMenu(hWnd), IDM_TOPMOST, FALSE, &menuInfo);
				if(menuInfo.fState & MFS_CHECKED){
					menuInfo.fState = MFS_UNCHECKED;
					SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				} else {
					menuInfo.fState = MFS_CHECKED;
					SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				}
				SetMenuItemInfo(GetMenu(hWnd), IDM_TOPMOST, FALSE, &menuInfo);
				
			} else if(LOWORD(wp) == IDM_SSH_OPEN){
				STARTUPINFO         tStartupInfo = {0};
				PROCESS_INFORMATION tProcessInfomation = {0};
				char text[256];
				snprintf(text, 64, "cmd /K ssh %s@%s",ci.user,ci.hostname);
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				BOOL bResult = CreateProcess(
					NULL
					, (LPWSTR)wtext
					, NULL
					, NULL
					, FALSE
					, 0
					, NULL
					, NULL
					, &tStartupInfo
					, &tProcessInfomation
				);
				if(0 == bResult){
					int err = GetLastError();
					MessageBox(hWnd, L"ãNìÆÇ…é∏îsÇµÇ‹ÇµÇΩ", L"Error", MB_OK);
				}
			} else if(LOWORD(wp) == IDM_REMOTEDESKTOP){
				STARTUPINFO         tStartupInfo = {0};
				PROCESS_INFORMATION tProcessInfomation = {0};
				char text[256];
				snprintf(text, 64, "mstsc.exe /v %s", ci.hostname);
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				BOOL bResult = CreateProcess(
					NULL
					, (LPWSTR)wtext
					, NULL
					, NULL
					, FALSE
					, 0
					, NULL
					, NULL
					, &tStartupInfo
					, &tProcessInfomation
				);
				if(0 == bResult){
					int err = GetLastError();
					MessageBox(hWnd, L"ãNìÆÇ…é∏îsÇµÇ‹ÇµÇΩ", L"Error", MB_OK);
				}
			}
			return 0;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, msg, wp, lp);
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT CALLBACK StatusWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	RECT rc;
	HDC hdc;
	PAINTSTRUCT ps;
	HPEN oldPen;
	HFONT oldFont;
	HBRUSH hBrush;
	static HPEN hPen;
	static HFONT hFont;
	static wchar_t wtext[256] = L"Hello World";
	static CONNECT_INFO ci;

	switch(msg){
		case WM_CREATE:
			hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
			hFont = CreateFont(20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"ÉÅÉCÉäÉI");
			break;

		case WM_CHANGE_STATUS:
			memcpy(wtext, (void*)lp, wp);
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);

			FillRect(hdc, &rc, (HBRUSH)SelectObject(hdc, GetStockObject(WHITE_BRUSH)));

			oldPen = (HPEN)SelectObject(hdc, hPen);
			MoveToEx(hdc, 0, rc.bottom - 30, NULL);
			LineTo(hdc, rc.right, rc.bottom - 30);
			SelectObject(hdc, oldPen);

			oldFont = (HFONT)SelectObject(hdc, hFont);
			TextOut(hdc, 10, rc.bottom - 25, wtext, lstrlen(wtext));
			SelectObject(hdc, oldFont);

			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			DeleteObject(hPen);
			DeleteObject(hFont);
			break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

BOOL CALLBACK DlgProc_ConnectSetting(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	static CONNECT_INFO* ci;
	wchar_t wtext[256];
	char text[256];
	
	switch(msg){
		case WM_INITDIALOG:
			ci = (CONNECT_INFO*)lp;
			MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, ci->hostname.c_str(), -1, wtext, 64);
			SetWindowText(GetDlgItem(hWnd, IDC_IPADDRESS1), wtext);
			MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, ci->user.c_str(), -1, wtext, 64);
			SetWindowText(GetDlgItem(hWnd, IDC_EDIT_USER), wtext);
			MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, ci->pass.c_str(), -1, wtext, 64);
			SetWindowText(GetDlgItem(hWnd, IDC_EDIT_PASSWORD), wtext);
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 1);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wp)){
				case IDOK:
					GetWindowText(GetDlgItem(hWnd, IDC_IPADDRESS1), wtext, 256);
					WideCharToMultiByte(CP_ACP, 0, wtext, -1, text, 64, NULL, NULL);
					ci->hostname = text;
					GetWindowText(GetDlgItem(hWnd, IDC_EDIT_USER), wtext, 256);
					WideCharToMultiByte(CP_ACP, 0, wtext, -1, text, 64, NULL, NULL);
					ci->user = text;
					GetWindowText(GetDlgItem(hWnd, IDC_EDIT_PASSWORD), wtext, 256);
					WideCharToMultiByte(CP_ACP, 0, wtext, -1, text, 64, NULL, NULL);
					ci->pass = text;
					WriteSetting((char*)"setting.cfg", *ci);
					EndDialog(hWnd, 0);
					break;

				case IDCANCEL:
					EndDialog(hWnd, 1);
					break;
			}
			return TRUE;
	}
	return FALSE;
}

DWORD WINAPI ConnectThread(LPVOID param){
	HWND hWnd = (HWND)param;
	CONNECT_INFO ci;
	char text[256];
	wchar_t wtext[256];
	int result;

	SendMessage(hWnd, WM_GET_IP, 0, (LPARAM)&ci);
	snprintf(text, 256, "Remote Performance Monitor[%s@%s]", ci.user.c_str(), ci.hostname.c_str());
	MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, -1, wtext, 256);
	SetWindowText(hWnd, wtext);

	result = ssh.Connect((char*)ci.hostname.c_str(), (char*)ci.user.c_str(), (char*)ci.pass.c_str());
	SendMessage(hWnd, WM_CONNECT_RESULT, result, 0);

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	WNDCLASSEX wc = {0};
	hInst = hInstance;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	InitCommonControls();

	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"Remote Monitor";
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClassEx(&wc)){
		return -1;
	}

	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"Status Window";
	wc.hInstance = hInstance;
	wc.lpfnWndProc = StatusWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if(!RegisterClassEx(&wc)){
		return -1;
	}

	HWND hWnd = CreateWindow(L"Remote Monitor", L"Remote Monitor", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 500, NULL, NULL, hInstance, 0);

	setlocale(LC_CTYPE,"");
	int result;
	MSG msg;
	while(1){
		result = GetMessage(&msg, NULL, 0, 0);
		if(result == 0 || result == -1) break;

		if(!IsDialogMessage(hWnd, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	GdiplusShutdown(gdiplusToken);

	return 0;
}

