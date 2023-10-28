#pragma once
#include "Controls.h"

#pragma region IATHook
template <typename T, typename T1, typename T2>
constexpr T RVA2VA(T1 base, T2 rva)
{
    return reinterpret_cast<T>(reinterpret_cast<ULONG_PTR>(base) + rva);
}

template <typename T>
constexpr T DataDirectoryFromModuleBase(void* moduleBase, size_t entryID)
{
    auto dosHdr = reinterpret_cast<PIMAGE_DOS_HEADER>(moduleBase);
    auto ntHdr = RVA2VA<PIMAGE_NT_HEADERS>(moduleBase, dosHdr->e_lfanew);
    auto dataDir = ntHdr->OptionalHeader.DataDirectory;
    return RVA2VA<T>(moduleBase, dataDir[entryID].VirtualAddress);
}

PIMAGE_THUNK_DATA FindAddressByName(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, const char* funcName)
{
    for (; impName->u1.Ordinal; ++impName, ++impAddr)
    {
        if (IMAGE_SNAP_BY_ORDINAL(impName->u1.Ordinal))
            continue;

        auto import = RVA2VA<PIMAGE_IMPORT_BY_NAME>(moduleBase, impName->u1.AddressOfData);
        if (strcmp(import->Name, funcName) != 0)
            continue;
        return impAddr;
    }
    return nullptr;
}

PIMAGE_THUNK_DATA FindAddressByOrdinal(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, uint16_t ordinal)
{
    for (; impName->u1.Ordinal; ++impName, ++impAddr)
    {
        if (IMAGE_SNAP_BY_ORDINAL(impName->u1.Ordinal) && IMAGE_ORDINAL(impName->u1.Ordinal) == ordinal)
            return impAddr;
    }
    return nullptr;
}

PIMAGE_THUNK_DATA FindIatThunkInModule(void* moduleBase, const char* dllName, const char* funcName)
{
    auto imports = DataDirectoryFromModuleBase<PIMAGE_IMPORT_DESCRIPTOR>(moduleBase, IMAGE_DIRECTORY_ENTRY_IMPORT);
    for (; imports->Name; ++imports)
    {
        if (_stricmp(RVA2VA<LPCSTR>(moduleBase, imports->Name), dllName) != 0)
            continue;

        auto origThunk = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->OriginalFirstThunk);
        auto thunk = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->FirstThunk);
        return FindAddressByName(moduleBase, origThunk, thunk, funcName);
    }
    return nullptr;
}

PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, const char* funcName)
{
    auto imports = DataDirectoryFromModuleBase<PIMAGE_DELAYLOAD_DESCRIPTOR>(moduleBase, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
    for (; imports->DllNameRVA; ++imports)
    {
        if (_stricmp(RVA2VA<LPCSTR>(moduleBase, imports->DllNameRVA), dllName) != 0)
            continue;

        auto impName = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportNameTableRVA);
        auto impAddr = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportAddressTableRVA);
        return FindAddressByName(moduleBase, impName, impAddr, funcName);
    }
    return nullptr;
}

PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, uint16_t ordinal)
{
    auto imports = DataDirectoryFromModuleBase<PIMAGE_DELAYLOAD_DESCRIPTOR>(moduleBase, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
    for (; imports->DllNameRVA; ++imports)
    {
        if (_stricmp(RVA2VA<LPCSTR>(moduleBase, imports->DllNameRVA), dllName) != 0)
            continue;

        auto impName = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportNameTableRVA);
        auto impAddr = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportAddressTableRVA);
        return FindAddressByOrdinal(moduleBase, impName, impAddr, ordinal);
    }
    return nullptr;
}
#pragma endregion
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
#pragma region Variables
fnSetPreferredAppMode SetPreferredAppMode;
fnAllowDarkModeForWindow AllowDarkModeForWindow;
fnAllowDarkModeForApp AllowDarkModeForApp;
fnOpenNcThemeData OpenNcThemeData;
DWORD g_buildNumber = 0;
BOOL IsDarkMode = FALSE;
#pragma endregion
#pragma region Menu
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
        DrawThemeBackground(g_menuTheme, pUDM->hdc, MENU_POPUPITEM, MPI_NORMAL, &rc, nullptr);
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
        DrawThemeBackground(g_menuTheme, pUDMI->um.hdc, MENU_POPUPITEM, iBackgroundStateID, &pUDMI->dis.rcItem, nullptr);
        DrawThemeText(g_menuTheme, pUDMI->um.hdc, MENU_POPUPITEM, iTextStateID, menuString, mii.cch, dwFlags, 0, &pUDMI->dis.rcItem);
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
    HTHEME them = OpenThemeData(0, IsDarkMode ? L"DarkMode_Explorer::TaskDialog" : L"Explorer::TaskDialog");
    GetThemeColor(them, TDLG_CONTROLPANE, 0, TMT_FILLCOLOR, &color);
    SetDCBrushColor(hdc, color);
    FillRect(hdc, &rcAnnoyingLine, (HBRUSH)GetStockObject(DC_BRUSH));
    ReleaseDC(hWnd, hdc);
}

