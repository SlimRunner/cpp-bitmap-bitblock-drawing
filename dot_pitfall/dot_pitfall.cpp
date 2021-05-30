// dot_pitfall.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "dot_pitfall.h"

//loads windows graphic device interface
#include "Wingdi.h"
//used param macros and gdi object macros
#include "Windowsx.h"
//for sqrt
#include "cmath"

#define NDEBUG
#include <assert.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

template <typename T>
T maxv(T a, T b)
{
	return a >= b ? a : b;
}

template <typename T>
T minv(T a, T b)
{
	return a <= b ? a : b;
}

float lerp(float a, float b, float t)
{
	return (1 - t)*a + t * b;
}

float exerp(float start, float end, float tvalue)
{
	if (!tvalue) return end;
	return lerp(start, end, maxv<float>(minv<float>(log2(600/tvalue) / log2(100), 1), 0));
}

float getMouseDist(POINT mse, int x_in, int y_in)
{
	int x, y;
	x = mse.x - x_in;
	y = mse.y - y_in;
	return sqrtf(x * x + y * y);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	//this are just to avoid VS from complaining about unused variables
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DOTPITFALL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DOTPITFALL));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DOTPITFALL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = CreateSolidBrush(COLORREF(RGB(0, 0, 0))); //(HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DOTPITFALL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static POINT msePos;
	static RECT clSize;
	const int DOTGAP = 20;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;

			//get a device context (NOTE: must be released regardless if there is another DC of the same window handle)
			HDC srcDC = GetDC(hWnd);
			//create a compatible device context for the backbuffer
			HDC bbDC = CreateCompatibleDC(srcDC);
			//create a compatible bitmap to be assigned to the backbuffer DC
			HBITMAP bbBit = CreateCompatibleBitmap(srcDC, clSize.right, clSize.bottom);

			//select created bitmap in backbuffer DC and save old bitmap (NOTE: must replace old one before clean up)
			HBITMAP oldBitmap = SelectBitmap(bbDC, bbBit);

			int intensity, dist;

			for (int y = 0; y < clSize.bottom; y += DOTGAP)
			{
				for (int x = 0; x < clSize.right; x += DOTGAP)
				{
					dist = getMouseDist(msePos, x, y);
					intensity = minv<int>(maxv<int>(dist, 0), 255);
					//prints pixel to the backbuffer
					SetPixel(bbDC, exerp(x, msePos.x, dist), exerp(y, msePos.y, dist), RGB(intensity, intensity, intensity));
				}
			}

			//prepares window for painting
            HDC hdc = BeginPaint(hWnd, &ps);

			//dump backbuffer into current DC
			BitBlt(hdc, 0, 0, clSize.right, clSize.bottom, bbDC, 0, 0, SRCCOPY);
			
			//Cleanup (be sure to unwind correctly)
			//selects old object into backbuffer DC
			SelectObject(bbDC, oldBitmap);
			//deletes hanging bitmap that was previously assgined to backbuffer DC
			DeleteObject(bbBit);
			//deletes DC used as backbuffer
			DeleteDC(bbDC);
			//release DC pointer to current window
			ReleaseDC(hWnd, srcDC);

			//finishes window painting
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_MOUSEMOVE:
		//gets mouse location captured by the mousemove window event in lParam
		msePos.x = GET_X_LPARAM(lParam);
		msePos.y = GET_Y_LPARAM(lParam);

		/* NOTE
		 * RedrawWindow is equivalent to calling InvalidateRect followed by UpdateWindow
		 */

		//forces program to call WM_PAINT (refresh)
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
		//InvalidateRect(hWnd, NULL, FALSE);
		//UpdateWindow(hWnd);
		
		break;
	case WM_SIZE:
		//captures new client width and height of window
		GetClientRect(hWnd, &clSize);

		//this information is within lParam. Consider changing this

		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
