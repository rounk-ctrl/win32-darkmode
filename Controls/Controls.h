#pragma once

#include "resource.h"

#define MAX_LOADSTRING 100

#pragma region Flags
enum IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};
enum class PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

#pragma endregion
#pragma region Function Signatures
typedef HTHEME(WINAPI* OpenNcThemeData_t)(HWND hWnd, LPCWSTR pszClassList); // ordinal 49
typedef PreferredAppMode(WINAPI* SetPreferredAppMode_t)(PreferredAppMode); // ordinal 135
typedef bool (WINAPI* ShouldAppsUseDarkMode_t)(); // ordinal 132
typedef bool (WINAPI* AllowDarkModeForWindowWithParentFallback_t)(HWND, bool); // ordinal 145

#pragma endregion

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND status;
HFONT defaultFont;
COLORREF clr;
COLORREF clrLight;
COLORREF editBg;
COLORREF clrTxt;
COLORREF clrTxtDis;

/* HWNDS */
HWND stdBtn;
HWND disblBtn;

/* Function prototypes */
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

void UpdateFont(HWND hWndParent, WORD id, WORD id2)
{
    if (!defaultFont)
    {
        NONCLIENTMETRICS metrics = { sizeof(NONCLIENTMETRICS) };
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &metrics, 0))
        {
            defaultFont = CreateFontIndirect(&metrics.lfMessageFont);
        }
    }
	for (int i = id; i <= id2; i++)
	{
        SetWindowFont(GetDlgItem(hWndParent, i), defaultFont, TRUE);
	}
}