#pragma endregion
#pragma region Radio
LRESULT CALLBACK RadioButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    RECT paint;
    int stateId;
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paint = ps.rcPaint;
        // Clear the device context with a white brush
        HBRUSH hBrush = CreateSolidBrush(clr);
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);

        // Determine the state of the button
        stateId = SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? RBS_CHECKEDNORMAL : RBS_UNCHECKEDNORMAL;
        if (!IsWindowEnabled(hwnd))
        {
            stateId = SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? RBS_CHECKEDDISABLED : RBS_UNCHECKEDDISABLED;
        }
        else
        {
            POINT ptCursor;
            GetCursorPos(&ptCursor);
            ScreenToClient(hwnd, &ptCursor);
            if (PtInRect(&ps.rcPaint, ptCursor))
            {
                stateId = SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? RBS_CHECKEDHOT : RBS_UNCHECKEDHOT;
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
                {
                    stateId = SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? RBS_CHECKEDPRESSED : RBS_UNCHECKEDPRESSED;
                }
            }
        }
        RECT rc;
        HTHEME hTheme = OpenThemeData(hwnd, L"Button");
        if (hTheme)
        {
            GetClientRect(hwnd, &rc);
            rc.right = 14;
            DrawThemeBackgroundEx(hTheme, hdc, BP_RADIOBUTTON, stateId, &rc, NULL);
            CloseThemeData(hTheme);
            GetClientRect(hwnd, &rc);
            rc.left += 17;
        }
        // Draw the button text
        TCHAR szText[256];
        GetWindowText(hwnd, szText, 256);
        if (_tcslen(szText) > 0)
        {
            HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            if (stateId == RBS_CHECKEDDISABLED || stateId == RBS_UNCHECKEDDISABLED) {
                SetTextColor(hdc, clrTxtDis);
            }
            else {
                SetTextColor(hdc, clrTxt);
            }
            DrawText(hdc, szText, _tcslen(szText), &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }
        // Clean up
        EndPaint(hwnd, &ps);
        return 0;
    }
    default:
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void InitRadio(HWND hWnd)
{
    SetWindowTheme(hWnd, L"Explorer", NULL);
    AllowDarkModeForWindow(hWnd, IsDarkMode);
    SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
    SetWindowSubclass(hWnd, RadioButtonProc, 0, 0);
}
void InitRadioRange(HWND hWndParent, WORD id, WORD id2)
{
    for (int i = id; i <= id2; i++)
    {
        InitRadio(GetDlgItem(hWndParent, i));
    }
}
#pragma endregion
#pragma region Checkbox
LRESULT CALLBACK CheckBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    RECT paint;
    int stateId;
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paint = ps.rcPaint;
        // Clear the device context with a white brush
        HBRUSH hBrush = CreateSolidBrush(clr);
        FillRect(hdc, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);

        // Determine the state of the button
        stateId = CBS_UNCHECKEDNORMAL;
        if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
        {
            stateId = CBS_CHECKEDNORMAL;
        }
        else if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
        {
            stateId = CBS_MIXEDNORMAL;
        }
        if (!IsWindowEnabled(hwnd))
        {
            stateId = CBS_UNCHECKEDDISABLED;
            if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                stateId = CBS_CHECKEDDISABLED;
            }
            else if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
            {
                stateId = CBS_MIXEDDISABLED;
            }
        }
        else
        {
            POINT ptCursor;
            GetCursorPos(&ptCursor);
            ScreenToClient(hwnd, &ptCursor);
            if (PtInRect(&ps.rcPaint, ptCursor))
            {
                stateId = CBS_UNCHECKEDHOT;
                if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                    stateId = CBS_CHECKEDHOT;
                }
                else if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
                {
                    stateId = CBS_MIXEDHOT;
                }
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
                {
                    stateId = CBS_UNCHECKEDPRESSED;
                    if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        stateId = CBS_CHECKEDPRESSED;
                    }
                    else if (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
                    {
                        stateId = CBS_MIXEDPRESSED;
                    }
                }
            }
        }
        RECT rc;
        HTHEME hTheme = OpenThemeData(hwnd, L"Button");
        if (hTheme)
        {
            GetClientRect(hwnd, &rc);
            rc.right = 14;
            DrawThemeBackgroundEx(hTheme, hdc, BP_CHECKBOX, stateId, &rc, NULL);
            CloseThemeData(hTheme);
            GetClientRect(hwnd, &rc);
            rc.left += 17;
        }
        // Draw the button text
        TCHAR szText[256];
        GetWindowText(hwnd, szText, 256);
        if (_tcslen(szText) > 0)
        {
            HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            if (stateId == CBS_CHECKEDDISABLED || stateId == CBS_UNCHECKEDDISABLED || stateId == CBS_MIXEDDISABLED) {
                SetTextColor(hdc, clrTxtDis);
            }
            else {
                SetTextColor(hdc, clrTxt);
            }
            DrawText(hdc, szText, _tcslen(szText), &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }
        // Clean up
        EndPaint(hwnd, &ps);
        return 0;
    }
    default:
        return DefSubclassProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void InitCheckBox(HWND hWnd)
{
    SetWindowTheme(hWnd, L"Explorer", NULL);
    AllowDarkModeForWindow(hWnd, IsDarkMode);
    SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
    SetWindowSubclass(hWnd, CheckBoxProc, 0, 0);
}
void InitCheckBoxRange(HWND hWndParent, WORD id, WORD id2)
{
    for (int i = id; i <= id2; i++)
    {
        InitCheckBox(GetDlgItem(hWndParent, i));
    }
}
#pragma endregion
#pragma region Statusbar
#pragma region Variables
typedef struct _PHP_THEME_WINDOW_STATUSBAR_CONTEXT
{
    BOOLEAN MouseActive;
    HTHEME StatusThemeData;
} PHP_THEME_WINDOW_STATUSBAR_CONTEXT, * PPHP_THEME_WINDOW_STATUSBAR_CONTEXT;

