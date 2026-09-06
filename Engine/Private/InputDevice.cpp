#include "InputDevice.h"

CInputDevice::CInputDevice(void)
{
	ZeroMemory(m_byKeyState, sizeof(m_byKeyState));
	ZeroMemory(m_byPrevKeyState, sizeof(m_byPrevKeyState));
}

HRESULT CInputDevice::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	/* 0. DInput */
	if (FAILED(DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pInputSDK,
		NULL)))
		return E_FAIL;

	/* 1. KeyBoard */
	if (FAILED(m_pInputSDK->CreateDevice(
		GUID_SysKeyboard,
		&m_pKeyBoard,
		nullptr)))
		return E_FAIL;

	m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard);
	m_pKeyBoard->SetCooperativeLevel(
		hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE); 
	m_pKeyBoard->Acquire();

	/* 2. Mouse */
	if (FAILED(m_pInputSDK->CreateDevice(
		GUID_SysMouse,
		&m_pMouse,
		nullptr)))
		return E_FAIL;

	m_pMouse->SetDataFormat(&c_dfDIMouse);
	m_pMouse->SetCooperativeLevel(
		hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	m_pMouse->Acquire();

	return S_OK;
}

void CInputDevice::Update(void)
{
	/* 1. 이전상태 백업 */
	memcpy(m_byPrevKeyState, m_byKeyState, sizeof(m_byKeyState));

	/* 2. 현재 상태 갱신 */
	m_pKeyBoard->GetDeviceState(256, m_byKeyState);
	m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);
}

_byte CInputDevice::Get_DIKeyState(_ubyte byKeyID)
{
	// ex. m_pGameInstance->Get_DIKeyState(DIK_F) & 0x80
	return m_byKeyState[byKeyID];
}

_byte CInputDevice::Get_DIKeyDown(_ubyte byKeyID)
{
	return (m_byKeyState[byKeyID] & 0x80) &&
		!(m_byPrevKeyState[byKeyID] & 0x80);
}

_byte CInputDevice::Get_DIKeyUp(_ubyte byKeyID)
{
	return !(m_byKeyState[byKeyID] & 0x80) &&
		(m_byPrevKeyState[byKeyID] & 0x80);
}

_byte CInputDevice::Get_DIKeyPressing(_ubyte byKeyID)
{
	return (m_byKeyState[byKeyID] & 0x80);
}

_byte CInputDevice::Get_DIMouseState(DIMB eMouse)
{
	return m_tMouseState.rgbButtons[static_cast<_uint>(eMouse)];
}

_long CInputDevice::Get_DIMouseMove(DIMM eMouseState)
{
	return *((reinterpret_cast<_long*>(&m_tMouseState))
		+ static_cast<_uint>(eMouseState));
}

CInputDevice* CInputDevice::Create(HINSTANCE hInstance, HWND hWnd)
{
	CInputDevice* pInstance = new CInputDevice();
	if (FAILED(pInstance->Initialize(hInstance, hWnd)))
	{
		MSG_BOX("FAILED TO CREATED : CInputDevice");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CInputDevice::Free(void)
{
	Safe_Release(m_pMouse);
	Safe_Release(m_pKeyBoard);
	Safe_Release(m_pInputSDK);
}
