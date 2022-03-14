/*!
gpu_info.h

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#pragma once
#include"ssh.h"
#include<string>
#include<vector>
#include <sstream>
#include<windows.h>
#include"graph.h"
#include"util.h"

class GPU_Info{
	public:
		GPU_Info();
		HWND Create(int x, int y, int width, int height, HWND Parent_, HINSTANCE hInst_, std::map<std::string, std::string> info);
		HWND GethWnd();
		int RegisterWndClass(HINSTANCE hInst_);
		void Update(std::map<std::string, std::string> info,std::vector<std::vector<std::string>> process_);
		void Sort(int index, int dir);
		void Init_GUI(std::map<std::string, std::string> info);
		void ClearData();

	private:
		Graph util;
		Graph temp;
		Graph power;
		Graph mem;
		HWND hWnd = NULL, hParent = NULL, hList;
		HINSTANCE hInst;
		HBITMAP hBitmap = NULL;
		HDC hdcMem;
		std::vector<std::vector<std::string>> process;
		std::string uuid;
		std::string gpu_name;
		float val_mem_used;
		float val_mem_total;
		float val_gpu_util;
		float val_temp;
		float val_power;
		float val_power_limit;
		int sort_index = 0, sort_direction = 0;
		int id;

		static std::map< HWND, GPU_Info*> ginfohash;
		static bool wndclass_regist;

	protected:
		static GPU_Info* getObjPtr(HWND hWnd);
		static void regist(GPU_Info* pGinfo);
		static LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

};

std::vector<std::map<std::string, std::string>> GetGPUInfo(SSH& ssh);
std::vector<std::vector<std::string>> GetGPUProcess(SSH& ssh);