HWND StatusBarHandle;
PHP_THEME_WINDOW_STATUSBAR_CONTEXT StatusBarContext;
HFONT PhpStatusBarFontHandle = NULL;
ULONG PhpThemeColorMode = 0;
BOOLEAN PhpThemeEnable = FALSE;
#pragma endregion
LRESULT CALLBACK PhpThemeWindowStatusbarWndSubclassProc(
    _In_ HWND WindowHandle,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ UINT_PTR uIdSubclass,
    _In_ DWORD_PTR dwRefData
)
{
    PPHP_THEME_WINDOW_STATUSBAR_CONTEXT context;
    context = &StatusBarContext;
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return FALSE;
    case WM_PAINT:
    {
        RECT clientRect;
        PAINTSTRUCT ps;
        INT blockCoord[128];
        INT blockCount = (INT)SendMessage(WindowHandle, (UINT)SB_GETPARTS, (WPARAM)ARRAYSIZE(blockCoord), (WPARAM)blockCoord);
        GetClientRect(WindowHandle, &clientRect);
        if (!BeginPaint(WindowHandle, &ps))
            break;
        SetBkMode(ps.hdc, TRANSPARENT);
        HDC hdc = CreateCompatibleDC(ps.hdc);
        SetBkMode(hdc, TRANSPARENT);
        HBITMAP hbm = CreateCompatibleBitmap(ps.hdc, clientRect.right, clientRect.bottom);
        SelectBitmap(hdc, hbm);
        if (!PhpStatusBarFontHandle)
        {
            NONCLIENTMETRICS metrics = { sizeof(NONCLIENTMETRICS) };
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &metrics, 0))
            {
                PhpStatusBarFontHandle = CreateFontIndirect(&metrics.lfMessageFont);
            }
        }
        SelectFont(hdc, PhpStatusBarFontHandle);
        HTHEME them = OpenThemeData(WindowHandle, L"DarkMode::ExplorerStatusBar");
        COLORREF clr;
        COLORREF txtClr;
        GetThemeColor(them, 0, 0, TMT_FILLCOLOR, &clr);
        GetThemeColor(them, 0, 0, TMT_TEXTCOLOR, &txtClr);
        SetTextColor(hdc, txtClr);
        SetDCBrushColor(hdc, clr);
        FillRect(hdc, &clientRect, GetStockBrush(DC_BRUSH));
        for (INT i = 0; i < blockCount; i++)
        {
            RECT blockRect = { 0, 0, 0, 0 };
            WCHAR buffer[MAX_PATH] = L"";
            if (!SendMessage(WindowHandle, SB_GETRECT, (WPARAM)i, (WPARAM)&blockRect))
                continue;
            if (!SendMessage(WindowHandle, SB_GETTEXT, (WPARAM)i, (LPARAM)buffer))
                continue;
            POINT pt;
            GetCursorPos(&pt);
            MapWindowPoints(NULL, WindowHandle, &pt, 1);
            SetTextColor(hdc, txtClr);
            SetDCBrushColor(hdc, clr);
            FillRect(hdc, &blockRect, GetStockBrush(DC_BRUSH));
            DrawText(hdc, buffer, -1, &blockRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        {
            RECT sizeGripRect;
            sizeGripRect.left = clientRect.right - GetSystemMetrics(SM_CXHSCROLL);
            sizeGripRect.top = clientRect.bottom - GetSystemMetrics(SM_CYVSCROLL);
            sizeGripRect.right = clientRect.right;
            sizeGripRect.bottom = clientRect.bottom;
            if (context->StatusThemeData)
                DrawThemeBackground(context->StatusThemeData, hdc, SP_GRIPPER, 0, &sizeGripRect, &sizeGripRect);
            else
                DrawFrameControl(hdc, &sizeGripRect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
        }
        BitBlt(ps.hdc, 0, 0, clientRect.right, clientRect.bottom, hdc, 0, 0, SRCCOPY);
        DeleteDC(hdc);
        DeleteBitmap(hbm);
        EndPaint(WindowHandle, &ps);
    }
    goto DefaultWndProc;
    default:
        break;
    }
    return DefSubclassProc(WindowHandle, uMsg, wParam, lParam);

DefaultWndProc:
    return DefWindowProc(WindowHandle, uMsg, wParam, lParam);
}

