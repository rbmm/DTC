#include "stdafx.h"

#include "MiniWnd.h"

enum { CREATE_DANGEROUS_WINDOW = WM_USER + 0x1337, DESTROY_DANGEROUS_WINDOW };

static const PCWSTR DangerousClass = L"Dangerous Class";
static const PCWSTR DangerousWindow = L"Dangerous Window";

struct DisplayWnd : public YWnd {
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
		}
		return __super::WindowProc(hwnd, uMsg, wParam, lParam);
	}
};

struct ServiceWnd : public YWnd
{
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case CREATE_DANGEROUS_WINDOW:
			if (DisplayWnd* wnd = new DisplayWnd)
			{
				CREATESTRUCT *Baby = (CREATESTRUCT*)wParam;

				wnd->Create(Baby->dwExStyle,
					Baby->lpszClass,
					Baby->lpszName,
					Baby->style,
					Baby->x,
					Baby->y,
					Baby->cx,
					Baby->cy,
					Baby->hwndParent,
					Baby->hMenu);

				wnd->Release();
			}

			return 0;

		case DESTROY_DANGEROUS_WINDOW:
			DestroyWindow((HWND)wParam);
			return 0;

		case WM_NCDESTROY:
			OnDestroy();
			break;
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
};

static DWORD WINAPI MainThread(HWND ServiceWindow)
{
    CREATESTRUCTW Baby = {
		0, (HINSTANCE)&__ImageBase, 0,  0, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		WS_OVERLAPPEDWINDOW|WS_VISIBLE, 
		DangerousWindow, 
		DangerousClass 
	};

	ULONG MainThreadID = GetCurrentThreadId();

	SendMessageW(ServiceWindow, CREATE_DANGEROUS_WINDOW, (WPARAM)&Baby, MainThreadID);

    ULONG X = 0, WindowCount;
	do 
	{
		MSG Message;
		while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
			switch(Message.message)
			{
			case WM_CHAR:
				SendMessageW(ServiceWindow, CREATE_DANGEROUS_WINDOW, (WPARAM)&Baby, MainThreadID);
				break;

			case WM_CLOSE:
				SendMessageW(ServiceWindow, DESTROY_DANGEROUS_WINDOW, Message.wParam, MainThreadID);
				break;
			}
		}

		LONG MidPoint = (X++ & 0xFFFF) >> 6;

		WindowCount = 0;

		HWND Window = 0;
		while (Window = FindWindowExW(0, Window, DangerousClass, DangerousWindow))
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
			++WindowCount;
		}

	} while (WindowCount);

	return PostMessageW(ServiceWindow, WM_QUIT, 0, 0);
}

void WINAPI ep(void*)
{
	if (!YWnd::Register(DangerousClass))
	{
		ServiceWnd wnd;
		if (HWND ServiceWindow = wnd.Create(0, DangerousClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0))
		{
			ULONG MainThreadID;
			if (HANDLE hThread = CreateThread(0, 0, (PTHREAD_START_ROUTINE)MainThread, ServiceWindow, 0, &MainThreadID))
			{
				CloseHandle(hThread);

				MSG Message;
				while (0 < GetMessageW(&Message, 0, 0, 0))
				{
					TranslateMessage(&Message);
					switch (Message.message)
					{
					case WM_CHAR:
						PostThreadMessageW(MainThreadID, Message.message, Message.wParam, Message.lParam);
						break;
					default:
						DispatchMessageW(&Message);
						break;
					}
				}
			}

			DestroyWindow(ServiceWindow);
		}

		YWnd::Unregister(DangerousClass);
	}

	ExitProcess(0);
}
