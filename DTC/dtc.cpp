#include "stdafx.h"

#include "MiniWnd.h"

static const PCWSTR DangerousClass = L"Dangerous Class";
static const PCWSTR DangerousWindow = L"Dangerous Window";

struct DisplayWnd : public YWnd 
{
	PLONG m_pCount;
	int m_MidPoint = 0;

	static HWND CreateNewWindow(PLONG pCount)
	{
		if (DisplayWnd* wnd = new DisplayWnd(pCount))
		{
			HWND hwnd = wnd->Create(0, DangerousClass, DangerousWindow, WS_OVERLAPPEDWINDOW|WS_VISIBLE, 
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0);
			wnd->Release();

			return hwnd;
		}

		return 0;
	}

	void DoDraw(HWND Window, LONG MidPoint)
	{
		RECT Client;
		if (GetClientRect(Window, &Client))
		{
			if (HDC DC = GetDC(Window))
			{
				PatBlt(DC, 0, 0, MidPoint, Client.bottom, BLACKNESS);
				if (Client.right > MidPoint)
				{
					PatBlt(DC, MidPoint, 0, Client.right - MidPoint, Client.bottom, WHITENESS);
				}
				ReleaseDC(Window, DC);
			}
		}
	}

	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_PAINT:
			PAINTSTRUCT ps;
			if (BeginPaint(hwnd, &ps))
			{
				EndPaint(hwnd, &ps);
			}
			break;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_CHAR:
			CreateNewWindow(m_pCount);
			break;

		case WM_TIMER:
			DoDraw(hwnd, ++m_MidPoint & 0x3ff);
			break;

		case WM_CREATE:
			SetTimer(hwnd, 0, 1, 0);
			break;
		}
		return __super::WindowProc(hwnd, uMsg, wParam, lParam);
	}

	static void DecWindowsCount(PLONG pCount)
	{
		if (!--*pCount)
		{
			PostQuitMessage(0);
		}
	}

	DisplayWnd(PLONG pCount) : m_pCount(pCount)
	{
		++*pCount;
	}

	~DisplayWnd()
	{
		DecWindowsCount(m_pCount);
	}
};

void WINAPI ep(void*)
{
	if (!YWnd::Register(DangerousClass))
	{
		LONG Count = 1;
		if (HWND hwnd = DisplayWnd::CreateNewWindow(&Count))
		{
			DisplayWnd::DecWindowsCount(&Count);
			MSG Message;
			while (0 < GetMessageW(&Message, 0, 0, 0))
			{
				TranslateMessage(&Message);
				DispatchMessageW(&Message);
			}
		}

		YWnd::Unregister(DangerousClass);
	}

	ExitProcess(0);
}
