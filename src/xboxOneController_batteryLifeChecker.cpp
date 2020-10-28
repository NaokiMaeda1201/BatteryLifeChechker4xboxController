#include "framework.h"
#include "Windows.h"
#include "Xinput.h"
#include "xboxOneController_batteryLifeChecker.h"
#pragma	comment(lib,"Gdiplus.lib")
#include "ole2.h"
#include "gdiplus.h"
#pragma comment (lib, "xinput.lib")
using namespace Gdiplus;

#define MAX_LOADSTRING 100
#define CONTROLLER_IS_NOT_CONNECTED -1
#define INTERVAL 5000
#define USER_CONTROLLER 0
#define ORIGINAL_BACKGROUND_COLOR RGB(0, 0, 255)

#define EMPTY_PNG "./img/empty.png"
#define LOW_PNG "./img/low.png"
#define MEDIUM_PNG "./img/medium.png"
#define FULL_PNG "./img/full.png"

// grobal variables:
HINSTANCE hInst;
WCHAR szWindowClass[MAX_LOADSTRING];
INT batteryLevel;

// functions:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
VOID                SetBatteryLevel();
VOID                DrawImage(HDC hdc, PCTSTR strimage);
VOID                FillClientArea(HDC hdc, COLORREF color);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// initialize groval strings
	LoadStringW(hInstance, IDC_XBOXONECONTROLLERBATTERYLIFECHECKER, szWindowClass, MAX_LOADSTRING);

	// register window class
	MyRegisterClass(hInstance);

	// create main window
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// initialize battery level
	SetBatteryLevel();

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XBOXONECONTROLLERBATTERYLIFECHECKER));

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
// register window class
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(ORIGINAL_BACKGROUND_COLOR);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_XBOXONECONTROLLERBATTERYLIFECHECKER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = 0;

	return RegisterClassExW(&wcex);
}

//
// create main window
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST,
		szWindowClass,
		nullptr,
		WS_OVERLAPPED,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		80,
		80,
		NULL,
		NULL,
		hInst,
		NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// remove title bar
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
// controll messages 
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		if (LOWORD(wParam) == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
	}
	break;
	case WM_NCHITTEST:
	{
		LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
		if (hit == HTCLIENT) {
			hit = HTCAPTION;
		}
		return hit;
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// set battery image 
		PCTSTR strimage = TEXT(EMPTY_PNG);
		switch (batteryLevel) {
		case BATTERY_LEVEL_LOW:
			strimage = TEXT(LOW_PNG);
			break;
		case BATTERY_LEVEL_MEDIUM:
			strimage = TEXT(MEDIUM_PNG);
			break;
		case BATTERY_LEVEL_FULL:
			strimage = TEXT(FULL_PNG);
			break;
		}

		DrawImage(hdc, strimage);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_TIMER:
	{
		SetBatteryLevel();
		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;
	case WM_CREATE:
		SetLayeredWindowAttributes(hWnd, ORIGINAL_BACKGROUND_COLOR, 0, LWA_COLORKEY);
		SetTimer(hWnd, 1, INTERVAL, NULL);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//
// set battery level
//
VOID SetBatteryLevel() {

	XINPUT_BATTERY_INFORMATION batterystate;
	ZeroMemory(&batterystate, sizeof(XINPUT_BATTERY_INFORMATION));
	DWORD result = XInputGetBatteryInformation(USER_CONTROLLER, BATTERY_DEVTYPE_GAMEPAD, &batterystate);

	batteryLevel = CONTROLLER_IS_NOT_CONNECTED;
	if (result == ERROR_SUCCESS)
	{
		// controller is connected
		batteryLevel = batterystate.BatteryLevel;
	}
}

//
// draw PNG Image 
//
VOID DrawImage(HDC hdc, PCTSTR strimage) {
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	Graphics graphics(hdc);
	Image batteryImage(strimage);
	graphics.DrawImage(&batteryImage, 0, 0);
}
