#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CFontManager final : public CBase
{
public:
	typedef struct tagFontDesc
	{
		_wstring wstrText;
		_float2 vPosition;
		_float4 vColor;
		class CCustomFont* pFont;
		_float fRotation = 0.f;
		_float fScale = 1.f;
	}FONT_DESC;

public:
	CFontManager();
	virtual ~CFontManager() = default;

public:
	HRESULT Initialize();
	HRESULT Add_Fonts(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pFontTag, const _tchar* _pFontFilePath);
	HRESULT Add_Message(const _tchar* _pFontTag, const _tchar* _pText, _fvector _vPosition, _float _fScale = 1.f, 
		_fvector _vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float _fRotation = 0.f);
	HRESULT Render_Fonts(ID3D11DeviceContext* _DeviceContext);

private:
	map<const _tchar*, class CCustomFont*> m_Fonts;
	vector<FONT_DESC> m_vecFonts;

private:
	class CCustomFont* Find_Fonts(const _tchar* _pFontTag);
	vector<_wstring> Split_Lines(const _wstring& _wstrText);

public:
	static CFontManager* Create();
	virtual void Free() override;
};
NS_END