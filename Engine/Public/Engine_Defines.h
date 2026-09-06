#ifndef Engine_Define_h__

#define Engine_Define_h__

/* 00. Warning */
#pragma warning(disable : 4251)

/* 01. DX11 */
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <FX11/d3dx11effect.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/SpriteBatch.h>
#include <DirectXTK/SpriteFont.h>

#include <DirectXTK/PrimitiveBatch.h>
#include <DirectXTK/Effects.h>
#include <DirectXTK/VertexTypes.h>
using namespace DirectX;

/* File */
#include <filesystem>
#include <fstream>

/* XML */
#include "tinyxml2.h"

/* 02. Basic STL */
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <ctime>
using namespace std;

/* 03. Connet my Engine_Defines.. */
#include "Engine_Typedef.h"
#include "Engine_Enum.h"
#include "Engine_Struct.h"
#include "Engine_Marcro.h"
#include "Engine_Function.h"

/* Sound */
#include "FMOD/fmod.h"
#include "FMOD/fmod.hpp"

/* 04. Key Input */
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

/* 05. Memory Leak */
#ifdef  _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 
#endif

#endif //  _DEBUG

/* 99. namespace Engine */
using namespace Engine;

namespace Engine
{
	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608; // 2048;
}

#endif // Engine_Define_h__