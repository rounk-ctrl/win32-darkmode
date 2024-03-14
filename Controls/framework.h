// header.h : include file for standard system include files,
// or project specific include files
//
#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <WindowsX.h>
#include <Vssym32.h>
#include <tchar.h>
#include <cstdint>
#include <limits>
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Msimg32.lib")