#pragma once
#include"ssh.h"
#include<windows.h>
#include<string>
#include<vector>
#include"graph.h"


class General_Info{
	public:
		General_Info();
		HWND Create(int x, int y, int width, int height, HWND hParent_, HINSTANCE hInst_, std::map<std::string, std::string> info);
		HWND GethWnd();
		int RegisterWndClass(HINSTANCE hInst_);
		void Update(std::map<std::string, std::string> info, std::vector<std::vector<std::string>> process_);
		void Sort(int index, int dir);
		void Init_GUI(std::map<std::string, std::string> info);
		void ClearData();

	private:
		Graph util;
		Graph temp;
		Graph freq;
		Graph mem;
		HWND hWnd = NULL, hParent = NULL, hList = NULL,hUptime = NULL;
		HINSTANCE hInst = NULL;
		HBITMAP hBitmap = NULL;
		HDC hdcMem;
		std::vector<std::vector<std::string>> process;
		std::string cpu_name,uptime;
		float val_mem_used = 0;
		float val_mem_total = 0;
		float val_cpu_util = 0;
		float val_temp = 0;
		float val_freq = 0;
		int sort_index = 0, sort_direction = 0;
		bool push = false;

		static std::map< HWND, General_Info*> ginfohash;
		static bool wndclass_regist;

	protected:
		static General_Info* getObjPtr(HWND hWnd);
		static void regist(General_Info* pGinfo);
		static LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};

std::map<std::string, std::string> GetGeneralInfo(SSH& ssh);
std::vector<std::vector<std::string>> GetGeneralProcess(SSH& ssh);