BOOL SubclassStatusBar(HWND hWnd)
{
    if (StatusBarHandle)
        return FALSE;
    StatusBarHandle = hWnd;
    PPHP_THEME_WINDOW_STATUSBAR_CONTEXT context = &StatusBarContext;
    memset(context, 0, sizeof(*context));
    context->StatusThemeData = OpenThemeData(StatusBarHandle, VSCLASS_STATUS);
    SetWindowSubclass(StatusBarHandle, PhpThemeWindowStatusbarWndSubclassProc, 0, 0);
    InvalidateRect(StatusBarHandle, NULL, FALSE);
    return TRUE;
}

#pragma endregion
#pragma region Tab
typedef struct _PHP_THEME_WINDOW_TAB_CONTEXT
{
    WNDPROC DefaultWindowProc;
    BOOLEAN MouseActive;
    POINT CursorPos;
} PHP_THEME_WINDOW_TAB_CONTEXT, * PPHP_THEME_WINDOW_TAB_CONTEXT;
PHP_THEME_WINDOW_TAB_CONTEXT TabContext;

FORCEINLINE
BOOLEAN
PhPtInRect(
    _In_ PRECT Rect,
    _In_ POINT Point
)
{
    return Point.x >= Rect->left && Point.x < Rect->right &&
        Point.y >= Rect->top && Point.y < Rect->bottom;
}

FORCEINLINE
VOID
PhOffsetRect(
    _In_ PRECT Rect,
    _In_ INT dx,
    _In_ INT dy
)
{
    Rect->left += dx;
    Rect->top += dy;
    Rect->right += dx;
    Rect->bottom += dy;
}
COLORREF PhThemeWindowBackground2Color = RGB(65, 65, 65);
COLORREF PhThemeWindowHighlightColor = RGB(128, 128, 128);

