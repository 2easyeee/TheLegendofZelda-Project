#include "UI_WorldBackground.h"
#include "GameInstance.h"

CUI_WorldBackground::CUI_WorldBackground(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
	: CUI_Rect{ _pDevice, _pDeviceContext }
{
}

CUI_WorldBackground::CUI_WorldBackground(const CUI_WorldBackground& _Prototype)
	: CUI_Rect(_Prototype)
{
}

HRESULT CUI_WorldBackground::Initialize_Prototype()
{
	if (FAILED(CUI_Rect::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_WorldBackground::Initialize(void* _pArg)
{
	if (FAILED(CUI_Rect::Initialize(_pArg)))
		return E_FAIL;

	return S_OK;
}

void CUI_WorldBackground::Priority_Update(_float _fTimeDelta)
{
	CUI_Rect::Priority_Update(_fTimeDelta);
}

void CUI_WorldBackground::Update(_float _fTimeDelta)
{
	CUI_Rect::Update(_fTimeDelta);
}

void CUI_WorldBackground::Late_Update(_float _fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_RenderGroup(RENDERGROUP::PRIORITY, this)))
		return;
}

HRESULT CUI_WorldBackground::Render()
{
	if (FAILED(CUI_Rect::Render()))
		return E_FAIL;

	return S_OK;
}

CUI_WorldBackground* CUI_WorldBackground::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDeviceContext)
{
	CUI_WorldBackground* pInstance = new CUI_WorldBackground(_pDevice, _pDeviceContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("FAILED TO CREATED : CUI_WorldBackground");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_WorldBackground::Clone(void* _pArg)
{
	CUI_WorldBackground* pInstance = new CUI_WorldBackground(*this);
	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX("FAILED TO CLONED : CUI_WorldBackground");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_WorldBackground::Free()
{
	CUI_Rect::Free();
}
