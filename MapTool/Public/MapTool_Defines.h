#pragma once

#ifndef MapTool_Define_h__

#define MapTool_Define_h__

#include <Windows.h>
#include <process.h>

/* DX */
#include <DirectXTex/DirectXTex.h>

/* ImGui */
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

/* Assimp */
#include <Assimp/scene.h>
#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>

/* XML */
#include "tinyxml2.h"

/* Engine*/
#include "Engine_Defines.h"

/* Connet my Engine_Defines.. */
#include "MapTool_Struct.h"

namespace MapTool {
	static constexpr unsigned int g_iWinSizeX = 1280;
	static constexpr unsigned int g_iWinSizeY = 720;

	enum class LEVEL { STATIC, LOADING, MAPEDITOR, ASSIMPEDITOR, PARTSBUILDER, NAVIGATIONEDITOR, EFFECTEDITOR, END };
}

extern HWND g_hWnd;
extern HINSTANCE g_hInstance;

using namespace MapTool;

#endif // MapTool_Define_h__