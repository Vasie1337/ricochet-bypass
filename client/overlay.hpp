#pragma once
#include <include.hpp>

class overlay
{
private:
	static inline HWND hwnd;
	static inline HINSTANCE hInstance;

public:
	static bool create(HINSTANCE hInst = GetModuleHandleA(0))
	{
		hInstance = hInst;

		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = WndProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = L"OverlayWindow";
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		if (!RegisterClassEx(&wc))
			return false;

		hwnd = CreateWindowExW(
			WS_EX_TOPMOST,
			wc.lpszClassName, L"Overlay",
			WS_POPUP,
			0, 0,
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			nullptr, nullptr, hInstance, nullptr);

		if (!hwnd)
			return false;

		SetWindowLong(hwnd, GWL_EXSTYLE, (int)GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);

		SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
		SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);

		return true;
	}

	static void render()
	{

	}

	static void destroy()
	{
		if (hwnd)
		{
			DestroyWindow(hwnd);
			hwnd = nullptr;
		}
	}

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);

			SetTextColor(hdc, RGB(0, 255, 0));
			SetBkMode(hdc, TRANSPARENT);

			RECT rect;
			GetClientRect(hwnd, &rect);

			uint64_t base = drv::base();
			short mz = drv::read<short>(base);
			std::string text = "MZ: " + std::to_string(mz);
			DrawTextA(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}
};