VOID ThemeWindowRenderTabControl(
    _In_ PPHP_THEME_WINDOW_TAB_CONTEXT Context,
    _In_ HWND WindowHandle,
    _In_ HDC bufferDc,
    _In_ PRECT clientRect,
    _In_ WNDPROC WindowProcedure
)
{
    SetBkMode(bufferDc, TRANSPARENT);
    SelectFont(bufferDc, GetWindowFont(WindowHandle));
    SetTextColor(bufferDc, clrTxt);
    FillRect(bufferDc, clientRect, CreateSolidBrush(clr));
    INT currentSelection = TabCtrl_GetCurSel(WindowHandle);
    INT count = TabCtrl_GetItemCount(WindowHandle);

    for (INT i = 0; i < count; i++)
    {
        RECT itemRect;
        TabCtrl_GetItemRect(WindowHandle, i, &itemRect);

        if (PhPtInRect(&itemRect, Context->CursorPos) && (currentSelection != i))
        {
            PhOffsetRect(&itemRect, 2, 2);
            //SetDCBrushColor(bufferDc, PhThemeWindowHighlightColor);
            //FillRect(bufferDc, &itemRect, GetStockBrush(DC_BRUSH));
        }
        else
        {
            PhOffsetRect(&itemRect, 2, 2);
            if (currentSelection == i)
            {
                FillRect(bufferDc, &itemRect, CreateSolidBrush(clr));
            }
            else
            {
                SetDCBrushColor(bufferDc, PhThemeWindowBackground2Color);// PhThemeWindowHighlightColor); // PhThemeWindowForegroundColor);
                FillRect(bufferDc, &itemRect, GetStockBrush(DC_BRUSH));
            }
        }
        {
            TCITEM tabItem;
            WCHAR tabHeaderText[MAX_PATH] = L"";
            memset(&tabItem, 0, sizeof(TCITEM));
            tabItem.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
            tabItem.dwStateMask = TCIS_BUTTONPRESSED | TCIS_HIGHLIGHTED;
            tabItem.cchTextMax = RTL_NUMBER_OF(tabHeaderText);
            tabItem.pszText = tabHeaderText;
            if (TabCtrl_GetItem(WindowHandle, i, &tabItem))
            {
                DrawText(
                    bufferDc,
                    tabItem.pszText,
                    _tcslen(tabItem.pszText),
                    &itemRect,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_HIDEPREFIX
                );
            }
        }
    }
}


LRESULT CALLBACK PhpThemeWindowTabControlWndSubclassProc(
    _In_ HWND WindowHandle,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    PPHP_THEME_WINDOW_TAB_CONTEXT context;
    WNDPROC oldWndProc;
    context = &TabContext;
    oldWndProc = context->DefaultWindowProc;
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return TRUE;
    case WM_MOUSEMOVE:
    {
        if (!context->MouseActive)
        {
            TRACKMOUSEEVENT trackEvent =
            {
                sizeof(TRACKMOUSEEVENT),
                TME_LEAVE,
                WindowHandle,
                0
            };

            TrackMouseEvent(&trackEvent);
            context->MouseActive = TRUE;
        }

        context->CursorPos.x = GET_X_LPARAM(lParam);
        context->CursorPos.y = GET_Y_LPARAM(lParam);

        InvalidateRect(WindowHandle, NULL, FALSE);
    }
    break;
    case WM_MOUSELEAVE:
    {
        context->CursorPos.x = LONG_MIN;
        context->CursorPos.y = LONG_MIN;

        context->MouseActive = FALSE;
        InvalidateRect(WindowHandle, NULL, FALSE);
    }
    break;
    case WM_PAINT:
    {
        {
            RECT clientRect;
            HDC hdc;
            HDC bufferDc;
            HBITMAP bufferBitmap;
            HBITMAP oldBufferBitmap;

            GetClientRect(WindowHandle, &clientRect);

            hdc = GetDC(WindowHandle);
            bufferDc = CreateCompatibleDC(hdc);
            bufferBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            oldBufferBitmap = SelectBitmap(bufferDc, bufferBitmap);

            {
                RECT clientRect2 = clientRect;
                TabCtrl_AdjustRect(WindowHandle, FALSE, &clientRect2);
                ExcludeClipRect(hdc, clientRect2.left, clientRect2.top, clientRect2.right, clientRect2.bottom);
            }

            ThemeWindowRenderTabControl(context, WindowHandle, bufferDc, &clientRect, oldWndProc);

            BitBlt(hdc, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom, bufferDc, 0, 0, SRCCOPY);
            SelectBitmap(bufferDc, oldBufferBitmap);
            DeleteBitmap(bufferBitmap);
            DeleteDC(bufferDc);
            ReleaseDC(WindowHandle, hdc);
        }
    }
    goto DefaultWndProc;
    }

    return CallWindowProc(oldWndProc, WindowHandle, uMsg, wParam, lParam);

