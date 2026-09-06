#include "CustomFont.h"

CCustomFont::CCustomFont(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: m_pDevice { _pDevice }
	, m_pDeviceContext { _pDeviceContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pDeviceContext);
}

HRESULT CCustomFont::Initialize(const _tchar* _pFontFilePath)
{
	m_pFont = new SpriteFont(m_pDevice, _pFontFilePath);

	return S_OK;
}

CCustomFont* CCustomFont::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext, const _tchar* _pFontFilePath)
{
	CCustomFont* pInstance =  new CCustomFont(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize(_pFontFilePath)))
	{
		MSG_BOX("FAILED TO CREATED : CCustomFont");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCustomFont::Free()
{
	Safe_Delete(m_pFont);
	Safe_Release(m_pDeviceContext);
	Safe_Release(m_pDevice);
}
