// Controls.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Controls.h"
#include "DarkMode.h"
using namespace DarkMode;

#pragma region Comctl 6
#if defined(_M_IX86)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_X64)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    InitDarkMode();
    FixDarkScrollBar();

    HTHEME them = OpenThemeData(0, IsDarkMode ? L"DarkMode_Explorer::TaskDialog" : L"Explorer::TaskDialog");
    GetThemeColor(them, TDLG_PRIMARYPANEL, 0, TMT_FILLCOLOR, &clr);
    GetThemeColor(them, TDLG_PRIMARYPANEL, 0, TMT_TEXTCOLOR, &clrTxt);
    them = OpenThemeData(0, IsDarkMode ? L"DarkMode_Explorer::Button" : L"Explorer::Button");
    GetThemeColor(them, BP_RADIOBUTTON, CBS_UNCHECKEDDISABLED, TMT_TEXTCOLOR, &clrTxtDis);
    them = OpenThemeData(0, IsDarkMode ? L"DarkMode_CFD::Edit" : L"CFD::Edit");
    GetThemeColor(them, 3, 1, TMT_FILLCOLOR, &editBg);
    CloseThemeData(them);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CONTROLS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CONTROLS));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        /*
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
        */
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        //}
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
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CONTROLS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(clr);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CONTROLS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_CONTROLS));

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
   AllowDarkModeForWindow(hWnd, IsDarkMode);
   status = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L" Status Bar", hWnd, 159);
   if (IsDarkMode)
   {
       SubclassStatusBar(status);
   }
   UpdateTitleBar(hWnd);
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
    LRESULT lr = 0;
    if (IsDarkMode && Menu::UAHDarkModeWndProc(hWnd, message, wParam, lParam, &lr))
    {
        return lr;
    }
    if (message == WM_CREATE)
    {
        stdBtn = CreateWindow(WC_BUTTON, L"Standard", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 10, 29, 72, 22, hWnd, (HMENU)300, hInst, 0);
        disblBtn = CreateWindow(WC_BUTTON, L"Disabled", WS_DISABLED | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 54, 72, 22, hWnd, (HMENU)301, hInst, 0);
        HWND btnStatic = CreateWindow(WC_STATIC, L"Button: ", WS_VISIBLE | WS_CHILD, 12, 10, 72, 15, hWnd, (HMENU)302, hInst, 0);

        HWND radStatic = CreateWindow(WC_STATIC, L"Radio: ", WS_VISIBLE | WS_CHILD, 109, 10, 72, 15, hWnd, (HMENU)303, hInst, 0);
        HWND radBtn = CreateWindow(WC_BUTTON, L"Unset", BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 109, 29, 72, 22, hWnd, (HMENU)304, hInst, 0);
        HWND radDisBtn = CreateWindow(WC_BUTTON, L"Disabled", WS_DISABLED | BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE, 109, 50, 72, 22, hWnd, (HMENU)305, hInst, 0);
        HWND radDefBtn = CreateWindow(WC_BUTTON, L"Set", BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 109, 72, 72, 22, hWnd, (HMENU)306, hInst, 0);

        HWND checkStatic = CreateWindow(WC_STATIC, L"Checkbox: ", WS_VISIBLE | WS_CHILD, 206, 10, 72, 15, hWnd, (HMENU)307, hInst, 0);
        HWND uncheckBtn = CreateWindow(WC_BUTTON, L"Unchecked", BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 29, 128, 22, hWnd, (HMENU)308, hInst, 0);
        HWND uncheckDisBtn = CreateWindow(WC_BUTTON, L"Unchecked Disabled", WS_DISABLED | BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 50, 128, 22, hWnd, (HMENU)309, hInst, 0);
        HWND checkBtn = CreateWindow(WC_BUTTON, L"Checked", BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 72, 128, 22, hWnd, (HMENU)310, hInst, 0);
        HWND checkDisBtn = CreateWindow(WC_BUTTON, L"Checked Disabled", WS_DISABLED | BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 94, 128, 22, hWnd, (HMENU)311, hInst, 0);
        HWND triStateBtn = CreateWindow(WC_BUTTON, L"Tri-state", BS_AUTO3STATE | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 116, 128, 22, hWnd, (HMENU)312, hInst, 0);
        HWND triStateDisBtn = CreateWindow(WC_BUTTON, L"Tri-state Disabled", WS_DISABLED | BS_AUTO3STATE | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 138, 128, 22, hWnd, (HMENU)313, hInst, 0);

        HWND editStatic = CreateWindow(WC_STATIC, L"Edit: ", WS_VISIBLE | WS_CHILD, 348, 10, 72, 15, hWnd, (HMENU)314, hInst, 0);
        HWND editCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"normal", WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 348, 29, 102, 20, hWnd, (HMENU)315, hInst, 0);
        HWND editDisCtrl = CreateWindowEx(IsDarkMode ? 0 : WS_EX_CLIENTEDGE, WC_EDIT, L"disabled", IsDarkMode ? WS_DISABLED | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD | WS_BORDER
            : WS_DISABLED | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 348, 52, 102, 20, hWnd, (HMENU)316, hInst, 0);
        HWND editPasCtrl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"password", ES_PASSWORD | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 348, 75, 102, 20, hWnd, (HMENU)317, hInst, 0);

        HWND cmboBoxStatic = CreateWindow(WC_STATIC, L"Combobox: ", WS_VISIBLE | WS_CHILD, 478, 10, 72, 15, hWnd, (HMENU)318, hInst, 0);
        HWND cmboBox = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWN | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 478, 29, 102, 20, hWnd, (HMENU)319, hInst, 0);
        HWND cmboBoxLis = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWNLIST | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 478, 56, 102, 20, hWnd, (HMENU)320, hInst, 0);
        HWND cmboBoxDis = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWNLIST | WS_DISABLED | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 478, 83, 102, 20, hWnd, (HMENU)321, hInst, 0);

        HWND tabCtrl = CreateWindow(WC_TABCONTROL, L"tab",  WS_VISIBLE | WS_CHILD, 598, 29, 222, 150, hWnd, (HMENU)322, hInst, 0);
        if (IsDarkMode) PhInitializeThemeWindowTabControl(tabCtrl);
        TCITEM tie;
        int i;
        TCHAR achTemp[256];  // Temporary buffer for strings.
        tie.mask = TCIF_TEXT | TCIF_IMAGE;
        tie.iImage = -1;
        tie.pszText = achTemp;
        LoadStringW(hInst, IDC_CONTROLS, achTemp, MAX_LOADSTRING);
        for (i = 0; i < 2; i++)
        {
            if (TabCtrl_InsertItem(tabCtrl, i, &tie) == -1)
            {
                DestroyWindow(tabCtrl);
                return NULL;
            }
        }
        TCHAR text[3][10] =
        {
            TEXT("Item 1"), TEXT("Item 2"), TEXT("Item 3")
        };

        TCHAR A[16];
        memset(&A, 0, sizeof(A));
        for (int k = 0; k <= 2; k++)
        {
            wcscpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)text[k]);
            SendMessage(cmboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
            SendMessage(cmboBoxLis, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
            SendMessage(cmboBoxDis, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
        }
        SendMessage(cmboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        SendMessage(cmboBoxLis, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        SendMessage(cmboBoxDis, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        CheckRadioButton(hWnd, 304, 306, 306);
        CheckDlgButton(hWnd, 310, BST_CHECKED);
        CheckDlgButton(hWnd, 311, BST_CHECKED);
        CheckDlgButton(hWnd, 312, BST_INDETERMINATE);
        CheckDlgButton(hWnd, 313, BST_INDETERMINATE);

        InitRadioRange(hWnd, 304, 306);
        InitCheckBoxRange(hWnd, 308, 313);

        EnableDarkMode(stdBtn, L"Explorer");
        EnableDarkMode(disblBtn, L"Explorer");
        EnableDarkMode(editCtrl, L"CFD");
        EnableDarkMode(editDisCtrl, L"CFD");
        EnableDarkMode(editPasCtrl, L"CFD");
        EnableDarkMode(cmboBox, L"CFD");
        EnableDarkMode(cmboBoxLis, L"CFD");
        EnableDarkMode(cmboBoxDis, L"CFD");
        
        UpdateFont(hWnd, 300, 322);
    }
    else if (message == WM_SIZE || message == WM_ACTIVATE)
    {
        if (IsDarkMode)
        {
            LRESULT lr = DefWindowProc(hWnd, message, wParam, lParam);
            Menu::drawUAHMenuNCBottomLine(hWnd);
        }
    }
    else if (message == WM_CTLCOLORSTATIC) {
        if (IsDarkMode)
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetBkColor(hdc, clr);
            SetTextColor(hdc, clrTxt);
        }
        return reinterpret_cast<INT_PTR>(CreateSolidBrush(clr));
    }
    else if (message == WM_CTLCOLORBTN) {
        if (IsDarkMode)
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetBkColor(hdc, clr);
            SetTextColor(hdc, clrTxt);
        }
        return reinterpret_cast<INT_PTR>(CreateSolidBrush(clr));
    }
    else if (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORLISTBOX) {
        if (IsDarkMode)
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetBkColor(hdc, editBg);
            SetTextColor(hdc, clrTxt);
        }
        return reinterpret_cast<INT_PTR>(CreateSolidBrush(editBg));
    }
    else if (message == WM_COMMAND)
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        if (wmId == IDM_ABOUT)
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        }
        else if (wmId == IDM_EXIT)
        {
            DestroyWindow(hWnd);
        }
        else 
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    else if (message == WM_WINDOWPOSCHANGED)
    {
        if (IsDarkMode) Menu::drawUAHMenuNCBottomLine(hWnd);
        SendMessage(status, WM_SIZE, 0, 0);
    }
    /*
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    */
    else if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
    }
    else
    {
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
