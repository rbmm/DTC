#include "stdafx.h"

#include "MiniWnd.h"

enum { CREATE_DANGEROUS_WINDOW = WM_USER + 0x1337, DESTROY_DANGEROUS_WINDOW };

static const PCWSTR DangerousClass = L"Dangerous Class";
static const PCWSTR DangerousWindow = L"Dangerous Window";

struct DisplayWnd : public YWnd 
{
	static HWND CreateNewWindow()
	{
		if (DisplayWnd* wnd = new DisplayWnd)
		{
			HWND hwnd = wnd->Create(0, DangerousClass, DangerousWindow, WS_OVERLAPPEDWINDOW|WS_VISIBLE, 
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0);
			wnd->Release();

			return hwnd;
		}

		return 0;
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
			CreateNewWindow();
			break;
		}
		return __super::WindowProc(hwnd, uMsg, wParam, lParam);
	}
};

static DWORD WINAPI WorkThread(PVOID dwThreadId)
{
    ULONG WindowCount;
	LONG MidPoint = 0;
	do 
	{
		++MidPoint &= 0x3ff;

		WindowCount = 0;

		for (HWND Window = 0; Window = FindWindowExW(0, Window, DangerousClass, DangerousWindow); ++WindowCount)
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

			Sleep(1);
		}

	} while (WindowCount);

	return PostThreadMessageW((ULONG)(ULONG_PTR)dwThreadId, WM_QUIT, 0, 0);
}

void WINAPI ep(void*)
{
	if (!YWnd::Register(DangerousClass))
	{
		if (HWND hwnd = DisplayWnd::CreateNewWindow())
		{
			if (HANDLE hThread = CreateThread(0, 0, WorkThread, (PVOID)(ULONG_PTR)GetCurrentThreadId(), 0, 0))
			{
				CloseHandle(hThread);

				MSG Message;
				while (0 < GetMessageW(&Message, 0, 0, 0))
				{
					TranslateMessage(&Message);
					DispatchMessageW(&Message);
				}
			}
			else
			{
				DestroyWindow(hwnd);
			}
		}

		YWnd::Unregister(DangerousClass);
	}

	ExitProcess(0);
}