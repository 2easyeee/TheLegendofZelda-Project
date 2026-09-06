#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CCustomFont final : public CBase
{
private:
	CCustomFont(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext);
	virtual ~CCustomFont() = default;

public:
	HRESULT Initialize(const _tchar* _pFontFilePath);

	SpriteFont* Get_Font() { return m_pFont; }

private:
	ID3D11Device* m_pDevice = {};
	ID3D11DeviceContext* m_pDeviceContext = {};

	SpriteFont* m_pFont = {};

public:
	static CCustomFont* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pFontFilePath);
	virtual void Free() override;
};
NS_END