#pragma once
#include "Controls.h"
#include "DarkMode.IAT.h"

#pragma region Variables
SetPreferredAppMode_t SetPreferredAppMode;
OpenNcThemeData_t OpenNcThemeData;
ShouldAppsUseDarkMode_t ShouldAppsUseDarkMode;
AllowDarkModeForWindowWithParentFallback_t AllowDarkModeForWindowWithParentFallback;

BOOL IsDarkMode = FALSE;
#pragma endregion
namespace DarkMode
{
	namespace Menu
	{
#pragma region Flags
		// window messages related to menu bar drawing
#define WM_UAHDESTROYWINDOW    0x0090	// handled by DefWindowProc
#define WM_UAHDRAWMENU         0x0091	// lParam is UAHMENU
#define WM_UAHDRAWMENUITEM     0x0092	// lParam is UAHDRAWMENUITEM
#define WM_UAHINITMENU         0x0093	// handled by DefWindowProc
#define WM_UAHMEASUREMENUITEM  0x0094	// lParam is UAHMEASUREMENUITEM
#define WM_UAHNCPAINTMENUPOPUP 0x0095	// handled by DefWindowProc

// describes the sizes of the menu bar or menu item
		typedef union tagUAHMENUITEMMETRICS
		{
			// cx appears to be 14 / 0xE less than rcItem's width!
			// cy 0x14 seems stable, i wonder if it is 4 less than rcItem's height which is always 24 atm
			struct {
				DWORD cx;
				DWORD cy;
			} rgsizeBar[2];
			struct {
				DWORD cx;
				DWORD cy;
			} rgsizePopup[4];
		} UAHMENUITEMMETRICS;

		// not really used in our case but part of the other structures
		typedef struct tagUAHMENUPOPUPMETRICS
		{
			DWORD rgcx[4];
			DWORD fUpdateMaxWidths : 2; // from kernel symbols, padded to full dword
		} UAHMENUPOPUPMETRICS;

		// hmenu is the main window menu; hdc is the context to draw in
		typedef struct tagUAHMENU
		{
			HMENU hmenu;
			HDC hdc;
			DWORD dwFlags; // no idea what these mean, in my testing it's either 0x00000a00 or sometimes 0x00000a10
		} UAHMENU;

		// menu items are always referred to by iPosition here
		typedef struct tagUAHMENUITEM
		{
			int iPosition; // 0-based position of menu item in menubar
			UAHMENUITEMMETRICS umim;
			UAHMENUPOPUPMETRICS umpm;
		} UAHMENUITEM;

		// the DRAWITEMSTRUCT contains the states of the menu items, as well as
		// the position index of the item in the menu, which is duplicated in
		// the UAHMENUITEM's iPosition as well
		typedef struct UAHDRAWMENUITEM
		{
			DRAWITEMSTRUCT dis; // itemID looks uninitialized
			UAHMENU um;
			UAHMENUITEM umi;
		} UAHDRAWMENUITEM;

		// the MEASUREITEMSTRUCT is intended to be filled with the size of the item
		// height appears to be ignored, but width can be modified
		typedef struct tagUAHMEASUREMENUITEM
		{
			MEASUREITEMSTRUCT mis;
			UAHMENU um;
			UAHMENUITEM umi;
		} UAHMEASUREMENUITEM;
#pragma endregion
		static HTHEME g_menuTheme = nullptr;

