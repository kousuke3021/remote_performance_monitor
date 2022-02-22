/*!
CustomButton.h

Copyright (c) 2022 Kosuke Nakai

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#pragma once
#include<string>
#include<vector>
#include<windows.h>
#include"graph.h"

#define WM_BTN_SELECTED (WM_APP+2)
class GraphButton{
	public:
		HWND Create(int x, int y, int width, int height,int id,HWND hParent_, HINSTANCE hInst_, float val);
		HWND GethWnd();
		void Update(float val);
		void SetColor(int R, int G, int B);
		void ShowFormat(const char* format_);
		void SetTitle(const wchar_t* name_);

	private:
		Graph show;
		HWND hWnd = NULL, hParent = NULL;
		HINSTANCE hInst = NULL;
		WNDPROC DefaultButtonProc = NULL;
		HBITMAP hBitmap1 = NULL,hBitmap2 = NULL;
		HDC hdcMem1 = NULL,hdcMem2 = NULL;
		std::wstring title = L"CPU";
		std::string format = "%3.1f%%";
		float current_val = 0;
		bool mouse_over = false;
		bool mouse_l_push = false;
		bool focus = false;
		
		static GraphButton* selected;
		static std::map< HWND, GraphButton*> gbuttonhash;
		
	protected:
		static GraphButton* getObjPtr(HWND hWnd);
		static void regist(GraphButton* pGinfo);
		static LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

};
