/*!
General_Info.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#define _CRT_SECURE_NO_WARNINGS
#include"General_Info.h"
#include<CommCtrl.h>
#include<uxtheme.h>
#include<map>
#include"util.h"
#include"resource.h"

#pragma comment(lib,"UxTheme.lib")

#define SPLIT_MIN 5

using namespace std;

int MaxMinPoint(LPARAM lParam, RECT rc);
LRESULT CALLBACK ListProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

bool General_Info::wndclass_regist = false;
std::map< HWND, General_Info*> General_Info::ginfohash;

extern int Button_Width;
extern SSH ssh;

General_Info::General_Info(){
}

General_Info* General_Info::getObjPtr(HWND hWnd){
	map< HWND, General_Info* >::iterator it = General_Info::ginfohash.find(hWnd);
	if(it != General_Info::ginfohash.end()){
		return it->second;
	}
	return NULL;
};

void General_Info::regist(General_Info* pGinfo){
	// ボタンのウィンドウハンドル取得
	HWND hWnd = pGinfo->hWnd;
	// バイナリツリーに登録
	General_Info::ginfohash.insert(make_pair(hWnd, pGinfo));
};

int General_Info::RegisterWndClass(HINSTANCE hInst_){
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"General_Info Class";
	wc.hInstance = hInst_;
	wc.lpfnWndProc = GlobalWindowProc;
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
	wndclass_regist = true;
	return 0;
}

void General_Info::Init_GUI(map<string, string> info){
	ClearData();
	util.SetColor(255, 100, 255);
	util.SetShowInfoFormat("%3.0f%%");
	util.SetminmaxFormat("%3.0f%%");
	util.SetMinMax(0, 100);
	util.SetName(L"cpu");

	mem.SetColor(255, 100, 255);
	mem.SetShowInfoFormat("%3.1f/%3.1f GiB");
	mem.SetminmaxFormat("%3.0f%%");
	mem.SetMinMax(0, stof(info["mem_total"]) / 1024.0 / 1024);
	mem.SetName(L"memory");
	mem.show_fraction_flag = true;

	temp.SetColor(255, 100, 255);
	temp.SetShowInfoFormat("%3.1f℃");
	temp.SetminmaxFormat("%3.0f℃");
	temp.SetMinMax(0, 100);
	temp.SetName(L"temp");

	freq.SetColor(255, 100, 255);
	freq.SetShowInfoFormat("%3.2fGHz");
	freq.SetminmaxFormat("%3.0fGHz");
	freq.SetMinMax(0, 5);
	freq.SetName(L"freq");
	freq.mem_val_flag = true;

}

HWND General_Info::Create(int x, int y, int width, int height, HWND hParent_, HINSTANCE hInst_, map<string, string> info){
	hParent = hParent_;
	hInst = hInst_;

	Init_GUI(info);
	if(!wndclass_regist){
		RegisterWndClass(hInst);
	}


	if(hWnd != NULL){
		return NULL;
	}
	hWnd = CreateWindow(L"General_Info Class", L"Graph", WS_CHILD | WS_CLIPCHILDREN , x, y, width, height, hParent, NULL, hInst, this);
	if(hWnd != NULL){
		regist(this);
	}

	return hWnd;
}

HWND General_Info::GethWnd(){
	return hWnd;
}

void General_Info::ClearData(){
	util.ClearData();
	mem.ClearData();
	temp.ClearData();
	freq.ClearData();
}

void General_Info::Update(map<string, string> info, vector<vector<string>> process_){
	if(info.size() >= 6){
		cpu_name = info["cpu_name"];
		val_mem_total = stof(info["mem_total"]) / 1024.0 / 1024;
		val_mem_used = stof(info["mem_used"]) / 1024.0 / 1024;
		val_cpu_util = stof(info["cpu_util"]);
		val_temp = stof(info["cpu_temp"]);
		val_temp /= 1000.0;
		val_freq = stof(info["cpu_freq"])/1e6;
		uptime = info["uptime"];

		InvalidateRect(hWnd, NULL, FALSE);
		util.update(val_cpu_util);
		mem.update(val_mem_used);
		temp.update(val_temp);
		freq.update(val_freq);
	}
	if(process_.size() >= 1){
		int count = 0;
		wchar_t wtext[256];
		char text[256];
		process.clear();
		for(auto itr = process_.begin(); itr != process_.end(); ++itr){
			vector<string> temp = {(*itr)[1],(*itr)[10],(*itr)[0],(*itr)[2],(*itr)[3]};
			process.emplace_back(temp);
			++count;
		}
		Sort(sort_index,sort_direction);
		ListView_SetItemCountEx(hList, count, LVSICF_NOINVALIDATEALL);
	}
}

void General_Info::Sort(int index,int dir){
	if(dir){
		switch(index){
			case 0:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stoi(alpha[index]) > stoi(beta[index])); });
				break;
			case 1:
			case 2:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (alpha[index] > beta[index]); });
				break;
			case 3:
			case 4:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stof(alpha[index]) > stof(beta[index])); });
				break;
		}
	} else{
		switch(index){
			case 0:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stoi(alpha[index]) < stoi(beta[index])); });
				break;
			case 1:
			case 2:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (alpha[index] < beta[index]); });
				break;
			case 3:
			case 4:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stof(alpha[index]) < stof(beta[index])); });
				break;
		}
	}
}


LRESULT CALLBACK General_Info::GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	static HFONT hFont_id, hFont_Name,hFont_s_title;
	static HPEN hPen;
	static HMENU tmp_menu, popupmenu;
	HFONT oldFont;
	HPEN oldPen;
	HBRUSH oldBrush;
	HDC hdc;
	PAINTSTRUCT ps;
	SIZE size,size_2;
	RECT rc;
	LVCOLUMN col = {0};
	LVITEM item = {0};
	char text[256];
	wchar_t wtext[256];
	int iCount = 0;

	General_Info* general_info = getObjPtr(hWnd);

	if(general_info != NULL || msg == WM_CREATE){
		switch(msg){
			case WM_CREATE:
				general_info = (General_Info*)(((CREATESTRUCT*)lp)->lpCreateParams);
				GetClientRect(hWnd, &rc);

				general_info->util.Create(10, 50, 200, 120, hWnd, general_info->hInst);
				general_info->mem.Create(220, 50, 200, 120, hWnd, general_info->hInst);
				general_info->temp.Create(10, 170, 200, 120, hWnd, general_info->hInst);
				general_info->freq.Create(220, 170, 200, 120, hWnd, general_info->hInst);

				general_info->hList = CreateWindowEx(0, WC_LISTVIEW, 0, WS_CHILD | WS_VISIBLE |  LVS_REPORT   | LVS_OWNERDATA ,
					10, 350, 420, 200, hWnd, (HMENU)1, general_info->hInst, NULL);
				SendMessage(general_info->hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
				col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH ;
				col.fmt = LVCFMT_LEFT;
				col.cx = 50;
				col.pszText = (LPWSTR)L"pid";
				ListView_InsertColumn(general_info->hList, 0, &col);
				col.pszText = (LPWSTR)L"process";
				col.cx = rc.right - 250;
				ListView_InsertColumn(general_info->hList, 1, &col);
				col.pszText = (LPWSTR)L"user";
				col.cx = 60;
				ListView_InsertColumn(general_info->hList, 2, &col);
				col.pszText = (LPWSTR)L"cpu(%)";
				col.cx = 60;
				ListView_InsertColumn(general_info->hList, 3, &col);
				col.pszText = (LPWSTR)L"mem(%)"; 
				col.cx = 60;
				ListView_InsertColumn(general_info->hList, 4, &col);	
				
				if(hFont_Name == NULL){
					hFont_Name = CreateFont(20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				}
				if(hFont_id == NULL){
					hFont_id = CreateFont(30, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				}
				if(hFont_s_title == NULL){
					hFont_s_title = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				}
				if(hPen == NULL){
					hPen = CreatePen(PS_SOLID,1,RGB(220,220,220));
				}

				hdc = GetDC(hWnd);
				general_info->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				general_info->hdcMem = CreateCompatibleDC(hdc);		// カレントスクリーン互換
				SelectObject(general_info->hdcMem, general_info->hBitmap);		// MDCにビットマップを割り付け
				ReleaseDC(hWnd, hdc);


				tmp_menu = LoadMenu(general_info->hInst, MAKEINTRESOURCE(IDR_MENU2));
				popupmenu = GetSubMenu(tmp_menu, 0);
				SetWindowSubclass(general_info->hList, ListProc, 1, 0);
				break;
				

			case WM_SIZE:
				GetClientRect(hWnd, &rc);
				
				SetWindowPos(general_info->util.GethWnd(), NULL, 10, 50, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				SetWindowPos(general_info->mem.GethWnd(), NULL, 10 + (rc.right - 40) / 2 + 10, 50, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				SetWindowPos(general_info->temp.GethWnd(), NULL, 10, 170, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				SetWindowPos(general_info->freq.GethWnd(), NULL, 10 + (rc.right - 40) / 2 + 10, 170, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				
				hdc = GetDC(hWnd);
				DeleteObject(general_info->hdcMem);
				DeleteObject(general_info->hBitmap);
				general_info->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				general_info->hdcMem = CreateCompatibleDC(hdc);		// カレントスクリーン互換
				SelectObject(general_info->hdcMem, general_info->hBitmap);		// MDCにビットマップを割り付け
				ReleaseDC(hWnd, hdc);

				SetWindowPos(general_info->hList, NULL, 10, 340, rc.right - 10, rc.bottom - 340 - 10, SWP_NOZORDER);
				{
					GetClientRect(general_info->hList, &rc);
					int width = 0;
					col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
					for(int i = 0; i <= 4; i++){
						if(i == 1) continue;
						ListView_GetColumn(general_info->hList, i, &col);
						width += col.cx;
					}
					width += 30;
					ListView_SetColumnWidth(general_info->hList, 1, rc.right - width);
				}
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case WM_NOTIFY:
			{
				LPNMHDR lpnmhdr = (LPNMHDR)lp;
				int col, row;
				if(lpnmhdr->hwndFrom == general_info->hList){   // BINTABLEからのメッセージであることをチェック
					switch(lpnmhdr->code){
						case LVN_GETDISPINFO:
						{
							LV_DISPINFO* pLvDispInfo = (LV_DISPINFO*)lp;
							TCHAR szString[MAX_PATH] = L"";

							if(pLvDispInfo->item.mask & LVIF_TEXT){       // TEXTならば
								col = pLvDispInfo->item.iSubItem;       // 列番号
								row = pLvDispInfo->item.iItem;          // 行番号
								if(general_info->process.size() > row && general_info->process[row].size() > col){
									MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, general_info->process[row][col].c_str(), MAX_PATH, szString, MAX_PATH);
								}
								lstrcpy(pLvDispInfo->item.pszText, szString);
							}
						}
						return TRUE;

						case LVN_COLUMNCLICK:
						{
							NMLISTVIEW* pListView = (NMLISTVIEW*)lp;

							if(general_info->sort_index == pListView->iSubItem){
								general_info->sort_direction = !general_info->sort_direction;
							} else{
								general_info->sort_direction = true;
							}
							general_info->sort_index = pListView->iSubItem;
							general_info->Sort(general_info->sort_index, general_info->sort_direction);
							InvalidateRect(general_info->hList, NULL, FALSE);
						}
						return TRUE;

						case NM_RCLICK:
						{
							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lp;
							if(lpnmitem->iItem >= 0){
								POINT point;
								GetCursorPos(&point);
								TrackPopupMenu(popupmenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, point.x, point.y, 0, hWnd, NULL);
							}
							InvalidateRect(general_info->hList, NULL, FALSE);
						}
						return TRUE;

						default:
							break;
					}       
				}        
			}
			break;

			case WM_COMMAND:
				switch(LOWORD(wp)){
					case IDM_KILL:
						{
							int i;
							i = ListView_GetNextItem(general_info->hList, -1, LVIS_SELECTED);
							if(i >= 0){
								snprintf(text, 64, "pid:%s %s\nKill?", general_info->process[i][0].c_str(), general_info->process[i][1].c_str());
								MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
								if(MessageBox(hWnd, wtext, L"Task Kill", MB_YESNO) == IDYES){
									KillCmd(ssh, general_info->process[i][0]);
								}
							}
						}
						break;
				}
				return 0;

			case WM_LBUTTONDOWN:  // マウスの左ボタンが押されたとき
		   // マウスキャプチャを開始する
				if(LOWORD(lp) < 5){
					SetCapture(hWnd);
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
					general_info->push = true;        // 左ボタンが押された
				}
				break;

			case WM_LBUTTONUP:   // マウスの左ボタンが離されたとき
				if(general_info->push){
					RECT prc;

					GetWindowRect(hWnd, &rc);
					GetClientRect(general_info->hParent, &prc);
					// マウスカーソルの位置を取得して、終点として保存しておく
					short dx = LOWORD(lp);
					Button_Width += dx;
					if(Button_Width < 5){
						Button_Width = 5;
					} else if(Button_Width > (prc.right - prc.left) - 5){
						Button_Width = (prc.right - prc.left) - 5;
					}
					general_info->push = false;      // 左ボタンが押されてない
					// マウスキャプチャを終了する
					ReleaseCapture();
					//SetWindowPos(hWnd, NULL, Button_Width, 10, prc.right - Button_Width, prc.bottom - 35, SWP_NOZORDER);
					PostMessage(general_info->hParent, WM_SIZE, SIZE_RESTORED,((prc.bottom - prc.top) << 16) | (prc.right - prc.left));
				}
				break;

			case WM_MOUSEMOVE:      // マウスカーソルが移動したとき
				if(general_info->push){
					RECT prc;

					GetWindowRect(hWnd, &rc);
					GetClientRect(general_info->hParent, &prc);
					// マウスカーソルの位置を取得して、終点として保存しておく
					short dx = LOWORD(lp);
					Button_Width += dx;
					if(Button_Width < 5){
						Button_Width = 5;
					} else if(Button_Width > (prc.right - prc.left) - 5){
						Button_Width = (prc.right - prc.left) - 5;
					}
					//SetWindowPos(hWnd, NULL, Button_Width, 10, prc.right - Button_Width, prc.bottom - 35, SWP_NOZORDER);
					PostMessage(general_info->hParent, WM_SIZE, SIZE_RESTORED, ((prc.bottom - prc.top) << 16) | (prc.right - prc.left));
					InvalidateRect(hWnd, NULL, TRUE);
				} else if((0 <= (LOWORD(lp) + 5))
					&& (0 >= (LOWORD(lp) - 5))){
					SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				} else{
					SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
				break;

			//case WM_ERASEBKGND:
			//	return 1;

			case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rc);

				oldPen = (HPEN)SelectObject(general_info->hdcMem, GetStockObject(NULL_PEN));
				oldBrush = (HBRUSH)SelectObject(general_info->hdcMem, GetStockObject(WHITE_BRUSH));
				Rectangle(general_info->hdcMem, 0, 0, rc.right+1, rc.bottom+1);
				SelectObject(general_info->hdcMem, oldBrush);
				SelectObject(general_info->hdcMem, oldPen);

				oldPen = (HPEN)SelectObject(general_info->hdcMem, hPen);
				MoveToEx(general_info->hdcMem, 1, 0, NULL);
				LineTo(general_info->hdcMem, 1, rc.bottom);
				SelectObject(general_info->hdcMem, oldPen);

				oldFont = (HFONT)SelectObject(general_info->hdcMem, hFont_id);
				snprintf(text, 64, "General");
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				GetTextExtentPoint32(general_info->hdcMem, wtext, lstrlen(wtext), &size);
				TextOut(general_info->hdcMem, 10, 10, wtext, lstrlen(wtext));

				SelectObject(general_info->hdcMem, hFont_Name);
				snprintf(text, 64, "CPU:%s", general_info->cpu_name.c_str());
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				GetTextExtentPoint32(general_info->hdcMem, wtext, lstrlen(wtext), &size_2);
				if(10 + size.cx + 10 > rc.right - size_2.cx - 10){
					TextOut(general_info->hdcMem, 10 + size.cx + 10, 10 + size_2.cy / 2, wtext, lstrlen(wtext));
				} else{
					TextOut(general_info->hdcMem, rc.right - size_2.cx - 10, 10 + size_2.cy / 2, wtext, lstrlen(wtext));
				}
				SelectObject(general_info->hdcMem, oldFont);

				//uptime
				SetBkMode(general_info->hdcMem, TRANSPARENT);
				SelectObject(general_info->hdcMem, hFont_s_title);
				snprintf(text, 64, "Operating Time");
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				GetTextExtentPoint32(general_info->hdcMem, wtext, lstrlen(wtext), &size_2);
				SetTextColor(general_info->hdcMem, RGB(100, 100, 100));
				TextOut(general_info->hdcMem, 10, 290, wtext, lstrlen(wtext));

				SelectObject(general_info->hdcMem, hFont_id);
				snprintf(text, 64, "%s", general_info->uptime.c_str());
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				GetTextExtentPoint32(general_info->hdcMem, wtext, lstrlen(wtext), &size_2);
				SetTextColor(general_info->hdcMem, RGB(0, 0, 0));
				TextOut(general_info->hdcMem, 10, 305, wtext, lstrlen(wtext));
				SelectObject(general_info->hdcMem, oldFont);

				BitBlt(hdc, 0, 0, rc.right, rc.bottom, general_info->hdcMem, 0, 0, SRCCOPY);
				EndPaint(hWnd, &ps);
				break;

			case WM_DESTROY:
				DeleteObject(hFont_id);
				DeleteObject(hFont_Name);
				DeleteObject(hPen);
				break;
		}
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

map<string, string> GetGeneralInfo(SSH& ssh){
	char buffer[0x4000] = {0};
	//vector<string> dummy;	
	map<string, string> info;

	if(ssh.connect_flag){
		string cpu_cmd = "vmstat";
		ssh.ExecCmd((char*)cpu_cmd.c_str(), buffer);
		auto g_info = csv2vector(pack_space(buffer),0,' ');
		/*for(auto itr = g_info[2].begin(); itr != g_info[2].end(); ++itr){
			if(itr->compare("") != 0){
				dummy.push_back(*itr);
			}
		}*/
		info["cpu_util"] = to_string(100 - stoi(g_info[2][15]));

		string temp_file = "/sys/devices/platform/coretemp.?/hwmon/hwmon?/temp?_input";
		memset(buffer, 0, sizeof(buffer));
		string temp_cmd = "cat ";
		temp_cmd += temp_file;
		ssh.ExecCmd((char*)temp_cmd.c_str(), buffer);
		g_info = csv2vector(buffer, 0, ':');
		float temp_avg = 0;
		for(int i = 0; i < g_info.size(); i++){
			temp_avg += stof(g_info[i][0]);
		}
		temp_avg /= g_info.size();
		info["cpu_temp"] = to_string(temp_avg);
		//dummy.push_back(buffer);

		string mem_cmd = "free";
		memset(buffer, 0, sizeof(buffer));
		ssh.ExecCmd((char*)mem_cmd.c_str(), buffer);
		g_info = csv2vector(pack_space(buffer), 0, ' ');
		/*for(auto itr = g_info[1].begin(); itr != g_info[1].end(); ++itr){
			if(itr->compare("") != 0){
				//dummy.push_back(*itr);
			}
		}*/
		info["mem_total"] = g_info[1][1];
		info["mem_used"] = g_info[1][2];

		memset(buffer, 0, sizeof(buffer));
		ssh.ExecCmd((char*)"cat /proc/cpuinfo | grep \"model name\" | uniq", buffer);
		
		g_info = csv2vector(buffer, 0, ':');
		info["cpu_name"] = g_info[0][1];
		//dummy.push_back(g_info[0][1]);

		string freq_file = "/sys/devices/system/cpu/cpu?/cpufreq/scaling_cur_freq";
		memset(buffer, 0, sizeof(buffer));
		string freq_cmd = "cat ";
		freq_cmd += freq_file;
		ssh.ExecCmd((char*)freq_cmd.c_str(), buffer);
		g_info = csv2vector(buffer, 0, ':');
		float freq_avg = 0;
		for(int i = 0; i < g_info.size(); i++){
			freq_avg += stof(g_info[i][0]);
		}
		freq_avg /= g_info.size();
		info["cpu_freq"] = to_string(freq_avg);
		//dummy.push_back(to_string(freq_avg));

		memset(buffer, 0, sizeof(buffer));
		ssh.ExecCmd((char*)"date +%s", buffer);
		time_t now = stol(buffer);
		struct tm* ptm;
		ptm = localtime(&now);
		strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", ptm);
		info["time"] = buffer;

		memset(buffer, 0, sizeof(buffer));
		ssh.ExecCmd((char*)"cat /proc/uptime", buffer);
		g_info = csv2vector(buffer, 0, ' ');
		unsigned long long uptime = stof(g_info[0][0]);
		unsigned int d, h, m, s,temp;
		d = uptime / (3600 * 24);
		temp = uptime % (3600 * 24);
		h = temp / 3600;
		temp = temp % 3600;
		m = temp / 60;
		s = temp % 60;
		snprintf(buffer, 256, "%d:%02d:%02d:%02d", d, h, m, s);
		info["uptime"] = buffer;

		return info;
	}
	
	return info;
}


vector<vector<string>> GetGeneralProcess(SSH& ssh){
	char buffer[0x4000] = {0};
	vector<vector<string>> dummy;
	string gpu_cmd = "ps auc";

	if(ssh.connect_flag){
		ssh.ExecCmd((char*)gpu_cmd.c_str(), buffer);
		string a, b = buffer;
		while(1){
			a = replace(b, "  ", " ");
			if(a == b) break;
			b = a;
		}
		dummy = csv2vector(a,1,(char)' ');

		return dummy;
	}
	
	return dummy;

}

LRESULT CALLBACK ListProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData){
	switch(msg){
		case WM_TIMER:
			KillTimer(hWnd, wp);
			return 0;
	}
	return DefSubclassProc(hWnd, msg, wp, lp);
}