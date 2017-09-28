#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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

	// レジストリに ReleaseID や UBR の値が存在する場合は取得する
	TCHAR szReleaseID[32] = { 0 };
	TCHAR szUBR[32] = { 0 };
	{
		HKEY hKey;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey))
		{
			DWORD dwType = REG_SZ;
			DWORD dwByte = sizeof(szReleaseID);
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("ReleaseId"), 0, &dwType, (LPBYTE)&szReleaseID, &dwByte))
			{
				lstrcat(szReleaseID, TEXT(" "));
			}
			dwType = REG_DWORD;
			dwByte = sizeof(DWORD);
			DWORD dwUBR = 0;
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, TEXT("UBR"), 0, &dwType, (LPBYTE)&dwUBR, &dwByte))
			{
				wsprintf(szUBR, TEXT(".%d"), dwUBR);
			}
			RegCloseKey(hKey);
		}
	}

	TCHAR szText[1024];
	wsprintf(szText, TEXT("Windows %d.%d %s(OS ビルド %d%s)"), dwMajorVersion, dwMinorVersion, szReleaseID, dwBuild, szUBR);
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
	case WM_SETFOCUS:
		SetFocus(hEdit);
		SendMessage(hEdit, EM_SETSEL, 0, -1);
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
