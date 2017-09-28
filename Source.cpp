﻿#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>

void GetWindowsVersion(HWND hEdit)
{
	DWORD dwVersion = GetVersion();
	DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
	DWORD dwBuild = 0;

	if (dwVersion < 0x80000000)
		dwBuild = (DWORD)(HIWORD(dwVersion));

	// RtlGetVersion 関数が使える場合は使う
	const HMODULE hModule = GetModuleHandle(TEXT("ntdll.dll"));
	if (hModule)
	{
		typedef void (WINAPI*fnRtlGetVersion)(OSVERSIONINFOEXW*);
		fnRtlGetVersion RtlGetVersion = (fnRtlGetVersion)GetProcAddress(hModule, "RtlGetVersion");
		if (RtlGetVersion)
		{
			OSVERSIONINFOEXW osw = { sizeof(OSVERSIONINFOEXW) };
			RtlGetVersion(&osw);
			dwMajorVersion = osw.dwMajorVersion;
			dwMinorVersion = osw.dwMinorVersion;
			dwBuild = osw.dwBuildNumber;
		}
	}

	// レジストリに UBR の値が存在する場合は取得する
	DWORD dwUBR = 0;
	BOOL bExist = FALSE;
	{
		HKEY hKey;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ, &hKey))
		{
			DWORD dwType = REG_DWORD;
			DWORD dwByte = sizeof(DWORD);
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("UBR"), 0, &dwType, (LPBYTE)&dwUBR, &dwByte))
			{
				bExist = TRUE;
			}
			RegCloseKey(hKey);
		}
	}

	TCHAR szText[1024];
	if (bExist)
	{
		wsprintf(szText, TEXT("Version is %d.%d (Build %d.%d)\r\n"), dwMajorVersion, dwMinorVersion, dwBuild, dwUBR);
	}
	else
	{
		wsprintf(szText, TEXT("Version is %d.%d (Build %d)\r\n"), dwMajorVersion, dwMinorVersion, dwBuild);
	}

	SetWindowText(hEdit, szText);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit;
	switch (msg)
	{
	case WM_CREATE:
		hEdit = CreateWindow(TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD |
			ES_READONLY | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
			0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		GetWindowsVersion(hEdit);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 20, TRUE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	TCHAR szClassName[] = TEXT("Window");
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Windows の バージョンを取得"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
