// Controls.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Controls.h"
#include "DarkMode.h"
#include "ListViewUtil.h"

using namespace DarkMode;
HWND listview = NULL;

#pragma region Comctl 6
#if defined(_M_IX86)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_X64)
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion

void UpdateThemeColors()
{
	HTHEME them = OpenThemeData(0, IsDarkMode ? L"DarkMode_DarkTheme::TaskDialog" : L"TaskDialog");
	GetThemeColor(them, 1, 0, TMT_FILLCOLOR, &clr);
	CloseThemeData(them);

	them = OpenThemeData(0, IsDarkMode ? L"DarkMode_DarkTheme::TaskDialog" : L"Explorer::TaskDialog");
	GetThemeColor(them, 1, 0, TMT_TEXTCOLOR, &clrTxt);
	CloseThemeData(them);

	them = OpenThemeData(0, IsDarkMode ? L"DarkMode_DarkTheme::Edit" : L"Edit");
	GetThemeColor(them, 3, 1, TMT_FILLCOLOR, &editBg);
	CloseThemeData(them);
}

HTREEITEM AddItemToTree(HWND hTreeView, HTREEITEM hParent, LPWSTR text) {
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_LAST;

	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = text;
	tvInsert.item.cchTextMax = lstrlen(text);

	return TreeView_InsertItem(hTreeView, &tvInsert);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitDarkMode();
	FixDarkScrollBar();

	UpdateThemeColors();

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CONTROLS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
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

	return (int)msg.wParam;
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
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CONTROLS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(clr);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CONTROLS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_CONTROLS));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) return FALSE;

	status = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L" Status Bar", hWnd, 159);
	UpdateTitleBar(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = 0;
	if (Menu::UAHDarkModeWndProc(hWnd, message, wParam, lParam, &lr))
	{
		return lr;
	}
	if (message == WM_CREATE)
	{
		stdBtn = CreateWindow(WC_BUTTON, L"Standard", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 10, 29, 72, 22, hWnd, (HMENU)300, hInst, 0);
		disblBtn = CreateWindow(WC_BUTTON, L"Disabled", WS_DISABLED | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 54, 72, 22, hWnd, (HMENU)301, hInst, 0);
		CreateWindow(WC_STATIC, L"Button: ", WS_VISIBLE | WS_CHILD, 12, 10, 72, 15, hWnd, (HMENU)302, hInst, 0);

		CreateWindow(WC_STATIC, L"Radio: ", WS_VISIBLE | WS_CHILD, 109, 10, 72, 15, hWnd, (HMENU)303, hInst, 0);
		CreateWindow(WC_BUTTON, L"Unset", BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 109, 29, 72, 22, hWnd, (HMENU)304, hInst, 0);
		CreateWindow(WC_BUTTON, L"Disabled", WS_DISABLED | BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE, 109, 50, 72, 22, hWnd, (HMENU)305, hInst, 0);
		CreateWindow(WC_BUTTON, L"Set", BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 109, 72, 72, 22, hWnd, (HMENU)306, hInst, 0);

		CreateWindow(WC_STATIC, L"Checkbox: ", WS_VISIBLE | WS_CHILD, 206, 10, 72, 15, hWnd, (HMENU)307, hInst, 0);
		CreateWindow(WC_BUTTON, L"Unchecked", BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 29, 128, 22, hWnd, (HMENU)308, hInst, 0);
		CreateWindow(WC_BUTTON, L"Unchecked Disabled", WS_DISABLED | BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 50, 128, 22, hWnd, (HMENU)309, hInst, 0);
		CreateWindow(WC_BUTTON, L"Checked", BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 72, 128, 22, hWnd, (HMENU)310, hInst, 0);
		CreateWindow(WC_BUTTON, L"Checked Disabled", WS_DISABLED | BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 94, 128, 22, hWnd, (HMENU)311, hInst, 0);
		CreateWindow(WC_BUTTON, L"Tri-state", BS_AUTO3STATE | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 116, 128, 22, hWnd, (HMENU)312, hInst, 0);
		CreateWindow(WC_BUTTON, L"Tri-state Disabled", WS_DISABLED | BS_AUTO3STATE | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 206, 138, 128, 22, hWnd, (HMENU)313, hInst, 0);

		CreateWindow(WC_STATIC, L"Edit: ", WS_VISIBLE | WS_CHILD, 348, 10, 72, 15, hWnd, (HMENU)314, hInst, 0);
		CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"normal", WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD , 348, 29, 102, 20, hWnd, (HMENU)315, hInst, 0);
		CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"disabled", WS_DISABLED | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 348, 52, 102, 20, hWnd, (HMENU)316, hInst, 0);
		CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"password", ES_PASSWORD | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 348, 75, 102, 20, hWnd, (HMENU)317, hInst, 0);
		CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"readonly", ES_READONLY | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 348, 98, 102, 20, hWnd, (HMENU)318, hInst, 0);

		CreateWindow(WC_STATIC, L"Combobox: ", WS_VISIBLE | WS_CHILD, 478, 10, 72, 15, hWnd, (HMENU)319, hInst, 0);
		HWND cmboBox = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP, 478, 29, 102, 20, hWnd, (HMENU)320, hInst, 0);
		HWND cmboBoxLis = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWNLIST | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 478, 56, 102, 20, hWnd, (HMENU)321, hInst, 0);
		HWND cmboBoxDis = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWNLIST | WS_DISABLED | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CHILD, 478, 83, 102, 20, hWnd, (HMENU)322, hInst, 0);

		CreateWindow(WC_STATIC, L"Tab: ", WS_VISIBLE | WS_CHILD, 598, 10, 100, 15, hWnd, (HMENU)343, hInst, 0);
		HWND tabCtrl = CreateWindow(WC_TABCONTROL, L"tab", WS_VISIBLE | WS_CHILD, 598, 29, 222, 150, hWnd, (HMENU)323, hInst, 0);
		CreateWindow(WC_BUTTON, L"Group Box", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 10, 94, 180, 90, hWnd, (HMENU)324, hInst, 0);
		
		CreateWindow(WC_STATIC, L"Spinner: ", WS_VISIBLE | WS_CHILD, 25, 115, 72, 15, hWnd, (HMENU)325, hInst, 0);
		HWND spinnerBuddy = CreateWindow(WC_EDIT, L"", ES_NUMBER | ES_LEFT | WS_GROUP | WS_VISIBLE | WS_CHILD | WS_BORDER, 25, 134, 170, 20, hWnd, (HMENU)326, hInst, 0);
		CreateUpDownControl(UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK | WS_GROUP | WS_CHILD | WS_VISIBLE | WS_BORDER, 160, 154, 60, 20, hWnd, 327, hInst, spinnerBuddy, 100, 0, 0);

		CreateWindow(WC_STATIC, L"Link: ", WS_VISIBLE | WS_CHILD, 348, 130, 72, 15, hWnd, (HMENU)328, hInst, 0);
		CreateWindow(WC_LINK, L"<a>example.com</a>", WS_VISIBLE | WS_CHILD, 348, 150, 72, 15, hWnd, (HMENU)329, hInst, 0);

		CreateWindow(WC_STATIC, L"Progressbar: ", WS_VISIBLE | WS_CHILD, 12, 200, 100, 15, hWnd, (HMENU)330, hInst, 0);
		HWND progress = CreateWindow(PROGRESS_CLASS, L"", WS_VISIBLE | WS_CHILD, 12, 220, 150, 25, hWnd, (HMENU)331, hInst, 0);
		HWND progressErr = CreateWindow(PROGRESS_CLASS, L"", WS_VISIBLE | WS_CHILD, 12, 250, 150, 25, hWnd, (HMENU)332, hInst, 0);
		HWND progressPaused = CreateWindow(PROGRESS_CLASS, L"", WS_VISIBLE | WS_CHILD, 12, 280, 150, 25, hWnd, (HMENU)333, hInst, 0);
		HWND progressmar = CreateWindow(PROGRESS_CLASS, L"", PBS_MARQUEE | WS_VISIBLE | WS_CHILD, 12, 310, 150, 25, hWnd, (HMENU)334, hInst, 0);
		
		CreateWindow(WC_STATIC, L"Trackbar: ", WS_VISIBLE | WS_CHILD, 12, 355, 100, 15, hWnd, (HMENU)335, hInst, 0);
		CreateWindow(TRACKBAR_CLASS, L"", WS_CHILD | WS_VISIBLE, 65, 350, 110, 25, hWnd, (HMENU)336, hInst, 0);

		CreateWindow(WC_STATIC, L"Listbox: ", WS_VISIBLE | WS_CHILD, 206, 200, 90, 15, hWnd, (HMENU)337, hInst, 0);
		HWND listbox = CreateWindow(WC_LISTBOX, L"", WS_BORDER | WS_CHILD | WS_VISIBLE, 206, 220, 120, 175, hWnd, (HMENU)338, hInst, 0);

		CreateWindow(WC_STATIC, L"Listview: ", WS_VISIBLE | WS_CHILD, 348, 200, 100, 15, hWnd, (HMENU)339, hInst, 0);
		listview = CreateWindow(WC_LISTVIEW, L"", LVS_REPORT | WS_BORDER | WS_CHILD | WS_VISIBLE, 348, 220, 180, 175, hWnd, (HMENU)340, hInst, 0);

		CreateWindow(WC_STATIC, L"Treeview: ", WS_VISIBLE | WS_CHILD, 598, 200, 100, 15, hWnd, (HMENU)341, hInst, 0);
		HWND treeview = CreateWindow(WC_TREEVIEW, L"", TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | WS_BORDER | WS_CHILD | WS_VISIBLE, 598, 220, 180, 175, hWnd, (HMENU)342, hInst, 0);

		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 50;
		lvc.iSubItem = 0;
		lvc.pszText = (LPWSTR)L"Col 1";
		ListView_InsertColumn(listview, 0, &lvc);

		lvc.cx =80;
		lvc.iSubItem = 1;
		lvc.pszText = (LPWSTR)L"Col 2";
		ListView_InsertColumn(listview, 1, &lvc);

		ListView_EnableGroupView(listview, TRUE);

		LVGROUP lg;
		lg.cbSize = sizeof(LVGROUP);
		lg.mask = LVGF_HEADER | LVGF_GROUPID;
		lg.pszHeader = (LPWSTR)L"Group 1";
		lg.iGroupId = 1;
		ListView_InsertGroup(listview, -1, &lg);

		lg.pszHeader = (LPWSTR)L"Group 2";
		lg.iGroupId = 2;
		ListView_InsertGroup(listview, -1, &lg);

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_TEXT | LVIF_GROUPID;
		lvi.iItem = 0;
		lvi.iSubItem = 0;
		lvi.pszText = (LPWSTR)L"Item 1";
		lvi.iGroupId = 1;
		int iRow = ListView_InsertItem(listview, &lvi);
		ListView_SetItemText(listview, iRow, 1, (LPWSTR)L"Item2");

		lvi.iGroupId = 2;
		iRow = ListView_InsertItem(listview, &lvi);
		ListView_SetItemText(listview, iRow, 1, (LPWSTR)L"Item2");

		InitListView(listview);

		HTREEITEM root1 = AddItemToTree(treeview, TVI_ROOT, (LPWSTR)L"Item 1");
		AddItemToTree(treeview, root1, (LPWSTR)L"Subitem 1");
		AddItemToTree(treeview, root1, (LPWSTR)L"Subitem 2");

		root1 = AddItemToTree(treeview, TVI_ROOT, (LPWSTR)L"Item 2");
		AddItemToTree(treeview, root1, (LPWSTR)L"Subitem 1");
		AddItemToTree(treeview, root1, (LPWSTR)L"Subitem 2");

		AddItemToTree(treeview, TVI_ROOT, (LPWSTR)L"Item 3");


		SendMessage(progress, PBM_SETPOS, (WPARAM)72, 0);
		SendMessage(progressErr, PBM_SETPOS, (WPARAM)50, 0);
		SendMessage(progressPaused, PBM_SETPOS, (WPARAM)28, 0);
		SendMessage(progressErr, PBM_SETSTATE, PBST_ERROR, 0);
		SendMessage(progressPaused, PBM_SETSTATE, PBST_PAUSED, 0);
		SendMessage(progressmar, PBM_SETMARQUEE, TRUE, 50);


		TCHAR achTemp[25] = L"Tab";
		TCITEM tie;
		tie.mask = TCIF_TEXT;
		tie.pszText = achTemp;
		for (int i = 0; i < 6; i++)
		{
			TabCtrl_InsertItem(tabCtrl, i, &tie);
		}

		TCHAR text[3][10] =
		{
			TEXT("Item 1"), TEXT("Item 2"), TEXT("Item 3")
		};

		TCHAR A[16];
		for (int k = 0; k <= 2; k++)
		{
			wcscpy_s(A, ARRAYSIZE(A), text[k]);
			ComboBox_AddString(cmboBox, A);
			ComboBox_AddString(cmboBoxDis, A);
			ComboBox_AddString(cmboBoxLis, A);
			ListBox_AddString(listbox, A);
		}

		ComboBox_SetCurSel(cmboBox, 0);
		ComboBox_SetCurSel(cmboBoxLis, 0);
		ComboBox_SetCurSel(cmboBoxDis, 0);

		CheckRadioButton(hWnd, 304, 306, 306);
		CheckDlgButton(hWnd, 310, BST_CHECKED);
		CheckDlgButton(hWnd, 311, BST_CHECKED);
		CheckDlgButton(hWnd, 312, BST_INDETERMINATE);
		CheckDlgButton(hWnd, 313, BST_INDETERMINATE);

		UpdateFont(hWnd, 300, 343);

		AllowDarkModeForWindowWithParentFallback(hWnd, TRUE);
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
			SetBkMode(hdc, TRANSPARENT);
		}
		return reinterpret_cast<INT_PTR>(CreateSolidBrush(clr));
	}
	else if (message == WM_CTLCOLORBTN) {
		if (IsDarkMode)
		{
			HDC hdc = reinterpret_cast<HDC>(wParam);
			SetBkMode(hdc, TRANSPARENT);
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
	else if (message == WM_ERASEBKGND)
	{
		HDC hdc = (HDC)wParam;
		RECT rc;
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, CreateSolidBrush(clr));
		return 1;
	}
	else if (message == WM_WINDOWPOSCHANGED)
	{
		if (IsDarkMode) Menu::drawUAHMenuNCBottomLine(hWnd);
		SendMessage(status, WM_SIZE, 0, 0);
	}
	else if (message == WM_THEMECHANGED)
	{
		IsDarkMode = ShouldAppsUseDarkMode();
		UpdateThemeColors();
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
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
		AllowDarkModeForWindowWithParentFallback(hDlg, TRUE);
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
