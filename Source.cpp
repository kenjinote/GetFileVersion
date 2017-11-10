#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "version")
#pragma comment(lib, "shlwapi")

#include <windows.h>
#include <shlwapi.h>

TCHAR szClassName[] = TEXT("Window");
WNDPROC EditWndProc;

struct VS_VERSIONINFO
{
	WORD                wLength;
	WORD                wValueLength;
	WORD                wType;
	WCHAR               szKey[1];
	WORD                wPadding1[1];
	VS_FIXEDFILEINFO    Value;
	WORD                wPadding2[1];
	WORD                wChildren[1];
};

BOOL GetFileVersion(LPCTSTR lpszFilePath, HWND hEdit)
{
	SetWindowText(hEdit, 0);
	VS_VERSIONINFO      *pVerInfo;
	LPBYTE              pOffsetBytes;
	VS_FIXEDFILEINFO    *pFixedInfo;
	DWORD               dwHandle;
	BOOL bResult = FALSE;
	const DWORD dwSize = GetFileVersionInfoSize(lpszFilePath, &dwHandle);
	if (0 < dwSize)
	{
		LPBYTE lpBuffer = new BYTE[dwSize];
		if (GetFileVersionInfo(lpszFilePath, 0, dwSize, lpBuffer) != FALSE)
		{
#define roundoffs(a,b,r) (((BYTE *) (b) - (BYTE *) (a) + ((r) - 1)) & ~((r) - 1))
#define roundpos(a,b,r) (((BYTE *) (a)) + roundoffs(a,b,r))
			pVerInfo = (VS_VERSIONINFO *)lpBuffer;
			pOffsetBytes = (BYTE *)&pVerInfo->szKey[lstrlenW(pVerInfo->szKey) + 1];
			pFixedInfo = (VS_FIXEDFILEINFO *)roundpos(pVerInfo, pOffsetBytes, 4);
			TCHAR szText[1024];
			wsprintf(szText, TEXT("File Version : %d.%d.%d.%d\r\n"), (pFixedInfo->dwFileVersionMS>>16) & 0xFFFF, pFixedInfo->dwFileVersionMS & 0xFFFF, (pFixedInfo->dwFileVersionLS >> 16) & 0xFFFF, pFixedInfo->dwFileVersionLS & 0x0000FFFF);
			SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szText);
			wsprintf(szText, TEXT("Product Version : %d.%d.%d.%d\r\n"), (pFixedInfo->dwProductVersionMS >> 16) & 0xFFFF, pFixedInfo->dwProductVersionMS & 0x0000FFFF, (pFixedInfo->dwProductVersionLS >> 16) & 0xFFFF, pFixedInfo->dwProductVersionLS & 0x0000FFFF);
			SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)szText);
			bResult = TRUE;
		}
		delete[] lpBuffer;
	}
	if (bResult == FALSE)
	{
		SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)TEXT("取得できませんでした。"));
	}
	return bResult;
}

LRESULT CALLBACK EditProc1(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CHAR:
		if (wParam == 0x0D)
		{
			HWND hParent = GetParent(hWnd);
			PostMessage(hParent, WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;
	}
	return CallWindowProc(EditWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1;
	static HWND hButton;
	static HWND hEdit2;
	switch (msg)
	{
	case WM_CREATE:
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("kernel32.dll"), WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		EditWndProc = (WNDPROC)SetWindowLongPtr(hEdit1, GWLP_WNDPROC, (LONG_PTR)EditProc1);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("取得"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOHSCROLL | ES_READONLY, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SETFOCUS:
		SendMessage(hEdit1, EM_SETSEL, 0, -1);
		SetFocus(hEdit1);
		break;
	case WM_SIZE:
		MoveWindow(hEdit1, 10, 10, LOWORD(lParam) - 128 - 20, 32, TRUE);
		MoveWindow(hButton, LOWORD(lParam) - 128 - 10, 10, 128, 32, TRUE);
		MoveWindow(hEdit2, 10, 50, LOWORD(lParam) - 20, HIWORD(lParam) - 60, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			TCHAR szFilePath[MAX_PATH];
			GetWindowText(hEdit1, szFilePath, _countof(szFilePath));
			PathUnquoteSpaces(szFilePath);
			GetFileVersion(szFilePath, hEdit2);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
		}
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
		TEXT("入力されたファイルのバージョンを取得"),
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
