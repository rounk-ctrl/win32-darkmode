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
using fnRtlGetNtVersionNumbers = void (WINAPI*)(LPDWORD major, LPDWORD minor, LPDWORD build);
// 1809 17763
using fnShouldAppsUseDarkMode = bool (WINAPI*)(); // ordinal 132
using fnAllowDarkModeForWindow = bool (WINAPI*)(HWND hWnd, bool allow); // ordinal 133
using fnFlushMenuThemes = void (WINAPI*)(); // ordinal 136
using fnAllowDarkModeForApp = bool (WINAPI*)(bool allow); // ordinal 135, in 1809
using fnRefreshImmersiveColorPolicyState = void (WINAPI*)(); // ordinal 104
using fnIsDarkModeAllowedForWindow = bool (WINAPI*)(HWND hWnd); // ordinal 137
using fnGetIsImmersiveColorUsingHighContrast = bool (WINAPI*)(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
using fnOpenNcThemeData = HTHEME(WINAPI*)(HWND hWnd, LPCWSTR pszClassList); // ordinal 49
// 1903 18362
using fnShouldSystemUseDarkMode = bool (WINAPI*)(); // ordinal 138
using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903
using fnIsDarkModeAllowedForApp = bool (WINAPI*)(); // ordinal 139
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