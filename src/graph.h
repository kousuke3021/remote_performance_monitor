#pragma once
#include<windows.h>
#include<deque>
#include <utility>
#include<stdio.h>
#include<string>
#include<map>
#include <gdiplus.h>

class Graph{
	public:
		Graph(size_t size_=100);
		HWND Create(int x, int y, int width, int height, HWND hParent_, HINSTANCE hInst_);
		HWND GethWnd();
		HDC Paint(HWND hWnd, Graph* graph);
		int RegisterWndClass(HINSTANCE hInst);
		void update(float value);
		void val2pos(float val, int time, int& x, int& y,RECT rc);
		void SetShowInfoFormat(const char* format_);
		void SetminmaxFormat(const char* format_);
		void SetName(const wchar_t* name_);
		void SetMinMax(float min_, float max_);
		Gdiplus::Color SetColor(int R_, int G_, int B_);
		void ClearData();

	private:
		void regist(Graph* pGrh);


	public:
		bool show_fraction_flag = false;
		bool mem_val_flag = false;
		bool grid_flag = true;
		bool only_graph = false;
		
	private:
		HWND hWnd, hParent;
		HINSTANCE hInst;
		std::deque<float> data;
		HBITMAP hBitmap;
		HDC hdcMem;
		HFONT hFont_data, hFont_scale, hFont_title;
		std::string format_current="";
		std::wstring name;
		std::string format_scale;
		float min = 0, max = 100;
		int size;
		int scale_cx;
		int BaseColor_R;
		int BaseColor_G;
		int BaseColor_B;

		static bool wndclass_regist;
		static std::map< HWND, Graph*> grhhash;

	protected:
		static LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
		static Graph* getObjPtr(HWND hWnd);

};

