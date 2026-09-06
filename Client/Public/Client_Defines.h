#pragma once

#include <Windows.h>
#include <process.h>

namespace Client {
	static constexpr unsigned int g_iWinSizeX = 1280;
	static constexpr unsigned int g_iWinSizeY = 720;

	enum class LEVEL {STATIC, LOADING, LOGO, MARINHOUSE, SHOP, TELEPHONEBOX,
						FIELD, MARTHAsBAY, DUNGEON, TOWER, DUNGEON_SIDEVIEW, 
						CREDIT, SANDBOX, END};
}

extern HWND g_hWnd;
extern HINSTANCE g_hInstance;

using namespace Client;