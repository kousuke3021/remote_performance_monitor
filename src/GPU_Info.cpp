/*!
GPU_Info.cpp

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#include "gpu_info.h"
#include <commctrl.h>
#include"resource.h"
using namespace std;

bool GPU_Info::wndclass_regist = false;
std::map< HWND, GPU_Info*> GPU_Info::ginfohash;

extern int Button_Width;
extern SSH ssh;

GPU_Info::GPU_Info(){

}

GPU_Info* GPU_Info::getObjPtr(HWND hWnd){
	map< HWND, GPU_Info* >::iterator it = GPU_Info::ginfohash.find(hWnd);
	if(it != GPU_Info::ginfohash.end()){
		return it->second;
	}
	return NULL;
};

void GPU_Info::regist(GPU_Info* pGinfo){
	// ボタンのウィンドウハンドル取得
	HWND hWnd = pGinfo->hWnd;
	// バイナリツリーに登録
	GPU_Info::ginfohash.insert(make_pair(hWnd, pGinfo));
};

int GPU_Info::RegisterWndClass(HINSTANCE hInst_){
	WNDCLASSEX wc = {0};

	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"GPU_Info Class";
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

void GPU_Info::Init_GUI(map<string, string> info){
	ClearData();
	util.SetColor(100, 100, 255);
	util.SetShowInfoFormat("%3.0f%%");
	util.SetminmaxFormat("%3.0f%%");
	util.SetMinMax(0, 100);
	util.SetName(L"util");

	mem.SetColor(100, 100, 255);
	mem.SetShowInfoFormat("%3.1f/%3.1f GiB");
	mem.SetminmaxFormat("%3.0f%%");
	mem.SetMinMax(0, stof(info["memory.total"]) / 1000.0);
	mem.SetName(L"memory");
	mem.show_fraction_flag = true;

	temp.SetColor(100, 100, 255);
	temp.SetShowInfoFormat("%3.1f℃");
	temp.SetminmaxFormat("%3.0f℃");
	temp.SetMinMax(0, 100);
	temp.SetName(L"temp");

	power.SetColor(100, 100, 255);
	power.SetShowInfoFormat("%3.0fW");
	power.SetminmaxFormat("%3.0fW");
	power.SetMinMax(0, stof(info["power.limit"]));
	power.SetName(L"power");
	power.mem_val_flag = true;

}

HWND GPU_Info::Create(int x, int y, int width, int height, HWND Parent_, HINSTANCE hInst_, map<string, string> info){
	hParent = Parent_;
	hInst = hInst_;
	
	if(!wndclass_regist){
		RegisterWndClass(hInst);
	}

	Init_GUI(info);

	if(hWnd != NULL){
		return NULL;
	}
	hWnd = CreateWindow(L"GPU_Info Class", L"Graph", WS_CHILD | WS_CLIPCHILDREN , x, y, width, height, hParent, NULL, hInst, this);

	if(hWnd != NULL){
		regist(this);
	}
	return hWnd;
}

void GPU_Info::ClearData(){
	util.ClearData();
	mem.ClearData();
	temp.ClearData();
	power.ClearData();
}


void GPU_Info::Update(map<string, string> info,vector<vector<string>> process_){
	if(info.size() > 10){
		id = stoi(info["index"]);
		uuid = info["uuid"];
		gpu_name = info["name"];
		val_mem_total = stof(info["memory.total"])/1000.0;
		val_mem_used = stof(info["memory.used"])/1000.0;
		val_gpu_util = stof(info["utilization.gpu"]);
		val_temp = stof(info["temperature.gpu"]);
		val_power = stof(info["power.draw"]);
		val_power_limit = stof(info["power.limit"]);

		InvalidateRect(hWnd, NULL, FALSE);
		util.update(val_gpu_util);
		mem.update(val_mem_used);
		temp.update(val_temp);
		power.SetMinMax(0, val_power_limit);
		power.update(val_power);
	}
	if(process_.size() >= 1){
		int count = 0;
		wchar_t wtext[256];
		char text[256];
		process.clear();
		for(auto itr = process_.begin(); itr != process_.end(); ++itr){
			if((*itr)[0].compare(uuid) == 0){
				vector<string> temp = {(*itr)[1],(*itr)[2],(*itr)[3]};
				process.emplace_back(temp);
				++count;
			}
		}
		Sort(sort_index, sort_direction);
		ListView_SetItemCountEx(hList, count, LVSICF_NOINVALIDATEALL);
	}
}

void GPU_Info::Sort(int index, int dir){
	if(dir){
		switch(index){
			case 0:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stoi(alpha[index]) > stoi(beta[index])); });
				break;
			case 1:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (alpha[index] > beta[index]); });
				break;
			case 2:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stof(alpha[index]) > stof(beta[index])); });
				break;
		}
	} else{
		switch(index){
			case 0:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stoi(alpha[index]) < stoi(beta[index])); });
				break;
			case 1:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (alpha[index] < beta[index]); });
				break;
			case 2:
				sort(process.begin(), process.end(), [=](const vector<string>& alpha, const vector<string>& beta){return (stof(alpha[index]) < stof(beta[index])); });
				break;
		}
	}
}

HWND GPU_Info::GethWnd(){
	return hWnd;
}

LRESULT CALLBACK GPU_Info::GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	static HFONT hFont_id,hFont_Name;
	static HMENU tmp_menu, popupmenu;
	static HPEN hPen;
	HFONT oldFont;
	HPEN oldPen;
	HBRUSH oldBrush;
	HDC hdc;
	PAINTSTRUCT ps;
	char text[256];
	wchar_t wtext[256];
	SIZE size;
	RECT rc;
	GPU_Info* gpu_info = getObjPtr(hWnd);
	LVCOLUMN col;
	LVITEM item = {0};
	int iCount = 0;

	if(gpu_info != NULL || msg == WM_CREATE){
		switch(msg){
			case WM_CREATE:
				GetClientRect(hWnd, &rc);
				gpu_info = (GPU_Info*)(((CREATESTRUCT*)lp)->lpCreateParams);

				gpu_info->util.Create(10, 50, 200, 120, hWnd, gpu_info->hInst);

				gpu_info->mem.Create(220, 50, 200, 120, hWnd, gpu_info->hInst);

				gpu_info->temp.Create(10, 170, 200, 120, hWnd, gpu_info->hInst);

				gpu_info->power.Create(220, 170, 200, 120, hWnd, gpu_info->hInst);
				if(hFont_Name == NULL){
					hFont_Name = CreateFont(20, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				}
				if(hFont_id == NULL){
					hFont_id = CreateFont(30, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"メイリオ");
				}
				if(hPen == NULL){
					hPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
				}
				gpu_info->hList = CreateWindowEx(0, WC_LISTVIEW, 0,	WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA,
					10, 300, 420, 200, hWnd, (HMENU)1, gpu_info->hInst, NULL);
				SendMessage(gpu_info->hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
				col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
				col.fmt = LVCFMT_LEFT;
				col.cx = 50;
				col.pszText = (LPWSTR)L"pid";
				ListView_InsertColumn(gpu_info->hList, 0, &col);
				col.pszText = (LPWSTR)L"process";
				col.cx = rc.right - 150;
				ListView_InsertColumn(gpu_info->hList, 1, &col);
				col.pszText = (LPWSTR)L"mem(MiB)";
				col.cx = 100;
				ListView_InsertColumn(gpu_info->hList, 2, &col);

				hdc = GetDC(hWnd);
				gpu_info->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				gpu_info->hdcMem = CreateCompatibleDC(hdc);		// カレントスクリーン互換
				SelectObject(gpu_info->hdcMem, gpu_info->hBitmap);		// MDCにビットマップを割り付け
				ReleaseDC(hWnd, hdc);

				tmp_menu = LoadMenu(gpu_info->hInst, MAKEINTRESOURCE(IDR_MENU2));
				popupmenu = GetSubMenu(tmp_menu, 0);
				break;

			case WM_SIZE:
				GetClientRect(hWnd, &rc);

				SetWindowPos(gpu_info->util.GethWnd(), NULL, 10, 50, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				SetWindowPos(gpu_info->mem.GethWnd(), NULL, 10 + (rc.right - 40) / 2 + 10, 50, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				SetWindowPos(gpu_info->temp.GethWnd(), NULL, 10, 170, (rc.right - 40) / 2, 120, SWP_NOZORDER);
				SetWindowPos(gpu_info->power.GethWnd(), NULL, 10 + (rc.right - 40) / 2 + 10, 170, (rc.right - 40) / 2, 120, SWP_NOZORDER);

				hdc = GetDC(hWnd);
				DeleteObject(gpu_info->hdcMem);
				DeleteObject(gpu_info->hBitmap);
				gpu_info->hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
				gpu_info->hdcMem = CreateCompatibleDC(hdc);		// カレントスクリーン互換
				SelectObject(gpu_info->hdcMem, gpu_info->hBitmap);		// MDCにビットマップを割り付け
				ReleaseDC(hWnd, hdc);

				SetWindowPos(gpu_info->hList, NULL, 10, 300, rc.right - 10, rc.bottom - 300 - 10, SWP_NOZORDER);
				{
					GetClientRect(gpu_info->hList, &rc);
					int width = 0;
					col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
					for(int i = 0; i <= 3; ++i){
						if(i == 1) continue;
						ListView_GetColumn(gpu_info->hList, 0, &col);
						width += col.cx;
					}
					width += 20;
					ListView_SetColumnWidth(gpu_info->hList, 1, rc.right - width);
				}
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			case WM_COMMAND:
				switch(LOWORD(wp)){
					case IDM_KILL:
					{
						int i;
						i = ListView_GetNextItem(gpu_info->hList, -1, LVIS_SELECTED);
						if(i >= 0){
							snprintf(text, 64, "pid:%s %s\nKill?", gpu_info->process[i][0].c_str(), gpu_info->process[i][1].c_str());
							MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
							if(MessageBox(hWnd, wtext, L"Task Kill", MB_YESNO) == IDYES){
								KillCmd(ssh, gpu_info->process[i][0]);
							}
						}
					}
					break;
				}
				return 0;

			case WM_NOTIFY:
				{
					LPNMHDR lpnmhdr;
					lpnmhdr = (LPNMHDR)lp;
					int col, row;
					if(lpnmhdr->hwndFrom == gpu_info->hList){   // BINTABLEからのメッセージであることをチェック
						switch(lpnmhdr->code){
							case LVN_GETDISPINFO:
							{
								LV_DISPINFO* pLvDispInfo;
								pLvDispInfo = (LV_DISPINFO*)lp;
								TCHAR szString[MAX_PATH] = L"";

								if(pLvDispInfo->item.mask & LVIF_TEXT){       // TEXTならば
									col = pLvDispInfo->item.iSubItem;       // 列番号
									row = pLvDispInfo->item.iItem;          // 行番号
									if(gpu_info->process.size() > row && gpu_info->process[row].size() > col){
										MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, gpu_info->process[row][col].c_str(), MAX_PATH, szString, MAX_PATH);
									}
									//wsprintf(szString, "%d,%d", col, row);     // とりあえず列行を出力
									lstrcpy(pLvDispInfo->item.pszText, szString);
								}
							}
							return TRUE;

							case LVN_COLUMNCLICK:
							{
								NMLISTVIEW* pListView = (NMLISTVIEW*)lp;
								if(gpu_info->sort_index == pListView->iSubItem){
									gpu_info->sort_direction = !gpu_info->sort_direction;
								} else{
									gpu_info->sort_direction = true;
								}
								gpu_info->sort_index = pListView->iSubItem;

								gpu_info->Sort(gpu_info->sort_index, gpu_info->sort_direction);
								InvalidateRect(gpu_info->hList, NULL, FALSE);
							}

							case NM_RCLICK:
							{
								LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lp;
								if(lpnmitem->iItem >= 0){
									POINT point;
									GetCursorPos(&point);
									TrackPopupMenu(popupmenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, point.x, point.y, 0, hWnd, NULL);
								}
								InvalidateRect(gpu_info->hList, NULL, FALSE);
							}
							return TRUE;
							
							default:
								break;
						}       // end of switch(lpnmhdr->code)
					}       // end of (hwndFrom == g.hbtable) 
				}
				break;

			case WM_ERASEBKGND:
				return 1;

			case WM_PAINT:
				hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rc);

				oldPen = (HPEN)SelectObject(gpu_info->hdcMem, GetStockObject(NULL_PEN));
				oldBrush = (HBRUSH)SelectObject(gpu_info->hdcMem, GetStockObject(WHITE_BRUSH));
				Rectangle(gpu_info->hdcMem, 0, 0, rc.right + 1, rc.bottom + 1);
				SelectObject(gpu_info->hdcMem, oldBrush);
				SelectObject(gpu_info->hdcMem, oldPen);

				oldPen = (HPEN)SelectObject(gpu_info->hdcMem, hPen);
				MoveToEx(gpu_info->hdcMem, 0, 0, NULL);
				LineTo(gpu_info->hdcMem, 0, rc.bottom);
				SelectObject(gpu_info->hdcMem, oldPen);

				oldFont = (HFONT)SelectObject(gpu_info->hdcMem, hFont_id);
				snprintf(text, 64, "GPU:%d", gpu_info->id);
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				GetTextExtentPoint32(gpu_info->hdcMem, wtext, lstrlen(wtext), &size);
				TextOut(gpu_info->hdcMem, 10, 10, wtext, lstrlen(wtext));

				SelectObject(gpu_info->hdcMem, hFont_Name);
				snprintf(text, 64, "%s", gpu_info->gpu_name.c_str());
				MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, text, 64, wtext, 64);
				GetTextExtentPoint32(gpu_info->hdcMem, wtext, lstrlen(wtext), &size);
				TextOut(gpu_info->hdcMem, rc.right - size.cx - 20, 10 + size.cy/2, wtext, lstrlen(wtext));
				SelectObject(gpu_info->hdcMem, oldFont);
				 
				BitBlt(hdc, 0, 0, rc.right, rc.bottom, gpu_info->hdcMem, 0, 0, SRCCOPY);
				EndPaint(hWnd, &ps);
				break;
		}
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

vector<map<string, string>> GetGPUInfo(SSH& ssh){
	char buffer[0x4000] = {0};
	vector<vector<string>> dummy;
	map<string, string> info;
	vector<map<string, string>> infos;
	string gpu_cmd = "nvidia-smi --format=csv,noheader,nounits --query-gpu=index,"
						"uuid,name,timestamp,memory.total,memory.used,utilization.gpu"
						",utilization.memory,temperature.gpu,power.draw,power.limit";

	if(ssh.connect_flag){
		ssh.ExecCmd((char*)gpu_cmd.c_str(), buffer);
		dummy = csv2vector(pack_space(buffer));
		for(int i = 0; i < dummy.size(); i++){
			info["index"] = dummy[i][0];
			info["uuid"] = replace(dummy[i][1]," ","");
			info["name"] = dummy[i][2];
			info["timestamp"] = dummy[i][3];
			info["memory.total"] = dummy[i][4];
			info["memory.used"] = dummy[i][5];
			info["utilization.gpu"] = dummy[i][6];
			info["utilization.memory"] = dummy[i][7];
			info["temperature.gpu"] = dummy[i][8];
			info["power.draw"] = dummy[i][9];
			info["power.limit"] = dummy[i][10];
			infos.emplace_back(info);
		}
		return infos;
	}
	
	return infos;
}

vector<vector<string>> GetGPUProcess(SSH& ssh){
	char buffer[0x4000] = {0};
	vector<vector<string>> dummy;
	string gpu_cmd = "nvidia-smi --format=csv,noheader,nounits --query-compute-apps=gpu_uuid,pid,process_name,used_memory";

	if(ssh.connect_flag){
		ssh.ExecCmd((char*)gpu_cmd.c_str(), buffer);
		string a, b = buffer;
		while(1){
			a = replace(b, "  ", " ");
			if(a == b) break;
			b = a;
		}
		dummy = csv2vector(a);
		return dummy;
	}
	
	return dummy;

}
