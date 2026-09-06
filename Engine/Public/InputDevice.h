#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CInputDevice : public CBase
{
private:
	CInputDevice(void);
	virtual ~CInputDevice(void) = default;

public:
	HRESULT Initialize(HINSTANCE hInstance, HWND hWnd);
	void Update(void);

	_byte Get_DIKeyState(_ubyte byKeyID);
	_byte Get_DIKeyDown(_ubyte byKeyID);
	_byte Get_DIKeyUp(_ubyte byKeyID);
	_byte Get_DIKeyPressing(_ubyte byKeyID);
	_byte Get_DIMouseState(DIMB eMouse);
	_long Get_DIMouseMove(DIMM eMouseState);

private:
	LPDIRECTINPUT8 m_pInputSDK = { nullptr };
	LPDIRECTINPUTDEVICE8 m_pKeyBoard = { nullptr };
	LPDIRECTINPUTDEVICE8 m_pMouse = { nullptr };

	_byte m_byKeyState[256] = {};
	_byte m_byPrevKeyState[256] = {};
	DIMOUSESTATE m_tMouseState = {};

public:
	static CInputDevice* Create(HINSTANCE hInstance, HWND hWnd);
	virtual void Free(void);
};
NS_END