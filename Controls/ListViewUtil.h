#pragma once
//https://github.com/ysc3839/win32-darkmode/blob/master/win32-darkmode/ListViewUtil.h
#include "framework.h"
#include "DarkMode.h"

struct SubclassInfo
{
	COLORREF headerTextColor;
};

void InitListView(HWND hListView)
{
	HWND hHeader = ListView_GetHeader(hListView);

	SetWindowSubclass(hListView, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR /*uIdSubclass*/, DWORD_PTR dwRefData) -> LRESULT {
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			if (reinterpret_cast<LPNMHDR>(lParam)->code == NM_CUSTOMDRAW)
			{
				LPNMCUSTOMDRAW nmcd = reinterpret_cast<LPNMCUSTOMDRAW>(lParam);
				switch (nmcd->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					auto info = reinterpret_cast<SubclassInfo*>(dwRefData);
					SetTextColor(nmcd->hdc, info->headerTextColor);
					return CDRF_DODEFAULT;
				}
				}
			}
		}
		break;
		case WM_THEMECHANGED:
		{
			{
				HWND hHeader = ListView_GetHeader(hWnd);

				HTHEME hTheme = OpenThemeData(nullptr, L"ItemsView");
				if (hTheme)
				{
					COLORREF color;
					if (SUCCEEDED(GetThemeColor(hTheme, 0, 0, TMT_TEXTCOLOR, &color)))
					{
						ListView_SetTextColor(hWnd, color);
					}
					if (SUCCEEDED(GetThemeColor(hTheme, 0, 0, TMT_FILLCOLOR, &color)))
					{
						ListView_SetTextBkColor(hWnd, color);
						ListView_SetBkColor(hWnd, color);
					}
					CloseThemeData(hTheme);
				}

				hTheme = OpenThemeData(hHeader, L"Header");
				if (hTheme)
				{
					auto info = reinterpret_cast<SubclassInfo*>(dwRefData);
					if (FAILED(GetThemeColor(hTheme, HP_HEADERITEM, 0, TMT_TEXTCOLOR, &(info->headerTextColor))))
					{
						info->headerTextColor = clrTxt;
					}
					CloseThemeData(hTheme);
				}

				SendMessageW(hHeader, WM_THEMECHANGED, wParam, lParam);

				RedrawWindow(hWnd, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
			}
		}
		break;
		case WM_DESTROY:
		{
			auto info = reinterpret_cast<SubclassInfo*>(dwRefData);
			delete info;
		}
		break;
		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}, 0, reinterpret_cast<DWORD_PTR>(new SubclassInfo{}));


	//SetWindowTheme(hHeader, L"ItemsView", nullptr);
	SetWindowTheme(hListView, L"ItemsView", nullptr);
}