		// processes messages related to UAH / custom menubar drawing.
		// return true if handled, false to continue with normal processing in your wndproc
		bool UAHDarkModeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lr)
		{
			switch (message)
			{
			case WM_UAHDRAWMENU:
			{
				UAHMENU* pUDM = (UAHMENU*)lParam;
				RECT rc = { 0 };
				{
					MENUBARINFO mbi = { sizeof(mbi) };
					GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi);
					RECT rcWindow;
					GetWindowRect(hWnd, &rcWindow);
					rc = mbi.rcBar;
					OffsetRect(&rc, -rcWindow.left, -rcWindow.top);
					rc.top -= 1;
				}
				if (!g_menuTheme) {
					g_menuTheme = OpenThemeData(hWnd, L"Menu");
				}
				if (IsDarkMode)
					DrawThemeBackground(g_menuTheme, pUDM->hdc, MENU_POPUPITEM, MPI_NORMAL, &rc, nullptr);
				else
					return false;
				return true;
			}
			case WM_UAHDRAWMENUITEM:
			{
				UAHDRAWMENUITEM* pUDMI = (UAHDRAWMENUITEM*)lParam;
				wchar_t menuString[256] = { 0 };
				MENUITEMINFO mii = { sizeof(mii), MIIM_STRING };
				{
					mii.dwTypeData = menuString;
					mii.cch = (sizeof(menuString) / 2) - 1;
					GetMenuItemInfo(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii);
				}
				DWORD dwFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
				int iTextStateID = 0;
				int iBackgroundStateID = 0;
				{
					if ((pUDMI->dis.itemState & ODS_INACTIVE) | (pUDMI->dis.itemState & ODS_DEFAULT)) {
						iTextStateID = MPI_NORMAL;
						iBackgroundStateID = MPI_NORMAL;
					}
					if (pUDMI->dis.itemState & ODS_HOTLIGHT) {
						iTextStateID = MPI_HOT;
						iBackgroundStateID = MPI_HOT;
					}
					if (pUDMI->dis.itemState & ODS_SELECTED) {
						iTextStateID = MPI_HOT;
						iBackgroundStateID = MPI_HOT;
					}
					if ((pUDMI->dis.itemState & ODS_GRAYED) || (pUDMI->dis.itemState & ODS_DISABLED)) {
						iTextStateID = MPI_DISABLED;
						iBackgroundStateID = MPI_DISABLED;
					}
					if (pUDMI->dis.itemState & ODS_NOACCEL) {
						dwFlags |= DT_HIDEPREFIX;
					}
				}
				if (!g_menuTheme) {
					g_menuTheme = OpenThemeData(hWnd, L"Menu");
				}
				if (IsDarkMode)
				{
					DrawThemeBackground(g_menuTheme, pUDMI->um.hdc, MENU_POPUPITEM, iBackgroundStateID, &pUDMI->dis.rcItem, nullptr);
					DrawThemeText(g_menuTheme, pUDMI->um.hdc, MENU_POPUPITEM, iTextStateID, menuString, mii.cch, dwFlags, 0, &pUDMI->dis.rcItem);
				}
				else
					return false;
				return true;
			}
			case WM_THEMECHANGED:
			{
				if (g_menuTheme) {
					CloseThemeData(g_menuTheme);
					g_menuTheme = nullptr;
				}
				return false;
			}
			default:
				return false;
			}
		}

		void drawUAHMenuNCBottomLine(HWND hWnd)
		{
			MENUBARINFO mbi = { sizeof(mbi) };
			if (!GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi)) return;
			RECT rcClient = { 0 };
			GetClientRect(hWnd, &rcClient);
			MapWindowPoints(hWnd, nullptr, (POINT*)&rcClient, 2);
			RECT rcWindow = { 0 };
			GetWindowRect(hWnd, &rcWindow);
			OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);
			RECT rcAnnoyingLine = rcClient;
			rcAnnoyingLine.bottom = rcAnnoyingLine.top;
			rcAnnoyingLine.top--;
			HDC hdc = GetWindowDC(hWnd);
			COLORREF color;
			HTHEME them = OpenThemeData(0, IsDarkMode ? L"DarkMode::ItemsView" : L"ItemsView");
			GetThemeColor(them, 0, 0, TMT_FILLCOLOR, &color);
			SetDCBrushColor(hdc, color);
			FillRect(hdc, &rcAnnoyingLine, (HBRUSH)GetStockObject(DC_BRUSH));
			ReleaseDC(hWnd, hdc);
		}

	}

	void InitDarkMode()
	{
		HMODULE hUxtheme = LoadLibraryEx(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (hUxtheme)
		{
			OpenNcThemeData = (OpenNcThemeData_t)(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(49)));
			ShouldAppsUseDarkMode = (ShouldAppsUseDarkMode_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132));
			SetPreferredAppMode = (SetPreferredAppMode_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
			AllowDarkModeForWindowWithParentFallback = (AllowDarkModeForWindowWithParentFallback_t)(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(145)));

			SetPreferredAppMode(PreferredAppMode::AllowDark);
			IsDarkMode = ShouldAppsUseDarkMode();
		}
	}

	void UpdateTitleBar(HWND hWnd)
	{
		if (IsDarkMode)
		{
			DwmSetWindowAttribute(hWnd, 20, &IsDarkMode, sizeof(IsDarkMode));
		}
	}

	void FixDarkScrollBar()
	{
		HMODULE hComctl = LoadLibraryExW(L"comctl32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (hComctl)
		{
			auto addr = FindDelayLoadThunkInModule(hComctl, "uxtheme.dll", 49); // OpenNcThemeData
			if (addr)
			{
				DWORD oldProtect;
				if (VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), PAGE_READWRITE, &oldProtect))
				{
					auto MyOpenThemeData = [](HWND hWnd, LPCWSTR classList) -> HTHEME {
						if (wcscmp(classList, L"ScrollBar") == 0)
						{
							hWnd = nullptr;
							classList = L"Explorer::ScrollBar";
						}
						return OpenNcThemeData(hWnd, classList);
					};

					addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<OpenNcThemeData_t>(MyOpenThemeData));
					VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
				}
			}
		}
	}
}