DefaultWndProc:
    return DefWindowProc(WindowHandle, uMsg, wParam, lParam);
}

VOID PhInitializeThemeWindowTabControl(
    _In_ HWND TabControlWindow
)
{
    PPHP_THEME_WINDOW_TAB_CONTEXT context = &TabContext;
    memset(context, 0, sizeof(*context));
    context->DefaultWindowProc = (WNDPROC)GetWindowLongPtr(TabControlWindow, GWLP_WNDPROC);
    context->CursorPos.x = LONG_MIN;
    context->CursorPos.y = LONG_MIN;
    SetWindowLongPtr(TabControlWindow, GWLP_WNDPROC, (LONG_PTR)PhpThemeWindowTabControlWndSubclassProc);
    InvalidateRect(TabControlWindow, NULL, FALSE);
}
#pragma endregion

bool IsHighContrast()
{
	HIGHCONTRASTW highContrast = { sizeof(highContrast) };
	if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
		return highContrast.dwFlags & HCF_HIGHCONTRASTON;
	return false;
}
int IsExplorerDarkTheme()
{
	HKEY hKeyPersonalization;
	RegOpenKeyEx(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		0, KEY_READ, &hKeyPersonalization
	);
	DWORD dwBufferSize(sizeof(DWORD));
	DWORD nResult(0);
	LONG nError = RegQueryValueEx(
		hKeyPersonalization,
		L"AppsUseLightTheme",
		0,
		NULL,
		reinterpret_cast<LPBYTE>(&nResult),
		&dwBufferSize
	);
	return ERROR_SUCCESS == nError ? !nResult : FALSE;
}
void InitDarkMode()
{
	auto RtlGetNtVersionNumbers = reinterpret_cast<fnRtlGetNtVersionNumbers>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlGetNtVersionNumbers"));
	if (RtlGetNtVersionNumbers)
	{
		DWORD major, minor;
		RtlGetNtVersionNumbers(&major, &minor, &g_buildNumber);
		g_buildNumber &= ~0xF0000000;
		if (major == 10 && g_buildNumber >= 17763)
		{
			HMODULE hUxtheme = LoadLibraryEx(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
			if (hUxtheme)
			{
				AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
				OpenNcThemeData = reinterpret_cast<fnOpenNcThemeData>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(49)));
				if (g_buildNumber >= 17763 && g_buildNumber < 18362)
				{
					AllowDarkModeForApp = reinterpret_cast<fnAllowDarkModeForApp>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));
					AllowDarkModeForApp(true);
				}
				else if (g_buildNumber >= 18362)
				{
					SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
					SetPreferredAppMode(PreferredAppMode::AllowDark);
				}
				IsDarkMode = IsExplorerDarkTheme() && !IsHighContrast();

			}
		}
	}
}

void UpdateTitleBar(HWND hWnd)
{

	if (IsDarkMode)
	{
		if (g_buildNumber >= 17763 && g_buildNumber < 18362)
		{
			SetProp(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<INT_PTR>(IsDarkMode)));
		}
		else if (g_buildNumber >= 18362 && g_buildNumber < 19041)
		{
			DwmSetWindowAttribute(hWnd, 19, &IsDarkMode, sizeof(IsDarkMode));
		}
		else if (g_buildNumber >= 19041)
		{
			DwmSetWindowAttribute(hWnd, 20, &IsDarkMode, sizeof(IsDarkMode));
		}
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

				addr->u1.Function = reinterpret_cast<ULONG_PTR>(static_cast<fnOpenNcThemeData>(MyOpenThemeData));
				VirtualProtect(addr, sizeof(IMAGE_THUNK_DATA), oldProtect, &oldProtect);
			}
		}
	}
}
void EnableDarkMode(HWND hWnd, LPCWSTR theme) {
	SetWindowTheme(hWnd, theme, NULL);
	AllowDarkModeForWindow(hWnd, IsDarkMode);
	SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
}
